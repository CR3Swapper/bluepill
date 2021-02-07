# Intel Processor Info

```
Processors: (two xeon cpus)
    Processor	Intel(R) Xeon(R) CPU           X5650  @ 2.67GHz, 2668 Mhz, 6 Core(s), 12 Logical Processor(s)
    Processor	Intel(R) Xeon(R) CPU           X5650  @ 2.67GHz, 2668 Mhz, 6 Core(s), 12 Logical Processor(s)
```

# VMCS - Guest Fields

#### 26.3.1.1 Checks on Guest Control Registers, Debug Registers, and MSRs

##### Checks on Guest Control Registers

* The CR0 field must not set any bit to a value not supported in VMX operation (see Section 23.8). The following
are exceptions: :white_check_mark:

    - Bit 0 (corresponding to CR0.PE) and bit 31 (PG) are not checked if the “unrestricted guest” VM-execution
control is 1.

    - Bit 29 (corresponding to CR0.NW) and bit 30 (CD) are never checked because the values of these bits are
not changed by VM entry; see Section 26.3.2.1.

* The following checks are performed on processors that support Intel 64 architecture:

    - If the “IA-32e mode guest” VM-entry control is 1, bit 31 in the CR0 field (corresponding to CR0.PG) and
bit 5 in the CR4 field (corresponding to CR4.PAE) must each be 1. :white_check_mark:

    - If the “IA-32e mode guest” VM-entry control is 0, bit 17 in the CR4 field (corresponding to CR4.PCIDE)
must be 0. (this value is one in my entry controls) :white_check_mark:

    - The CR3 field must be such that bits 63:52 and bits in the range 51:32 beyond the processor’s physicaladdress
width are 0. :white_check_mark:

* The CR4 field must not set any bit to a value not supported in VMX operation (see Section 23.8). :white_check_mark:

```
guest cr0: 0x0000000080050033 0b1000 0000 0000 0101 0000 0000 0011 0011
guest cr3: 0x00000000001AD000
guest cr4: 0x00000000000026F8 0b0010 0110 1111 1000
```

##### Checks on Guest MSRs

* If the “load debug controls” VM-entry control is 1, bits reserved in the IA32_DEBUGCTL MSR must be 0 in the
field for that register. (this is not set in vm entry control fields in my vmcs...) :white_check_mark:

* The IA32_SYSENTER_ESP field and the IA32_SYSENTER_EIP field must each contain a canonical address. (this is MSR is zero) :white_check_mark:

"In 64-bit mode, an address is considered to be in canonical form if address bits 63 through to the most-significant implemented bit by the microarchitecture are set to either all ones or all zeros..."

```
VMCS_GUEST_DEBUGCTL: 0x0000000000000000
VMCS_GUEST_SYSENTER_CS: 0x0000000000000000
VMCS_GUEST_SYSENTER_EIP: 0x0000000000000000
VMCS_GUEST_SYSENTER_ESP: 0x0000000000000000
```

#### 26.3.1.2 Checks on Guest Segment Registers

This section specifies the checks on the fields for CS, SS, DS, ES, FS, GS, TR, and LDTR.

* Selector fields.

    - TR. The TI flag (bit 2) must be 0. :white_check_mark:
    - LDTR. If LDTR is usable, the TI flag (bit 2) must be 0.  :white_check_mark:
    - SS. If the guest will not be virtual-8086 and the “unrestricted guest” VM-execution control is 0, the RPL :white_check_mark:
(bits 1:0) must equal the RPL of the selector field for CS. :white_check_mark:


* Base-address fields.

    - TR, FS, GS. The address must be canonical. :white_check_mark:
    - CS. Bits 63:32 of the address must be zero. :white_check_mark:
    - SS, DS, ES. If the register is usable, bits 63:32 of the address must be zero. :white_check_mark:
    
* Access-rights fields for CS, SS, DS, ES, FS, GS.

    - CS Bits 3:0 (Type): must be 9, 11, 13, or 15 (accessed code segment). :white_check_mark:
    - SS. If SS is usable, the Type must be 3 or 7 (read/write, accessed data segment). :white_check_mark:
    - DS, ES, FS, GS. The following checks apply if the register is usable :white_check_mark:
        - Bit 0 of the Type must be 1 (accessed). :white_check_mark:
        - If bit 3 of the Type is 1 (code segment), then bit 1 of the Type must be 1 (readable). :white_check_mark:
    - Bit 4 (S). If the register is CS or if the register is usable, S must be 1 :white_check_mark:
    
```
es selector: 0x000000000000002B
 	 - es.index: 5
	 - es.request_privilege_level: 3
	 - es.table: 0
es base address: 0x0000000000000000
es limit: 0x00000000FFFFFFFF
es rights: 0x000000000000C0F3
		- es_rights.available_bit: 0
		- es_rights.default_big: 1
		- es_rights.descriptor_privilege_level: 3
		- es_rights.descriptor_type: 1
		- es_rights.granularity: 1
		- es_rights.long_mode: 0
		- es_rights.present: 1
		- es_rights.type: 3
		- es_rights.unusable: 0
		
		
fs selector: 0x0000000000000053
		- fs.index: 10
		- fs.request_privilege_level: 3
		- fs.table: 0
fs base address: 0x0000000000000000
fs base (from readmsr): 0x0000000000000000
fs limit: 0x0000000000003C00
fs rights: 0x00000000000040F3
		- fs_rights.available_bit: 0
		- fs_rights.default_big: 1
		- fs_rights.descriptor_privilege_level: 3
		- fs_rights.descriptor_type: 1
		- fs_rights.granularity: 0
		- fs_rights.long_mode: 0
		- fs_rights.present: 1
		- fs_rights.type: 3
		- fs_rights.unusable: 0
		
		
gs selector: 0x000000000000002B
		- gs.index: 5
		- gs.request_privilege_level: 3
		- gs.table: 0
gs base address: 0x0000000000000000
gs base (from readmsr): 0xFFFFF80365406000
gs limit: 0x00000000FFFFFFFF
gs rights: 0x000000000000C0F3
		- gs_rights.available_bit: 0
		- gs_rights.default_big: 1
		- gs_rights.descriptor_privilege_level: 3
		- gs_rights.descriptor_type: 1
		- gs_rights.granularity: 1
		- gs_rights.long_mode: 0
		- gs_rights.present: 1
		- gs_rights.type: 3
		- gs_rights.unusable: 0
		
		
ss selector: 0x0000000000000018
		- ss.index: 3
		- ss.request_privilege_level: 0
		- ss.table: 0
ss base address: 0x0000000000000000
ss limit: 0x0000000000000000
ss rights: 0x0000000000004093
		- ss_rights.available_bit: 0
		- ss_rights.default_big: 1
		- ss_rights.descriptor_privilege_level: 0
		- ss_rights.descriptor_type: 1
		- ss_rights.granularity: 0
		- ss_rights.long_mode: 0
		- ss_rights.present: 1
		- ss_rights.type: 3
		- ss_rights.unusable: 0
		
		
cs selector: 0x0000000000000010
		- cs.index: 2
		- cs.request_privilege_level: 0
		- cs.table: 0
cs base address: 0x0000000000000000
cs limit: 0x0000000000000000
cs rights: 0x000000000000209B
		- cs_rights.available_bit: 0
		- cs_rights.default_big: 0
		- cs_rights.descriptor_privilege_level: 0
		- cs_rights.descriptor_type: 1
		- cs_rights.granularity: 0
		- cs_rights.long_mode: 1
		- cs_rights.present: 1
		- cs_rights.type: 11
		- cs_rights.unusable: 0
		
		
ds selector: 0x000000000000002B
		- ds.index: 5
		- ds.request_privilege_level: 3
		- ds.table: 0
ds base address: 0x0000000000000000
ds limit: 0x00000000FFFFFFFF
ds rights: 0x000000000000C0F3
		- ds_rights.available_bit: 0
		- ds_rights.default_big: 1
		- ds_rights.descriptor_privilege_level: 3
		- ds_rights.descriptor_type: 1
		- ds_rights.granularity: 1
		- ds_rights.long_mode: 0
		- ds_rights.present: 1
		- ds_rights.type: 3
		- ds_rights.unusable: 0
```

* TR Access Rights Checks. The different sub-fields are considered separately:

    - Bits 3:0 (Type).
        - If the guest will not be IA-32e mode, the Type must be 3 (16-bit busy TSS) or 11 (32-bit busy
TSS). :white_check_mark:
        - If the guest will be IA-32e mode, the Type must be 11 (64-bit busy TSS). :white_check_mark:
        
    - Bit 4 (S). S must be 0. :white_check_mark:
    - Bit 7 (P). P must be 1. :white_check_mark:
    - Bits 11:8 (reserved). These bits must all be 0. :white_check_mark:
    - Bit 15 (G).
        - If any bit in the limit field in the range 11:0 is 0, G must be 0. :white_check_mark:
        - If any bit in the limit field in the range 31:20 is 1, G must be 1. :white_check_mark:
    - Bit 16 (Unusable). The unusable bit must be 0. :white_check_mark:
    - Bits 31:17 (reserved). These bits must all be 0. :white_check_mark:
    
```
tr selector: 0x0000000000000040
		- tr.index: 8
		- tr.request_privilege_level: 0
		- tr.table: 0
tr base address: 0xFFFFF8036EA5F000
tr limit: 0x0000000000000067
tr rights: 0x000000000000008B
		- tr_rights.available_bit: 0
		- tr_rights.default_big: 0
		- tr_rights.descriptor_privilege_level: 0
		- tr_rights.descriptor_type: 0
		- tr_rights.granularity: 0
		- tr_rights.long_mode: 0
		- tr_rights.present: 1
		- tr_rights.type: 11
		- tr_rights.unusable: 0
```

* LDTR Access Rights Checks. The following checks on the different sub-fields apply only if LDTR is usable:

    - Bits 3:0 (Type). The Type must be 2 (LDT). :white_check_mark:
	- Bit 4 (S). S must be 0. :white_check_mark:
    - Bit 7 (P). P must be 1. :white_check_mark:
    - Bits 11:8 (reserved). These bits must all be 0. :white_check_mark:
    - Bit 15 (G).
        - If any bit in the limit field in the range 11:0 is 0, G must be 0. :white_check_mark:
        - If any bit in the limit field in the range 31:20 is 1, G must be 1. :white_check_mark:
    - Bits 31:17 (reserved). These bits must all be 0. :white_check_mark:
    
```
ldt selector: 0x0000000000000000
		- ldt.index: 0
		- ldt.request_privilege_level: 0
		- ldt.table: 0
ldt base address: 0xFFFFF8036EA5F000
ldt limit: 0x0000000000000067
ldt rights: 0x000000000000008B
		- ldt_rights.available_bit: 0
		- ldt_rights.default_big: 0
		- ldt_rights.descriptor_privilege_level: 0
		- ldt_rights.descriptor_type: 0
		- ldt_rights.granularity: 0
		- ldt_rights.long_mode: 0
		- ldt_rights.present: 1
		- ldt_rights.type: 11
		- ldt_rights.unusable: 0
```