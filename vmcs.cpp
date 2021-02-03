#include "vmcs.hpp"

namespace vmcs
{
	auto setup_host(void* host_rip, u64 host_rsp) -> void
	{
		segment_descriptor_register_64 gdt_value;
		segment_descriptor_register_64 idt_value;

		__sidt(&idt_value);
		_sgdt(&gdt_value);

		// use guest values for now... later on CR3 will be custom...
		__vmx_vmwrite(VMCS_HOST_CR0, __readcr0());
		__vmx_vmwrite(VMCS_HOST_CR3, __readcr3());
		__vmx_vmwrite(VMCS_HOST_CR4, __readcr4());

		// stack growns down...
		__vmx_vmwrite(VMCS_HOST_RSP, host_rsp + (PAGE_SIZE * HOST_STACK_PAGES) - 8);
		__vmx_vmwrite(VMCS_HOST_RIP, reinterpret_cast<u64>(host_rip));

		__vmx_vmwrite(VMCS_HOST_SYSENTER_CS, __readmsr(IA32_SYSENTER_CS));
		__vmx_vmwrite(VMCS_HOST_SYSENTER_EIP, __readmsr(IA32_SYSENTER_EIP));
		__vmx_vmwrite(VMCS_HOST_SYSENTER_ESP, __readmsr(IA32_SYSENTER_ESP));

		__vmx_vmwrite(VMCS_HOST_GDTR_BASE, gdt_value.base_address);
		__vmx_vmwrite(VMCS_HOST_IDTR_BASE, idt_value.base_address);

		// manual says that the priv level must be 0 and 
		// that the table flag also needs to be 0 so it uses the GDT...
		segment_selector es{ reades() };
		es.request_privilege_level = NULL;
		es.table = NULL;

		segment_selector cs{ readcs() };
		cs.request_privilege_level = NULL;
		cs.table = NULL;

		segment_selector ds{ readds() };
		ds.request_privilege_level = NULL;
		ds.table = NULL;

		segment_selector fs{ readfs() };
		fs.request_privilege_level = NULL;
		fs.table = NULL;

		segment_selector gs{ readgs() };
		gs.request_privilege_level = NULL;
		gs.table = NULL;

		segment_selector ss{ readss() };
		ss.request_privilege_level = NULL;
		ss.table = NULL;

		segment_selector tr{ readtr() };
		tr.request_privilege_level = NULL;
		tr.table = NULL;

		__vmx_vmwrite(VMCS_HOST_ES_SELECTOR, es.flags);
		__vmx_vmwrite(VMCS_HOST_CS_SELECTOR, cs.flags);
		__vmx_vmwrite(VMCS_HOST_DS_SELECTOR, ds.flags);
		__vmx_vmwrite(VMCS_HOST_FS_SELECTOR, fs.flags);
		__vmx_vmwrite(VMCS_HOST_GS_SELECTOR, gs.flags);
		__vmx_vmwrite(VMCS_HOST_SS_SELECTOR, ss.flags);
		__vmx_vmwrite(VMCS_HOST_TR_SELECTOR, tr.flags);

		__vmx_vmwrite(VMCS_HOST_GS_BASE, __readmsr(IA32_GS_BASE));
		__vmx_vmwrite(VMCS_HOST_FS_BASE, __readmsr(IA32_FS_BASE));
	}

	auto setup_guest() -> void
	{
		segment_descriptor_register_64 gdt_value;
		segment_descriptor_register_64 idt_value;

		__sidt(&idt_value);
		_sgdt(&gdt_value);

		__vmx_vmwrite(VMCS_GUEST_CR0, __readcr0());
		__vmx_vmwrite(VMCS_GUEST_CR3, __readcr3());
		__vmx_vmwrite(VMCS_GUEST_CR4, __readcr4());

		__vmx_vmwrite(VMCS_GUEST_VMCS_LINK_POINTER, ~0ULL);
		__vmx_vmwrite(VMCS_GUEST_DEBUGCTL, __readmsr(IA32_DEBUGCTL));

		__vmx_vmwrite(VMCS_GUEST_DEBUGCTL, __readmsr(IA32_DEBUGCTL));
		__vmx_vmwrite(VMCS_GUEST_SYSENTER_CS, __readmsr(IA32_SYSENTER_CS));
		__vmx_vmwrite(VMCS_GUEST_SYSENTER_EIP, __readmsr(IA32_SYSENTER_EIP));
		__vmx_vmwrite(VMCS_GUEST_SYSENTER_ESP, __readmsr(IA32_SYSENTER_ESP));

		__vmx_vmwrite(VMCS_GUEST_GDTR_BASE, gdt_value.base_address);
		__vmx_vmwrite(VMCS_GUEST_GDTR_LIMIT, gdt_value.limit);

		__vmx_vmwrite(VMCS_GUEST_IDTR_BASE, idt_value.base_address);
		__vmx_vmwrite(VMCS_GUEST_IDTR_LIMIT, idt_value.limit);

		__vmx_vmwrite(VMCS_GUEST_RFLAGS, __readeflags());
		__vmx_vmwrite(VMCS_GUEST_DR7, __readdr(7));

		const auto [es_rights, es_limit, es_base] = 
			segment::get_info(gdt_value, segment_selector{ reades() });

		__vmx_vmwrite(VMCS_GUEST_ES_BASE, es_base);
		__vmx_vmwrite(VMCS_GUEST_ES_LIMIT, es_limit);
		__vmx_vmwrite(VMCS_GUEST_ES_SELECTOR, reades());
		__vmx_vmwrite(VMCS_GUEST_ES_ACCESS_RIGHTS, es_rights.flags);

		const auto [ds_rights, ds_limit, ds_base] = 
			segment::get_info(gdt_value, segment_selector{ readds() });

		__vmx_vmwrite(VMCS_GUEST_DS_BASE, ds_base);
		__vmx_vmwrite(VMCS_GUEST_DS_LIMIT, ds_limit);
		__vmx_vmwrite(VMCS_GUEST_DS_SELECTOR, readds());
		__vmx_vmwrite(VMCS_GUEST_DS_ACCESS_RIGHTS, ds_rights.flags);

		const auto [fs_rights, fs_limit, fs_base] = 
			segment::get_info(gdt_value, segment_selector{ readfs() });

		__vmx_vmwrite(VMCS_GUEST_FS_BASE, fs_base);
		__vmx_vmwrite(VMCS_GUEST_FS_LIMIT, fs_limit);
		__vmx_vmwrite(VMCS_GUEST_FS_SELECTOR, readfs());
		__vmx_vmwrite(VMCS_GUEST_FS_ACCESS_RIGHTS, fs_rights.flags);

		const auto [gs_rights, gs_limit, gs_base] = 
			segment::get_info(gdt_value, segment_selector{ readgs() });

		__vmx_vmwrite(VMCS_GUEST_GS_BASE, gs_base);
		__vmx_vmwrite(VMCS_GUEST_GS_LIMIT, gs_limit);
		__vmx_vmwrite(VMCS_GUEST_GS_SELECTOR, readgs());
		__vmx_vmwrite(VMCS_GUEST_GS_ACCESS_RIGHTS, gs_rights.flags);

		const auto [ss_rights, ss_limit, ss_base] =
			segment::get_info(gdt_value, segment_selector{ readss() });

		__vmx_vmwrite(VMCS_GUEST_SS_BASE, ss_base);
		__vmx_vmwrite(VMCS_GUEST_SS_LIMIT, ss_limit);
		__vmx_vmwrite(VMCS_GUEST_SS_SELECTOR, readss());
		__vmx_vmwrite(VMCS_GUEST_SS_ACCESS_RIGHTS, ss_rights.flags);

		const auto [cs_rights, cs_limit, cs_base] =
			segment::get_info(gdt_value, segment_selector{ readcs() });

		__vmx_vmwrite(VMCS_GUEST_CS_BASE, cs_base);
		__vmx_vmwrite(VMCS_GUEST_CS_LIMIT, cs_limit);
		__vmx_vmwrite(VMCS_GUEST_CS_SELECTOR, readcs());
		__vmx_vmwrite(VMCS_GUEST_CS_ACCESS_RIGHTS, cs_rights.flags);

		const auto [tr_rights, tr_limit, tr_base] =
			segment::get_info(gdt_value, segment_selector{ readtr() });

		__vmx_vmwrite(VMCS_GUEST_TR_BASE, tr_base);
		__vmx_vmwrite(VMCS_GUEST_TR_LIMIT, tr_limit);
		__vmx_vmwrite(VMCS_GUEST_TR_SELECTOR, readtr());
		__vmx_vmwrite(VMCS_GUEST_TR_ACCESS_RIGHTS, tr_rights.flags);

		const auto [ldt_rights, ldt_limit, ldt_base] =
			segment::get_info(gdt_value, segment_selector{ readldt() });

		__vmx_vmwrite(VMCS_GUEST_LDTR_BASE, ldt_base);
		__vmx_vmwrite(VMCS_GUEST_LDTR_LIMIT, ldt_limit);
		__vmx_vmwrite(VMCS_GUEST_LDTR_SELECTOR, readldt());
		__vmx_vmwrite(VMCS_GUEST_LDTR_ACCESS_RIGHTS, ldt_rights.flags);

		__vmx_vmwrite(VMCS_GUEST_GS_BASE, __readmsr(IA32_GS_BASE));
		__vmx_vmwrite(VMCS_GUEST_FS_BASE, __readmsr(IA32_FS_BASE));
	}

	auto setup_controls() -> void
	{
		ia32_vmx_true_ctls_register msr_fix_value;
		ia32_vmx_pinbased_ctls_register pinbased_ctls;
		ia32_vmx_procbased_ctls_register procbased_ctls;
		ia32_vmx_procbased_ctls2_register procbased_ctls2;
		ia32_vmx_entry_ctls_register entry_ctls;
		ia32_vmx_exit_ctls_register exit_ctls;
		ia32_vmx_basic_register vmx_basic;

		vmx_basic.flags = __readmsr(IA32_VMX_BASIC);
		pinbased_ctls.flags = NULL;
		procbased_ctls.flags = NULL;
		procbased_ctls2.flags = NULL;
		entry_ctls.flags = NULL;
		exit_ctls.flags = NULL;

		if (vmx_basic.vmx_controls)
		{
			msr_fix_value.flags = __readmsr(IA32_VMX_TRUE_PINBASED_CTLS);
			pinbased_ctls.flags &= msr_fix_value.allowed_1_settings;
			pinbased_ctls.flags |= msr_fix_value.allowed_0_settings;
			__vmx_vmwrite(VMCS_CTRL_PIN_BASED_VM_EXECUTION_CONTROLS, pinbased_ctls.flags);

			DBG_PRINT("============================== pinbased vm-exit controls ==============================\n");
			DBG_PRINT("		- pinbased_ctls.activate_vmx_preemption_timer: %d\n", pinbased_ctls.activate_vmx_preemption_timer);
			DBG_PRINT("		- pinbased_ctls.external_interrupt_exiting: %d\n", pinbased_ctls.external_interrupt_exiting);
			DBG_PRINT("		- pinbased_ctls.nmi_exiting: %d\n", pinbased_ctls.nmi_exiting);
			DBG_PRINT("		- pinbased_ctls.process_posted_interrupts: %d\n", pinbased_ctls.process_posted_interrupts);
			DBG_PRINT("		- pinbased_ctls.virtual_nmi: %d\n", pinbased_ctls.virtual_nmi);
			DBG_PRINT("			- pinbased_ctls.flags: 0x%x\n", pinbased_ctls.flags);
			DBG_PRINT("			- IA32_VMX_TRUE_PINBASED_CTLS high bits mask: 0x%x\n", msr_fix_value.allowed_1_settings);
			DBG_PRINT("			- IA32_VMX_TRUE_PINBASED_CTLS low bits mask: 0x%x\n", msr_fix_value.allowed_0_settings);

			msr_fix_value.flags = __readmsr(IA32_VMX_TRUE_PROCBASED_CTLS);
			procbased_ctls.activate_secondary_controls = true;
			procbased_ctls.flags &= msr_fix_value.allowed_1_settings;
			procbased_ctls.flags |= msr_fix_value.allowed_0_settings;
			__vmx_vmwrite(VMCS_CTRL_PROCESSOR_BASED_VM_EXECUTION_CONTROLS, procbased_ctls.flags);

			DBG_PRINT("============================== processor based vm-exit controls ==============================\n");
			DBG_PRINT("		- procbased_ctls.activate_secondary_controls: %d\n", procbased_ctls.activate_secondary_controls);
			DBG_PRINT("		- procbased_ctls.cr3_load_exiting: %d\n", procbased_ctls.cr3_load_exiting);
			DBG_PRINT("		- procbased_ctls.cr3_store_exiting: %d\n", procbased_ctls.cr3_store_exiting);
			DBG_PRINT("		- procbased_ctls.cr8_load_exiting: %d\n", procbased_ctls.cr8_load_exiting);
			DBG_PRINT("		- procbased_ctls.cr8_store_exiting: %d\n", procbased_ctls.cr8_store_exiting);
			DBG_PRINT("		- procbased_ctls.hlt_exiting: %d\n", procbased_ctls.hlt_exiting);
			DBG_PRINT("		- procbased_ctls.interrupt_window_exiting: %d\n", procbased_ctls.interrupt_window_exiting);
			DBG_PRINT("		- procbased_ctls.invlpg_exiting: %d\n", procbased_ctls.invlpg_exiting);
			DBG_PRINT("		- procbased_ctls.monitor_exiting: %d\n", procbased_ctls.monitor_exiting);
			DBG_PRINT("		- procbased_ctls.monitor_trap_flag: %d\n", procbased_ctls.monitor_trap_flag);
			DBG_PRINT("		- procbased_ctls.mov_dr_exiting: %d\n", procbased_ctls.mov_dr_exiting);
			DBG_PRINT("		- procbased_ctls.mwait_exiting: %d\n", procbased_ctls.mwait_exiting);
			DBG_PRINT("		- procbased_ctls.nmi_window_exiting: %d\n", procbased_ctls.nmi_window_exiting);
			DBG_PRINT("		- procbased_ctls.pause_exiting: %d\n", procbased_ctls.pause_exiting);
			DBG_PRINT("		- procbased_ctls.rdpmc_exiting: %d\n", procbased_ctls.rdpmc_exiting);
			DBG_PRINT("		- procbased_ctls.rdtsc_exiting: %d\n", procbased_ctls.rdtsc_exiting);
			DBG_PRINT("		- procbased_ctls.unconditional_io_exiting: %d\n", procbased_ctls.unconditional_io_exiting);
			DBG_PRINT("		- procbased_ctls.use_io_bitmaps: %d\n", procbased_ctls.use_io_bitmaps);
			DBG_PRINT("		- procbased_ctls.use_msr_bitmaps: %d\n", procbased_ctls.use_msr_bitmaps);
			DBG_PRINT("		- procbased_ctls.use_tpr_shadow: %d\n", procbased_ctls.use_tpr_shadow);
			DBG_PRINT("		- procbased_ctls.use_tsc_offsetting: %d\n", procbased_ctls.use_tsc_offsetting);
			DBG_PRINT("			- procbased_ctls.flags: 0x%x\n", procbased_ctls.flags);
			DBG_PRINT("			- IA32_VMX_TRUE_PROCBASED_CTLS high bits mask: 0x%x\n", msr_fix_value.allowed_1_settings);
			DBG_PRINT("			- IA32_VMX_TRUE_PROCBASED_CTLS low bits mask: 0x%x\n", msr_fix_value.allowed_0_settings);

			msr_fix_value.flags = __readmsr(IA32_VMX_TRUE_ENTRY_CTLS);
			entry_ctls.ia32e_mode_guest = true;
			entry_ctls.flags &= msr_fix_value.allowed_1_settings;
			entry_ctls.flags |= msr_fix_value.allowed_0_settings;
			__vmx_vmwrite(VMCS_CTRL_VMENTRY_CONTROLS, entry_ctls.flags);

			DBG_PRINT("============================== vm-entry controls ==============================\n");
			DBG_PRINT("		- entry_ctls.conceal_vmx_from_pt: %d\n", entry_ctls.conceal_vmx_from_pt);
			DBG_PRINT("		- entry_ctls.deactivate_dual_monitor_treatment: %d\n", entry_ctls.deactivate_dual_monitor_treatment);
			DBG_PRINT("		- entry_ctls.entry_to_smm: %d\n", entry_ctls.entry_to_smm);
			DBG_PRINT("		- entry_ctls.ia32e_mode_guest: %d\n", entry_ctls.ia32e_mode_guest);
			DBG_PRINT("		- entry_ctls.load_cet_state: %d\n", entry_ctls.load_cet_state);
			DBG_PRINT("		- entry_ctls.load_debug_controls: %d\n", entry_ctls.load_debug_controls);
			DBG_PRINT("		- entry_ctls.load_ia32_bndcfgs: %d\n", entry_ctls.load_ia32_bndcfgs);
			DBG_PRINT("		- entry_ctls.load_ia32_efer: %d\n", entry_ctls.load_ia32_efer);
			DBG_PRINT("		- entry_ctls.load_ia32_pat: %d\n", entry_ctls.load_ia32_pat);
			DBG_PRINT("		- entry_ctls.load_ia32_perf_global_ctrl: %d\n", entry_ctls.load_ia32_perf_global_ctrl);
			DBG_PRINT("		- entry_ctls.load_ia32_rtit_ctl: %d\n", entry_ctls.load_ia32_rtit_ctl);
			DBG_PRINT("			- entry_ctls.flags: 0x%x\n", entry_ctls.flags);
			DBG_PRINT("			- IA32_VMX_TRUE_ENTRY_CTLS high bits mask: 0x%x\n", msr_fix_value.allowed_1_settings);
			DBG_PRINT("			- IA32_VMX_TRUE_ENTRY_CTLS low bits mask: 0x%x\n", msr_fix_value.allowed_0_settings);

			msr_fix_value.flags = __readmsr(IA32_VMX_TRUE_EXIT_CTLS);
			exit_ctls.host_address_space_size = true;
			exit_ctls.flags &= msr_fix_value.allowed_1_settings;
			exit_ctls.flags |= msr_fix_value.allowed_0_settings;
			__vmx_vmwrite(VMCS_CTRL_VMEXIT_CONTROLS, exit_ctls.flags);

			DBG_PRINT("============================== vm-exit controls ==============================\n");
			DBG_PRINT("		- exit_ctls.acknowledge_interrupt_on_exit: %d\n", exit_ctls.acknowledge_interrupt_on_exit);
			DBG_PRINT("		- exit_ctls.clear_ia32_bndcfgs: %d\n", exit_ctls.clear_ia32_bndcfgs);
			DBG_PRINT("		- exit_ctls.conceal_vmx_from_pt: %d\n", exit_ctls.conceal_vmx_from_pt);
			DBG_PRINT("		- exit_ctls.host_address_space_size: %d\n", exit_ctls.host_address_space_size);
			DBG_PRINT("		- exit_ctls.load_ia32_efer: %d\n", exit_ctls.load_ia32_efer);
			DBG_PRINT("		- exit_ctls.load_ia32_pat: %d\n", exit_ctls.load_ia32_pat);
			DBG_PRINT("		- exit_ctls.load_ia32_perf_global_ctrl: %d\n", exit_ctls.load_ia32_perf_global_ctrl);
			DBG_PRINT("		- exit_ctls.save_debug_controls: %d\n", exit_ctls.save_debug_controls);
			DBG_PRINT("		- exit_ctls.save_ia32_efer: %d\n", exit_ctls.save_ia32_efer);
			DBG_PRINT("		- exit_ctls.save_ia32_pat: %d\n", exit_ctls.save_ia32_pat);
			DBG_PRINT("		- exit_ctls.save_vmx_preemption_timer_value: %d\n", exit_ctls.save_vmx_preemption_timer_value);
			DBG_PRINT("			- exit_ctls.flags: 0x%x\n", exit_ctls.flags);
			DBG_PRINT("			- IA32_VMX_TRUE_EXIT_CTLS high bits mask: 0x%x\n", msr_fix_value.allowed_1_settings);
			DBG_PRINT("			- IA32_VMX_TRUE_EXIT_CTLS low bits mask: 0x%x\n", msr_fix_value.allowed_0_settings);
		}
		else
		{
			msr_fix_value.flags = __readmsr(IA32_VMX_PINBASED_CTLS);
			pinbased_ctls.flags &= msr_fix_value.allowed_1_settings;
			pinbased_ctls.flags |= msr_fix_value.allowed_0_settings;
			__vmx_vmwrite(VMCS_CTRL_PIN_BASED_VM_EXECUTION_CONTROLS, pinbased_ctls.flags);

			DBG_PRINT("============================== pinbased vm-exit controls ==============================\n");
			DBG_PRINT("		- pinbased_ctls.activate_vmx_preemption_timer: %d\n", pinbased_ctls.activate_vmx_preemption_timer);
			DBG_PRINT("		- pinbased_ctls.external_interrupt_exiting: %d\n", pinbased_ctls.external_interrupt_exiting);
			DBG_PRINT("		- pinbased_ctls.nmi_exiting: %d\n", pinbased_ctls.nmi_exiting);
			DBG_PRINT("		- pinbased_ctls.process_posted_interrupts: %d\n", pinbased_ctls.process_posted_interrupts);
			DBG_PRINT("		- pinbased_ctls.virtual_nmi: %d\n", pinbased_ctls.virtual_nmi);
			DBG_PRINT("			- pinbased_ctls.flags: 0x%x\n", pinbased_ctls.flags);
			DBG_PRINT("			- IA32_VMX_PINBASED_CTLS high bits mask: 0x%x\n", msr_fix_value.allowed_1_settings);
			DBG_PRINT("			- IA32_VMX_PINBASED_CTLS low bits mask: 0x%x\n", msr_fix_value.allowed_0_settings);

			msr_fix_value.flags = __readmsr(IA32_VMX_PROCBASED_CTLS);
			procbased_ctls.activate_secondary_controls = true;
			procbased_ctls.flags &= msr_fix_value.allowed_1_settings;
			procbased_ctls.flags |= msr_fix_value.allowed_0_settings;
			__vmx_vmwrite(VMCS_CTRL_PROCESSOR_BASED_VM_EXECUTION_CONTROLS, procbased_ctls.flags);

			DBG_PRINT("============================== processor based vm-exit controls ==============================\n");
			DBG_PRINT("		- procbased_ctls.activate_secondary_controls: %d\n", procbased_ctls.activate_secondary_controls);
			DBG_PRINT("		- procbased_ctls.cr3_load_exiting: %d\n", procbased_ctls.cr3_load_exiting);
			DBG_PRINT("		- procbased_ctls.cr3_store_exiting: %d\n", procbased_ctls.cr3_store_exiting);
			DBG_PRINT("		- procbased_ctls.cr8_load_exiting: %d\n", procbased_ctls.cr8_load_exiting);
			DBG_PRINT("		- procbased_ctls.cr8_store_exiting: %d\n", procbased_ctls.cr8_store_exiting);
			DBG_PRINT("		- procbased_ctls.hlt_exiting: %d\n", procbased_ctls.hlt_exiting);
			DBG_PRINT("		- procbased_ctls.interrupt_window_exiting: %d\n", procbased_ctls.interrupt_window_exiting);
			DBG_PRINT("		- procbased_ctls.invlpg_exiting: %d\n", procbased_ctls.invlpg_exiting);
			DBG_PRINT("		- procbased_ctls.monitor_exiting: %d\n", procbased_ctls.monitor_exiting);
			DBG_PRINT("		- procbased_ctls.monitor_trap_flag: %d\n", procbased_ctls.monitor_trap_flag);
			DBG_PRINT("		- procbased_ctls.mov_dr_exiting: %d\n", procbased_ctls.mov_dr_exiting);
			DBG_PRINT("		- procbased_ctls.mwait_exiting: %d\n", procbased_ctls.mwait_exiting);
			DBG_PRINT("		- procbased_ctls.nmi_window_exiting: %d\n", procbased_ctls.nmi_window_exiting);
			DBG_PRINT("		- procbased_ctls.pause_exiting: %d\n", procbased_ctls.pause_exiting);
			DBG_PRINT("		- procbased_ctls.rdpmc_exiting: %d\n", procbased_ctls.rdpmc_exiting);
			DBG_PRINT("		- procbased_ctls.rdtsc_exiting: %d\n", procbased_ctls.rdtsc_exiting);
			DBG_PRINT("		- procbased_ctls.unconditional_io_exiting: %d\n", procbased_ctls.unconditional_io_exiting);
			DBG_PRINT("		- procbased_ctls.use_io_bitmaps: %d\n", procbased_ctls.use_io_bitmaps);
			DBG_PRINT("		- procbased_ctls.use_msr_bitmaps: %d\n", procbased_ctls.use_msr_bitmaps);
			DBG_PRINT("		- procbased_ctls.use_tpr_shadow: %d\n", procbased_ctls.use_tpr_shadow);
			DBG_PRINT("		- procbased_ctls.use_tsc_offsetting: %d\n", procbased_ctls.use_tsc_offsetting);
			DBG_PRINT("			- procbased_ctls.flags: 0x%x\n", procbased_ctls.flags);
			DBG_PRINT("			- IA32_VMX_PROCBASED_CTLS high bits mask: 0x%x\n", msr_fix_value.allowed_1_settings);
			DBG_PRINT("			- IA32_VMX_PROCBASED_CTLS low bits mask: 0x%x\n", msr_fix_value.allowed_0_settings);

			msr_fix_value.flags = __readmsr(IA32_VMX_ENTRY_CTLS);
			entry_ctls.ia32e_mode_guest = true;
			entry_ctls.conceal_vmx_from_pt = true;
			entry_ctls.flags &= msr_fix_value.allowed_1_settings;
			entry_ctls.flags |= msr_fix_value.allowed_0_settings;
			__vmx_vmwrite(VMCS_CTRL_VMENTRY_CONTROLS, entry_ctls.flags);

			DBG_PRINT("============================== vm-entry controls ==============================\n");
			DBG_PRINT("		- entry_ctls.conceal_vmx_from_pt: %d\n", entry_ctls.conceal_vmx_from_pt);
			DBG_PRINT("		- entry_ctls.deactivate_dual_monitor_treatment: %d\n", entry_ctls.deactivate_dual_monitor_treatment);
			DBG_PRINT("		- entry_ctls.entry_to_smm: %d\n", entry_ctls.entry_to_smm);
			DBG_PRINT("		- entry_ctls.ia32e_mode_guest: %d\n", entry_ctls.ia32e_mode_guest);
			DBG_PRINT("		- entry_ctls.load_cet_state: %d\n", entry_ctls.load_cet_state);
			DBG_PRINT("		- entry_ctls.load_debug_controls: %d\n", entry_ctls.load_debug_controls);
			DBG_PRINT("		- entry_ctls.load_ia32_bndcfgs: %d\n", entry_ctls.load_ia32_bndcfgs);
			DBG_PRINT("		- entry_ctls.load_ia32_efer: %d\n", entry_ctls.load_ia32_efer);
			DBG_PRINT("		- entry_ctls.load_ia32_pat: %d\n", entry_ctls.load_ia32_pat);
			DBG_PRINT("		- entry_ctls.load_ia32_perf_global_ctrl: %d\n", entry_ctls.load_ia32_perf_global_ctrl);
			DBG_PRINT("		- entry_ctls.load_ia32_rtit_ctl: %d\n", entry_ctls.load_ia32_rtit_ctl);
			DBG_PRINT("			- entry_ctls.flags: 0x%x\n", entry_ctls.flags);
			DBG_PRINT("			- IA32_VMX_ENTRY_CTLS high bits mask: 0x%x\n", msr_fix_value.allowed_1_settings);
			DBG_PRINT("			- IA32_VMX_ENTRY_CTLS low bits mask: 0x%x\n", msr_fix_value.allowed_0_settings);

			msr_fix_value.flags = __readmsr(IA32_VMX_EXIT_CTLS);
			exit_ctls.host_address_space_size = true;
			exit_ctls.conceal_vmx_from_pt = true;
			exit_ctls.flags &= msr_fix_value.allowed_1_settings;
			exit_ctls.flags |= msr_fix_value.allowed_0_settings;
			__vmx_vmwrite(VMCS_CTRL_VMEXIT_CONTROLS, exit_ctls.flags);

			DBG_PRINT("============================== vm-exit controls ==============================\n");
			DBG_PRINT("		- exit_ctls.acknowledge_interrupt_on_exit: %d\n", exit_ctls.acknowledge_interrupt_on_exit);
			DBG_PRINT("		- exit_ctls.clear_ia32_bndcfgs: %d\n", exit_ctls.clear_ia32_bndcfgs);
			DBG_PRINT("		- exit_ctls.conceal_vmx_from_pt: %d\n", exit_ctls.conceal_vmx_from_pt);
			DBG_PRINT("		- exit_ctls.host_address_space_size: %d\n", exit_ctls.host_address_space_size);
			DBG_PRINT("		- exit_ctls.load_ia32_efer: %d\n", exit_ctls.load_ia32_efer);
			DBG_PRINT("		- exit_ctls.load_ia32_pat: %d\n", exit_ctls.load_ia32_pat);
			DBG_PRINT("		- exit_ctls.load_ia32_perf_global_ctrl: %d\n", exit_ctls.load_ia32_perf_global_ctrl);
			DBG_PRINT("		- exit_ctls.save_debug_controls: %d\n", exit_ctls.save_debug_controls);
			DBG_PRINT("		- exit_ctls.save_ia32_efer: %d\n", exit_ctls.save_ia32_efer);
			DBG_PRINT("		- exit_ctls.save_ia32_pat: %d\n", exit_ctls.save_ia32_pat);
			DBG_PRINT("		- exit_ctls.save_vmx_preemption_timer_value: %d\n", exit_ctls.save_vmx_preemption_timer_value);
			DBG_PRINT("			- exit_ctls.flags: 0x%x\n", exit_ctls.flags);
			DBG_PRINT("			- IA32_VMX_EXIT_CTLS high bits mask: 0x%x\n", msr_fix_value.allowed_1_settings);
			DBG_PRINT("			- IA32_VMX_EXIT_CTLS low bits mask: 0x%x\n", msr_fix_value.allowed_0_settings);
		}

		msr_fix_value.flags = __readmsr(IA32_VMX_PROCBASED_CTLS2);
		procbased_ctls2.enable_rdtscp = true;
		procbased_ctls2.enable_xsaves = true; // although my xeons dont support xsave... other cpus do!
		procbased_ctls2.conceal_vmx_from_pt = true; // although my xeons dont support processor tracing... other cpus do!
		procbased_ctls2.flags &= msr_fix_value.allowed_1_settings;
		procbased_ctls2.flags |= msr_fix_value.allowed_0_settings;
		__vmx_vmwrite(VMCS_CTRL_SECONDARY_PROCESSOR_BASED_VM_EXECUTION_CONTROLS, procbased_ctls2.flags);

		DBG_PRINT("============================== secondary processor based vm-exec controls ==============================\n");
		DBG_PRINT("		- procbased_ctls2.apic_register_virtualization: %d\n", procbased_ctls2.apic_register_virtualization);
		DBG_PRINT("		- procbased_ctls2.conceal_vmx_from_pt: %d\n", procbased_ctls2.conceal_vmx_from_pt);
		DBG_PRINT("		- procbased_ctls2.descriptor_table_exiting: %d\n", procbased_ctls2.descriptor_table_exiting);
		DBG_PRINT("		- procbased_ctls2.enable_encls_exiting: %d\n", procbased_ctls2.enable_encls_exiting);
		DBG_PRINT("		- procbased_ctls2.enable_ept: %d\n", procbased_ctls2.enable_ept);
		DBG_PRINT("		- procbased_ctls2.enable_invpcid: %d\n", procbased_ctls2.enable_invpcid);
		DBG_PRINT("		- procbased_ctls2.enable_pml: %d\n", procbased_ctls2.enable_pml);
		DBG_PRINT("		- procbased_ctls2.enable_rdtscp: %d\n", procbased_ctls2.enable_rdtscp);
		DBG_PRINT("		- procbased_ctls2.enable_vm_functions: %d\n", procbased_ctls2.enable_vm_functions);
		DBG_PRINT("		- procbased_ctls2.enable_vpid: %d\n", procbased_ctls2.enable_vpid);
		DBG_PRINT("		- procbased_ctls2.enable_xsaves: %d\n", procbased_ctls2.enable_xsaves);
		DBG_PRINT("		- procbased_ctls2.ept_violation: %d\n", procbased_ctls2.ept_violation);
		DBG_PRINT("		- procbased_ctls2.mode_based_execute_control_for_ept: %d\n", procbased_ctls2.mode_based_execute_control_for_ept);
		DBG_PRINT("		- procbased_ctls2.pause_loop_exiting: %d\n", procbased_ctls2.pause_loop_exiting);
		DBG_PRINT("		- procbased_ctls2.rdrand_exiting: %d\n", procbased_ctls2.rdrand_exiting);
		DBG_PRINT("		- procbased_ctls2.rdseed_exiting: %d\n", procbased_ctls2.rdseed_exiting);
		DBG_PRINT("		- procbased_ctls2.unrestricted_guest: %d\n", procbased_ctls2.unrestricted_guest);
		DBG_PRINT("		- procbased_ctls2.use_tsc_scaling: %d\n", procbased_ctls2.use_tsc_scaling);
		DBG_PRINT("		- procbased_ctls2.virtualize_apic_accesses: %d\n", procbased_ctls2.virtualize_apic_accesses);
		DBG_PRINT("		- procbased_ctls2.virtualize_x2apic_mode: %d\n", procbased_ctls2.virtualize_x2apic_mode);
		DBG_PRINT("		- procbased_ctls2.virtual_interrupt_delivery: %d\n", procbased_ctls2.virtual_interrupt_delivery);
		DBG_PRINT("		- procbased_ctls2.vmcs_shadowing: %d\n", procbased_ctls2.vmcs_shadowing);
		DBG_PRINT("		- procbased_ctls2.wbinvd_exiting: %d\n", procbased_ctls2.wbinvd_exiting);
		DBG_PRINT("			- procbased_ctls2.flags: 0x%x\n", procbased_ctls2.flags);
		DBG_PRINT("			- IA32_VMX_PROCBASED_CTLS2 high bits mask: 0x%x\n", msr_fix_value.allowed_1_settings);
		DBG_PRINT("			- IA32_VMX_PROCBASED_CTLS2 low bits mask: 0x%x\n", msr_fix_value.allowed_0_settings);
	}
}