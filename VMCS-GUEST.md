# Intel Processor Info

```
Processors: (two xeon cpus)
    Processor	Intel(R) Xeon(R) CPU           X5650  @ 2.67GHz, 2668 Mhz, 6 Core(s), 12 Logical Processor(s)
    Processor	Intel(R) Xeon(R) CPU           X5650  @ 2.67GHz, 2668 Mhz, 6 Core(s), 12 Logical Processor(s)
```

# VMCS - Guest Fields

#### Control Registers

* The CR0 field must not set any bit to a value not supported in VMX operation (see Section 23.8). The following
are exceptions:

    - Bit 0 (corresponding to CR0.PE) and bit 31 (PG) are not checked if the “unrestricted guest” VM-execution
control is 1.

    - Bit 29 (corresponding to CR0.NW) and bit 30 (CD) are never checked because the values of these bits are
not changed by VM entry; see Section 26.3.2.1.

```
guest cr0: 0x0000000080050033 0b1000 0000 0000 0101 0000 0000 0011 0011
guest cr3: 0x00000000001AD000
guest cr4: 0x00000000000026F8 0b0010 0110 1111 1000
```