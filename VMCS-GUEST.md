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
field for that register. The first processors to support the virtual-machine extensions supported only the 1-
setting of this control and thus performed this check unconditionally.



#### 26.3.1.2 Checks on Guest Segment Registers