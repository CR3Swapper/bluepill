<div align="center">
  <img width="1000" height="auto" src="https://imgur.com/b1bYNZU.png"/>
  <p>Figure 1. First ever vmexit...</p>
</div>

# Bluepill

Bluepill is an Intel type-2 research hypervisor. This project is purely for educational purposes and is designed to run on Windows 10 systems.
This project uses WDK and thus Windows Kernel functions to facilitate vmxlaunch. 

### Why Write A Hypervisor?

Why write a type-2 (Intel or AMD) hypervisor? "To learn" is the typical response, but to learn what? To learn VMX instructions? To learn how to write a windows kernel driver? To learn how to use windbg? Although all of the prior reasons to write a hypervisor are important, learning how to read technical documents and extract what you need from the reading material is much more valuable than all of the other stuff one might learn while writing a hypervisor. This is best summed up as the old saying goes: 

> “give a man a fish, he will feed himself for a day, teach a man to read the manual and he will make a hypervisor”

### VMCS

This section of the readme just contains notes and a list of things I stumbled on and took me a while to figure out and fix.

##### VMCS Controls

* One of the mistakes I made early on was setting bits high after applying high/low MSR values. For example my xeons dont support Intel Processor Trace (Intel PT) and I was setting `entry_ctls.conceal_vmx_from_pt = true` after applying the MSR high/low masks. This caused vmxerror #7 (invalid vmcs controls). Now i set the bit high before i apply the high/low bit mask so if my hypervisor runs on a cpu that has Intel PT support it will be concealed from Intel PT.
* My xeons also dont support xsave/xrstor and I was setting enable_xsave in secondary processor based vmexit controls after applying `IA32_VMX_PROCBASED_CTLS2` high/low bitmask. Which caused vmxerror #7 (invalid vmcs controls).

Dump of VMCS control fields can be found [here](https://githacks.org/_xeroxz/bluepill/-/blob/master/VMCS-CONTROLS.md). This is not required, but for learning its nice to
see exactly what the MSR masks are, and what VMCS field's are enabled after you apply high/low bit masks.

##### VMCS Guest State

* After getting my first vmexit the exit reason was 0x80000021 (invalid guest state). I thought it was segmentation code since I've never done anything with segments before but after a few days of checking every single segment check in chapter 26 section 3, I continued reading the guest requirements in chapter 24  section 4, part 2 goes over non-register states and I was not setting  `VMCS_GUEST_ACTIVITY_STATE` to zero. 

Dump of VMCS guest fields can be found [here](https://githacks.org/_xeroxz/bluepill/-/blob/master/VMCS-GUEST.md). 

### Host State Information

Bluepill has its "own" GDT, TSS, IDT, and address space. However, in order to allow for windbg usage, some interrupt handlers 
forward to guest controlled interrupt handlers such as #DB and interrupt handler three. The host GDT also contains a unique TR base (TSS), which contains three new interrupt stacks.
These stacks are used by bluepills interrupt routines. This is not required at all but I felt I should go the extra mile here and setup dedicated stacks for my interrupt handlers in the 
off chance that RSP contains an invalid address when a page fault, division error, or general protection error happens.

#### GDT - Global Descriptor Table

The host GDT is 1:1 with the guest GDT except firstly, a different, host controlled page is used for each cores GDT. Secondly the TR segment base address is updated to reflect
the new TSS (which is also 1:1 with the guest TSS but on a new page).

```cpp
segment_descriptor_register_64 gdt_value;
_sgdt(&gdt_value);

// the GDT can be 65536 bytes large 
// but on windows its less then a single page (4kb)
// ...
// also note each logical processor gets its own GDT... 
memcpy(vcpu->gdt, (void*)gdt_value.base_address, PAGE_SIZE);
```

###### TSS - Task State Segment

The host TSS is 1:1 with the guest TSS except that there are additional interrupt stack table entries. When an exception happens and execution is redirected to an interrupt handler, the address
in RSP cannot ***always*** be trusted. Therefore, ***especially*** on privilege level changes, RSP will be changed with a predetermined valid stack (which is located in the TSS). However if an exception happens and there is no privilege change (say you have an exception in ring-0),
RSP ***might not*** need to be changed as there is not a risk of privilege escalation. An OS (and type-2 hypervisor) designer can determine how they want RSP to be handled by the CPU by configuring interrupt descriptor table entries accordingly. 
In an interrupt descriptor table entry there is a bit field for interrupt stack table index. 

```cpp
typedef struct _tss64
{
	u32 reserved;
	
	// if you dont use an IST entry and there is a privilage change, 
	// rsp will be swapped with an address in this array...
	u64 privilege_stacks[3]; 
	
	u64 reserved_1;
	u64 interrupt_stack_table[7];
	u16 reserved_2;
	u16 iomap_base;
} tss64, *ptss64;

typedef union _idt_entry_t
{
	u128 flags;
	struct
	{
		u64 offset_low : 16;
		u64 segment_selector : 16;
		
		// if this is zero IST isnt used, if there is no privilage change then RSP wont be changed at all, 
		// and if there is a privilage change then RSP is swapped with an address in the TSS (rsp0).
		u64 ist_index : 3; 
		
		u64 reserved_0 : 5;
		u64 gate_type : 5;
		u64 dpl : 2;
		u64 present : 1;
		u64 offset_middle : 16;
		u64 offset_high : 32;
		u64 reserved_1 : 32;
	};
} idt_entry_t, *pidt_entry_t;
```

###### IST - Interrupt Stack Table
This interrupt stack table is located inside of the TSS. Bluepill interrupt routines have their own stack, this is the only change done to the TSS. IST entries zero through three are used by windows interrupt routines and entries four through six are used by Bluepill. 

```cpp
// host page fault interrupt stack...
vcpu->tss.interrupt_stack_table[idt::ist_idx::pf] =
	reinterpret_cast<u64>(ExAllocatePool(NonPagedPool, 
		PAGE_SIZE * HOST_STACK_PAGES)) + (PAGE_SIZE * HOST_STACK_PAGES);

// host general protection interrupt stack...
vcpu->tss.interrupt_stack_table[idt::ist_idx::gp] =
	reinterpret_cast<u64>(ExAllocatePool(NonPagedPool,
		PAGE_SIZE * HOST_STACK_PAGES)) + (PAGE_SIZE * HOST_STACK_PAGES);

// host division error interrupt stack...
vcpu->tss.interrupt_stack_table[idt::ist_idx::de] =
	reinterpret_cast<u64>(ExAllocatePool(NonPagedPool,
		PAGE_SIZE * HOST_STACK_PAGES)) + (PAGE_SIZE * HOST_STACK_PAGES);
```

#### IDT - Interrupt Descriptor Table