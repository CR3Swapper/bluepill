# Intel Processor Info

```
Processors: (two xeon cpus)
    Processor	Intel(R) Xeon(R) CPU           X5650  @ 2.67GHz, 2668 Mhz, 6 Core(s), 12 Logical Processor(s)
    Processor	Intel(R) Xeon(R) CPU           X5650  @ 2.67GHz, 2668 Mhz, 6 Core(s), 12 Logical Processor(s)
```
    
# VMCS - Control Fields

#### Pinbased VM-Exit Controls

```
============================== pinbased vm-exit controls ==============================
		- pinbased_ctls.activate_vmx_preemption_timer: 0
		- pinbased_ctls.external_interrupt_exiting: 0
		- pinbased_ctls.nmi_exiting: 0
		- pinbased_ctls.process_posted_interrupts: 0
		- pinbased_ctls.virtual_nmi: 0
			- pinbased_ctls.flags: 0x16
			- IA32_VMX_TRUE_PINBASED_CTLS high bits mask: 0x3f
			- IA32_VMX_TRUE_PINBASED_CTLS low bits mask: 0x16
```

(different bits in the mask just mean that the bit can be high or low)

```
IA32_VMX_TRUE_PINBASED_CTLS high bits mask: 0b00111111 
IA32_VMX_TRUE_PINBASED_CTLS low bits mask:  0b00010110 
```

#### Processor Based VM-Exit Controls

```
============================== processor based vm-exit controls ==============================
		- procbased_ctls.activate_secondary_controls: 1
		- procbased_ctls.cr3_load_exiting: 0
		- procbased_ctls.cr3_store_exiting: 0
		- procbased_ctls.cr8_load_exiting: 0
		- procbased_ctls.cr8_store_exiting: 0
		- procbased_ctls.hlt_exiting: 0
		- procbased_ctls.interrupt_window_exiting: 0
		- procbased_ctls.invlpg_exiting: 0
		- procbased_ctls.monitor_exiting: 0
		- procbased_ctls.monitor_trap_flag: 0
		- procbased_ctls.mov_dr_exiting: 0
		- procbased_ctls.mwait_exiting: 0
		- procbased_ctls.nmi_window_exiting: 0
		- procbased_ctls.pause_exiting: 0
		- procbased_ctls.rdpmc_exiting: 0
		- procbased_ctls.rdtsc_exiting: 0
		- procbased_ctls.unconditional_io_exiting: 0
		- procbased_ctls.use_io_bitmaps: 0
		- procbased_ctls.use_msr_bitmaps: 0
		- procbased_ctls.use_tpr_shadow: 0
		- procbased_ctls.use_tsc_offsetting: 0
			- procbased_ctls.flags: 0x84006172
			- IA32_VMX_TRUE_PROCBASED_CTLS high bits mask: 0xfff9fffe
			- IA32_VMX_TRUE_PROCBASED_CTLS low bits mask: 0x4006172
```

(different bits in the mask just mean that the bit can be high or low)

```
IA32_VMX_TRUE_PROCBASED_CTLS high bits mask: 0b11111111111110011111111111111110 
IA32_VMX_TRUE_PROCBASED_CTLS low bits mask:  0b00000100000000000110000101110010 
```

#### VM-Entry Controls

```
============================== vm-entry controls ==============================
		- entry_ctls.conceal_vmx_from_pt: 0
		- entry_ctls.deactivate_dual_monitor_treatment: 0
		- entry_ctls.entry_to_smm: 0
		- entry_ctls.ia32e_mode_guest: 1
		- entry_ctls.load_cet_state: 0
		- entry_ctls.load_debug_controls: 0
		- entry_ctls.load_ia32_bndcfgs: 0
		- entry_ctls.load_ia32_efer: 0
		- entry_ctls.load_ia32_pat: 0
		- entry_ctls.load_ia32_perf_global_ctrl: 0
		- entry_ctls.load_ia32_rtit_ctl: 0
			- entry_ctls.flags: 0x13fb
			- IA32_VMX_TRUE_ENTRY_CTLS high bits mask: 0xf3ff
			- IA32_VMX_TRUE_ENTRY_CTLS low bits mask: 0x11fb
```

(different bits in the mask just mean that the bit can be high or low)

```
IA32_VMX_TRUE_ENTRY_CTLS high bits: 0b1111001111111111‬
IA32_VMX_TRUE_ENTRY_CTLS low bits:  0b0001000111111011
```

#### VM-Exit Controls

```
============================== vm-exit controls ==============================
		- exit_ctls.acknowledge_interrupt_on_exit: 0
		- exit_ctls.clear_ia32_bndcfgs: 0
		- exit_ctls.conceal_vmx_from_pt: 0
		- exit_ctls.host_address_space_size: 1
		- exit_ctls.load_ia32_efer: 0
		- exit_ctls.load_ia32_pat: 0
		- exit_ctls.load_ia32_perf_global_ctrl: 0
		- exit_ctls.save_debug_controls: 0
		- exit_ctls.save_ia32_efer: 0
		- exit_ctls.save_ia32_pat: 0
		- exit_ctls.save_vmx_preemption_timer_value: 0
			- exit_ctls.flags: 0x36ffb
			- IA32_VMX_TRUE_EXIT_CTLS high bits mask: 0x3fffff
			- IA32_VMX_TRUE_EXIT_CTLS low bits mask: 0x36dfb
			
```

(different bits in the mask just mean that the bit can be high or low)

```
IA32_VMX_TRUE_EXIT_CTLS high bits mask: 0b001111111111111111111111
IA32_VMX_TRUE_EXIT_CTLS low bits mask:  0b000000110110110111111011
```

#### Secondary Processor Based VM-Exec Controls

```
============================== secondary processor based vm-exec controls ==============================
		- procbased_ctls2.apic_register_virtualization: 0
		- procbased_ctls2.conceal_vmx_from_pt: 0
		- procbased_ctls2.descriptor_table_exiting: 0
		- procbased_ctls2.enable_encls_exiting: 0
		- procbased_ctls2.enable_ept: 0
		- procbased_ctls2.enable_invpcid: 0
		- procbased_ctls2.enable_pml: 0
		- procbased_ctls2.enable_rdtscp: 1
		- procbased_ctls2.enable_vm_functions: 0
		- procbased_ctls2.enable_vpid: 0
		- procbased_ctls2.enable_xsaves: 0
		- procbased_ctls2.ept_violation: 0
		- procbased_ctls2.mode_based_execute_control_for_ept: 0
		- procbased_ctls2.pause_loop_exiting: 0
		- procbased_ctls2.rdrand_exiting: 0
		- procbased_ctls2.rdseed_exiting: 0
		- procbased_ctls2.unrestricted_guest: 0
		- procbased_ctls2.use_tsc_scaling: 0
		- procbased_ctls2.virtualize_apic_accesses: 0
		- procbased_ctls2.virtualize_x2apic_mode: 0
		- procbased_ctls2.virtual_interrupt_delivery: 0
		- procbased_ctls2.vmcs_shadowing: 0
		- procbased_ctls2.wbinvd_exiting: 0
			- procbased_ctls2.flags: 0x8
			- IA32_VMX_PROCBASED_CTLS2 high bits mask: 0x404fe
			- IA32_VMX_PROCBASED_CTLS2 low bits mask: 0x0
```

(different bits in the mask just mean that the bit can be high or low)

```
IA32_VMX_PROCBASED_CTLS2 high bits mask: 0b01000000010011111110
IA32_VMX_PROCBASED_CTLS2 low bits mask:  0b00000000000000000000
```