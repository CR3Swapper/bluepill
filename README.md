# Bluepill - Type-2 Intel Hypervisor For Windows 10 Systems

<div align="center">
  <img width="1000" height="auto" src="https://githacks.org/_xeroxz/bluepill/-/raw/37484f6966c18eb371e819b32ef60334b98ea0b3/screenshots/firstvmexit.png"/>
  <p>Figure 1. First ever vmexit...</p>
</div>

Bluepill is an Intel type-2 research hypervisor. This project is purely for educational purposes and is designed to run on Windows 10 systems.
This project uses WDK and thus Windows Kernel functions to facilitate vmxlaunch. 

##### Features

* Debug with Windbg... (int 3 handler is forwarded to the guest, along with #DB)...
* Integration with VDM...
* Hypervisor has its "own" GDT, TSS, IDT, and address space (some entries point to guest controlled addresses explained below)...
* Read/Write Virtual/Physical memory example...
* SEH in vmxroot handled by hypervisor interrupt handlers...
* SEH works reguardless if manually mapped or loaded normally...

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
} idt_entry_t, * pidt_entry_t;
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

The host IDT is 1:1 to the guest IDT except for three interrupt handlers, #PF, #DE, and #GP. These three different interrupt handlers all route to the same SEH handler function
which just changes RIP to the catch block of whatever try/except the exception happened in. This allows for page faults, general protection faults and division errors to be handled
by host controlled interrupt handlers.

```cpp
// setup IDT for host....
segment_descriptor_register_64 idt_value;
__sidt(&idt_value);

// copy the guest IDT entries...
memcpy(idt::table, (void*)idt_value.base_address, idt_value.limit);

// change gp, pf, and de to vmxroot handlers...
idt::table[general_protection] = idt::create_entry(hv::idt_addr_t{ __gp_handler }, idt::ist_idx::gp);
idt::table[page_fault] = idt::create_entry(hv::idt_addr_t{ __pf_handler }, idt::ist_idx::pf);
idt::table[divide_error] = idt::create_entry(hv::idt_addr_t{ __de_handler }, idt::ist_idx::de);
```

### Host CR3 - Hypervisor Address Space

The host CR3 value contains a different PML4 (Page Mape Level 4) PFN (Page Frame Number) then the system address space. This new address space contains the same PML4E's as the
system address space except the lower 255 PML4E's are reserved for the hypervisor. The reasoning for copying the PML4E's into the new address space is that the GDT, TSS, vcpu structures, 
VMCS, interrupt stacks, and vmexit stacks are all allocated with ExAllocatePool. PML4E 256 is a self referencing PML4E setup by the hypervisor in `drv_entry`.

###### Mapping PTE's
Entries in the hypervisors PML4 between 0 and 255 actually contain PTE's. The hypervisor does not maintain any PDPT's, PD's or PT's, all layers of address translation can be
done with a single PML4. This is possible due to the fact PML4E's, PDPTE's, and PDE's are all the same structure. This fact combined with self referencing entries allow
for an address to use the self referencing PML4E as a PDPTE, and also a PDE. 

Each logical processor has two PTE's. One for source page mapping, and destiniation mapping. This allows for copying of memory directly between two physical pages possible
without the need for an intermediate buffer.

```cpp
auto map_page(u64 phys_addr, map_type type) -> u64
{
    cpuid_eax_01 cpuid_value;
    virt_addr_t result{ (u64) vmxroot_pml4 };
    __cpuid((int*)&cpuid_value, 1);

    // PTE index is specified by the logical processor 
    // number + the mapping type (0 for src, and 1 for dest)...
    result.pt_index = (cpuid_value
        .cpuid_additional_information
            .initial_apic_id * 2)
            + (unsigned)type;

    reinterpret_cast<ppte>(vmxroot_pml4)
        [result.pt_index].pfn = phys_addr >> 12;

    __invlpg((void*)result.value);
    result.offset_4kb = phys_addr_t{ phys_addr }.offset_4kb;
    return result.value;
}
```

Also it may seem like you would need to have more then two PTE's (two pages), to facilitate first translating virtual addresses from another address space to a physical address
and then mapping that physical page into virtual memory. However, this is all accomplishable with only two PTE's. You will see throughout mm.cpp/mm.hpp a parameter `map_type type`.
All mapping required to translate and map a page must be done with either the `map_type::src` or `map_type::dest`. This makes it so if you have already mapped a page with `map_type::dest`
the PTE mapping that page will not be used to map the `map_type::src` page. 

# Demo - VDM Integration, Physical/Virtual Memory Read/Write

<div align="center">
  <img width="1000" height="auto" src="https://githacks.org/_xeroxz/bluepill/-/raw/37484f6966c18eb371e819b32ef60334b98ea0b3/screenshots/demo.png"/>
  <p>Figure 2. demo VDM example and read/write virtual memory...</p>
</div>

The demo code demonstrates bluepill integration with VDM and a read/write physical memory, and virtual memory of other address spaces. 

```cpp
// use the hypervisor to read and write physical memory...
vdm::read_phys_t _read_phys =
	[&](void* addr, void* buffer, std::size_t size) -> bool
{
	return bluepill::read_phys(
		reinterpret_cast<u64>(addr), buffer, size);
};

vdm::write_phys_t _write_phys =
	[&](void* addr, void* buffer, std::size_t size) -> bool
{
	return bluepill::write_phys(
		reinterpret_cast<u64>(addr), buffer, size);
};
```

# Related Work - Resources

[Intel Manual Volume 3C](https://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-vol-3c-part-3-manual.pdf)

[daax - 7 Days To Virtualization: A Series On Hypervisor Development](https://revers.engineering/7-days-to-virtualization-a-series-on-hypervisor-development/)

[Sina Karvandi - Hypervisor from scratch](https://rayanfam.com/topics/hypervisor-from-scratch-part-1/)

[HyperPlatform](https://github.com/tandasat/HyperPlatform)

[Gbps - gbhv](https://github.com/Gbps/gbhv)

[wbenny - hvpp (very nice btw, good reference)](https://github.com/wbenny/hvpp)

[_xeroxz - Hyper-v Hacking Framework](https://githacks.org/_xeroxz/voyager)
