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

		DBG_PRINT("host cr0: 0x%p\n", __readcr0());
		DBG_PRINT("host cr3: 0x%p\n", __readcr3());
		DBG_PRINT("host cr4: 0x%p\n", __readcr4());

		// stack growns down...
		__vmx_vmwrite(VMCS_HOST_RSP, host_rsp + (PAGE_SIZE * HOST_STACK_PAGES) - 0x10);
		__vmx_vmwrite(VMCS_HOST_RIP, reinterpret_cast<u64>(host_rip));

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

		// FS base is 0 on windows so no need to read it...
		__vmx_vmwrite(VMCS_HOST_GS_BASE, __readmsr(IA32_GS_BASE));
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

		DBG_PRINT("guest cr0: 0x%p\n", __readcr0());
		DBG_PRINT("guest cr3: 0x%p\n", __readcr3());
		DBG_PRINT("guest cr4: 0x%p\n", __readcr4());

		__vmx_vmwrite(VMCS_GUEST_VMCS_LINK_POINTER, ~0ULL);
		__vmx_vmwrite(VMCS_GUEST_GDTR_BASE, gdt_value.base_address);
		__vmx_vmwrite(VMCS_GUEST_GDTR_LIMIT, gdt_value.limit);

		DBG_PRINT("guest gdt base address: 0x%p\n", gdt_value.base_address);
		DBG_PRINT("guest gdt limit: 0x%x\n", gdt_value.limit);

		__vmx_vmwrite(VMCS_GUEST_IDTR_BASE, idt_value.base_address);
		__vmx_vmwrite(VMCS_GUEST_IDTR_LIMIT, idt_value.limit);

		DBG_PRINT("guest idt base address: 0x%p\n", idt_value.base_address);
		DBG_PRINT("guest idt limit: 0x%x\n", idt_value.limit);

		__vmx_vmwrite(VMCS_GUEST_RFLAGS, __readeflags());
		__vmx_vmwrite(VMCS_GUEST_DR7, __readdr(7));

		DBG_PRINT("guest rflags: 0x%p\n", __readeflags());
		DBG_PRINT("guest debug register 7: 0x%p\n", __readdr(7));

		const auto [es_rights, es_limit, es_base] = 
			segment::get_info(gdt_value, segment_selector{ reades() });

		DBG_PRINT("es selector: 0x%p\n", reades());
		DBG_PRINT("		- es.index: %d\n", segment_selector{ reades() }.index);
		DBG_PRINT("		- es.request_privilege_level: %d\n", segment_selector{ reades() }.request_privilege_level);
		DBG_PRINT("		- es.table: %d\n", segment_selector{ reades() }.table);
		DBG_PRINT("es base address: 0x%p\n", es_base);
		DBG_PRINT("es limit: 0x%p\n", es_limit);
		DBG_PRINT("es rights: 0x%p\n", es_rights.flags);
		DBG_PRINT("		- es_rights.available_bit: %d\n", es_rights.available_bit);
		DBG_PRINT("		- es_rights.default_big: %d\n", es_rights.default_big);
		DBG_PRINT("		- es_rights.descriptor_privilege_level: %d\n", es_rights.descriptor_privilege_level);
		DBG_PRINT("		- es_rights.descriptor_type: %d\n", es_rights.descriptor_type);
		DBG_PRINT("		- es_rights.granularity: %d\n", es_rights.granularity);
		DBG_PRINT("		- es_rights.long_mode: %d\n", es_rights.long_mode);
		DBG_PRINT("		- es_rights.present: %d\n", es_rights.present);
		DBG_PRINT("		- es_rights.type: %d\n", es_rights.type);
		DBG_PRINT("		- es_rights.unusable: %d\n", es_rights.unusable);

		__vmx_vmwrite(VMCS_GUEST_ES_BASE, es_base);
		__vmx_vmwrite(VMCS_GUEST_ES_LIMIT, es_limit);
		__vmx_vmwrite(VMCS_GUEST_ES_SELECTOR, reades());
		__vmx_vmwrite(VMCS_GUEST_ES_ACCESS_RIGHTS, es_rights.flags);

		const auto [fs_rights, fs_limit, fs_base] = 
			segment::get_info(gdt_value, segment_selector{ readfs() });

		DBG_PRINT("fs selector: 0x%p\n", readfs());
		DBG_PRINT("		- fs.index: %d\n", segment_selector{ readfs() }.index);
		DBG_PRINT("		- fs.request_privilege_level: %d\n", segment_selector{ readfs() }.request_privilege_level);
		DBG_PRINT("		- fs.table: %d\n", segment_selector{ readfs() }.table);
		DBG_PRINT("fs base address: 0x%p\n", fs_base);
		DBG_PRINT("fs limit: 0x%p\n", fs_limit);
		DBG_PRINT("fs rights: 0x%p\n", fs_rights.flags);
		DBG_PRINT("		- fs_rights.available_bit: %d\n", fs_rights.available_bit);
		DBG_PRINT("		- fs_rights.default_big: %d\n", fs_rights.default_big);
		DBG_PRINT("		- fs_rights.descriptor_privilege_level: %d\n", fs_rights.descriptor_privilege_level);
		DBG_PRINT("		- fs_rights.descriptor_type: %d\n", fs_rights.descriptor_type);
		DBG_PRINT("		- fs_rights.granularity: %d\n", fs_rights.granularity);
		DBG_PRINT("		- fs_rights.long_mode: %d\n", fs_rights.long_mode);
		DBG_PRINT("		- fs_rights.present: %d\n", fs_rights.present);
		DBG_PRINT("		- fs_rights.type: %d\n", fs_rights.type);
		DBG_PRINT("		- fs_rights.unusable: %d\n", fs_rights.unusable);

		__vmx_vmwrite(VMCS_GUEST_FS_BASE, fs_base);
		__vmx_vmwrite(VMCS_GUEST_FS_LIMIT, fs_limit);
		__vmx_vmwrite(VMCS_GUEST_FS_SELECTOR, readfs());
		__vmx_vmwrite(VMCS_GUEST_FS_ACCESS_RIGHTS, fs_rights.flags);

		const auto [gs_rights, gs_limit, gs_base] = 
			segment::get_info(gdt_value, segment_selector{ readgs() });

		DBG_PRINT("gs selector: 0x%p\n", readgs());
		DBG_PRINT("		- gs.index: %d\n", segment_selector{ readgs() }.index);
		DBG_PRINT("		- gs.request_privilege_level: %d\n", segment_selector{ readgs() }.request_privilege_level);
		DBG_PRINT("		- gs.table: %d\n", segment_selector{ readgs() }.table);
		DBG_PRINT("gs base address: 0x%p\n", gs_base);
		DBG_PRINT("gs limit: 0x%p\n", gs_limit);
		DBG_PRINT("gs rights: 0x%p\n", gs_rights.flags);
		DBG_PRINT("		- gs_rights.available_bit: %d\n", gs_rights.available_bit);
		DBG_PRINT("		- gs_rights.default_big: %d\n", gs_rights.default_big);
		DBG_PRINT("		- gs_rights.descriptor_privilege_level: %d\n", gs_rights.descriptor_privilege_level);
		DBG_PRINT("		- gs_rights.descriptor_type: %d\n", gs_rights.descriptor_type);
		DBG_PRINT("		- gs_rights.granularity: %d\n", gs_rights.granularity);
		DBG_PRINT("		- gs_rights.long_mode: %d\n", gs_rights.long_mode);
		DBG_PRINT("		- gs_rights.present: %d\n", gs_rights.present);
		DBG_PRINT("		- gs_rights.type: %d\n", gs_rights.type);
		DBG_PRINT("		- gs_rights.unusable: %d\n", gs_rights.unusable);

		__vmx_vmwrite(VMCS_GUEST_GS_BASE, gs_base);
		__vmx_vmwrite(VMCS_GUEST_GS_LIMIT, gs_limit);
		__vmx_vmwrite(VMCS_GUEST_GS_SELECTOR, readgs());
		__vmx_vmwrite(VMCS_GUEST_GS_ACCESS_RIGHTS, gs_rights.flags);

		const auto [ss_rights, ss_limit, ss_base] =
			segment::get_info(gdt_value, segment_selector{ readss() });

		DBG_PRINT("ss selector: 0x%p\n", readss());
		DBG_PRINT("		- ss.index: %d\n", segment_selector{ readss() }.index);
		DBG_PRINT("		- ss.request_privilege_level: %d\n", segment_selector{ readss() }.request_privilege_level);
		DBG_PRINT("		- ss.table: %d\n", segment_selector{ readss() }.table);
		DBG_PRINT("ss base address: 0x%p\n", ss_base);
		DBG_PRINT("ss limit: 0x%p\n", ss_limit);
		DBG_PRINT("ss rights: 0x%p\n", ss_rights.flags);
		DBG_PRINT("		- ss_rights.available_bit: %d\n", ss_rights.available_bit);
		DBG_PRINT("		- ss_rights.default_big: %d\n", ss_rights.default_big);
		DBG_PRINT("		- ss_rights.descriptor_privilege_level: %d\n", ss_rights.descriptor_privilege_level);
		DBG_PRINT("		- ss_rights.descriptor_type: %d\n", ss_rights.descriptor_type);
		DBG_PRINT("		- ss_rights.granularity: %d\n", ss_rights.granularity);
		DBG_PRINT("		- ss_rights.long_mode: %d\n", ss_rights.long_mode);
		DBG_PRINT("		- ss_rights.present: %d\n", ss_rights.present);
		DBG_PRINT("		- ss_rights.type: %d\n", ss_rights.type);
		DBG_PRINT("		- ss_rights.unusable: %d\n", ss_rights.unusable);

		__vmx_vmwrite(VMCS_GUEST_SS_BASE, ss_base);
		__vmx_vmwrite(VMCS_GUEST_SS_LIMIT, ss_limit);
		__vmx_vmwrite(VMCS_GUEST_SS_SELECTOR, readss());
		__vmx_vmwrite(VMCS_GUEST_SS_ACCESS_RIGHTS, ss_rights.flags);

		const auto [cs_rights, cs_limit, cs_base] =
			segment::get_info(gdt_value, segment_selector{ readcs() });

		DBG_PRINT("cs selector: 0x%p\n", readcs());
		DBG_PRINT("		- cs.index: %d\n", segment_selector{ readcs() }.index);
		DBG_PRINT("		- cs.request_privilege_level: %d\n", segment_selector{ readcs() }.request_privilege_level);
		DBG_PRINT("		- cs.table: %d\n", segment_selector{ readcs() }.table);
		DBG_PRINT("cs base address: 0x%p\n", cs_base);
		DBG_PRINT("cs limit: 0x%p\n", cs_limit);
		DBG_PRINT("cs rights: 0x%p\n", cs_rights.flags);
		DBG_PRINT("		- cs_rights.available_bit: %d\n", cs_rights.available_bit);
		DBG_PRINT("		- cs_rights.default_big: %d\n", cs_rights.default_big);
		DBG_PRINT("		- cs_rights.descriptor_privilege_level: %d\n", cs_rights.descriptor_privilege_level);
		DBG_PRINT("		- cs_rights.descriptor_type: %d\n", cs_rights.descriptor_type);
		DBG_PRINT("		- cs_rights.granularity: %d\n", cs_rights.granularity);
		DBG_PRINT("		- cs_rights.long_mode: %d\n", cs_rights.long_mode);
		DBG_PRINT("		- cs_rights.present: %d\n", cs_rights.present);
		DBG_PRINT("		- cs_rights.type: %d\n", cs_rights.type);
		DBG_PRINT("		- cs_rights.unusable: %d\n", cs_rights.unusable);

		__vmx_vmwrite(VMCS_GUEST_CS_BASE, cs_base);
		__vmx_vmwrite(VMCS_GUEST_CS_LIMIT, cs_limit);
		__vmx_vmwrite(VMCS_GUEST_CS_SELECTOR, readcs());
		__vmx_vmwrite(VMCS_GUEST_CS_ACCESS_RIGHTS, cs_rights.flags);

		const auto [ds_rights, ds_limit, ds_base] =
			segment::get_info(gdt_value, segment_selector{ readds() });

		DBG_PRINT("ds selector: 0x%p\n", readds());
		DBG_PRINT("		- ds.index: %d\n", segment_selector{ readds() }.index);
		DBG_PRINT("		- ds.request_privilege_level: %d\n", segment_selector{ readds() }.request_privilege_level);
		DBG_PRINT("		- ds.table: %d\n", segment_selector{ readds() }.table);
		DBG_PRINT("ds base address: 0x%p\n", ds_base);
		DBG_PRINT("ds limit: 0x%p\n", ds_limit);
		DBG_PRINT("ds rights: 0x%p\n", ds_rights.flags);
		DBG_PRINT("		- ds_rights.available_bit: %d\n", ds_rights.available_bit);
		DBG_PRINT("		- ds_rights.default_big: %d\n", ds_rights.default_big);
		DBG_PRINT("		- ds_rights.descriptor_privilege_level: %d\n", ds_rights.descriptor_privilege_level);
		DBG_PRINT("		- ds_rights.descriptor_type: %d\n", ds_rights.descriptor_type);
		DBG_PRINT("		- ds_rights.granularity: %d\n", ds_rights.granularity);
		DBG_PRINT("		- ds_rights.long_mode: %d\n", ds_rights.long_mode);
		DBG_PRINT("		- ds_rights.present: %d\n", ds_rights.present);
		DBG_PRINT("		- ds_rights.type: %d\n", ds_rights.type);
		DBG_PRINT("		- ds_rights.unusable: %d\n", ds_rights.unusable);

		__vmx_vmwrite(VMCS_GUEST_DS_BASE, ds_base);
		__vmx_vmwrite(VMCS_GUEST_DS_LIMIT, ds_limit);
		__vmx_vmwrite(VMCS_GUEST_DS_SELECTOR, readds());
		__vmx_vmwrite(VMCS_GUEST_DS_ACCESS_RIGHTS, ds_rights.flags);

		const auto [tr_rights, tr_limit, tr_base] =
			segment::get_info(gdt_value, segment_selector{ readtr() });

		DBG_PRINT("tr selector: 0x%p\n", readtr());
		DBG_PRINT("		- tr.index: %d\n", segment_selector{ readtr() }.index);
		DBG_PRINT("		- tr.request_privilege_level: %d\n", segment_selector{ readtr() }.request_privilege_level);
		DBG_PRINT("		- tr.table: %d\n", segment_selector{ readtr() }.table);
		DBG_PRINT("tr base address: 0x%p\n", tr_base);
		DBG_PRINT("tr limit: 0x%p\n", tr_limit);
		DBG_PRINT("tr rights: 0x%p\n", tr_rights.flags);
		DBG_PRINT("		- tr_rights.available_bit: %d\n", tr_rights.available_bit);
		DBG_PRINT("		- tr_rights.default_big: %d\n", tr_rights.default_big);
		DBG_PRINT("		- tr_rights.descriptor_privilege_level: %d\n", tr_rights.descriptor_privilege_level);
		DBG_PRINT("		- tr_rights.descriptor_type: %d\n", tr_rights.descriptor_type);
		DBG_PRINT("		- tr_rights.granularity: %d\n", tr_rights.granularity);
		DBG_PRINT("		- tr_rights.long_mode: %d\n", tr_rights.long_mode);
		DBG_PRINT("		- tr_rights.present: %d\n", tr_rights.present);
		DBG_PRINT("		- tr_rights.type: %d\n", tr_rights.type);
		DBG_PRINT("		- tr_rights.unusable: %d\n", tr_rights.unusable);

		__vmx_vmwrite(VMCS_GUEST_TR_BASE, tr_base);
		__vmx_vmwrite(VMCS_GUEST_TR_LIMIT, tr_limit);
		__vmx_vmwrite(VMCS_GUEST_TR_SELECTOR, readtr());
		__vmx_vmwrite(VMCS_GUEST_TR_ACCESS_RIGHTS, tr_rights.flags);

		const auto [ldt_rights, ldt_limit, ldt_base] =
			segment::get_info(gdt_value, segment_selector{ readldt() });

		DBG_PRINT("ldt selector: 0x%p\n", readldt());
		DBG_PRINT("		- ldt.index: %d\n", segment_selector{ readldt() }.index);
		DBG_PRINT("		- ldt.request_privilege_level: %d\n", segment_selector{ readldt() }.request_privilege_level);
		DBG_PRINT("		- ldt.table: %d\n", segment_selector{ readldt() }.table);
		DBG_PRINT("ldt base address: 0x%p\n", tr_base);
		DBG_PRINT("ldt limit: 0x%p\n", tr_limit);
		DBG_PRINT("ldt rights: 0x%p\n", tr_rights.flags);
		DBG_PRINT("		- ldt_rights.available_bit: %d\n", tr_rights.available_bit);
		DBG_PRINT("		- ldt_rights.default_big: %d\n", tr_rights.default_big);
		DBG_PRINT("		- ldt_rights.descriptor_privilege_level: %d\n", tr_rights.descriptor_privilege_level);
		DBG_PRINT("		- ldt_rights.descriptor_type: %d\n", tr_rights.descriptor_type);
		DBG_PRINT("		- ldt_rights.granularity: %d\n", tr_rights.granularity);
		DBG_PRINT("		- ldt_rights.long_mode: %d\n", tr_rights.long_mode);
		DBG_PRINT("		- ldt_rights.present: %d\n", tr_rights.present);
		DBG_PRINT("		- ldt_rights.type: %d\n", tr_rights.type);
		DBG_PRINT("		- ldt_rights.unusable: %d\n", tr_rights.unusable);

		__vmx_vmwrite(VMCS_GUEST_LDTR_BASE, ldt_base);
		__vmx_vmwrite(VMCS_GUEST_LDTR_LIMIT, ldt_limit);
		__vmx_vmwrite(VMCS_GUEST_LDTR_SELECTOR, readldt());
		__vmx_vmwrite(VMCS_GUEST_LDTR_ACCESS_RIGHTS, ldt_rights.flags);

		__vmx_vmwrite(VMCS_GUEST_GS_BASE, __readmsr(IA32_GS_BASE));
		DBG_PRINT("guest gs base (from readmsr): 0x%p\n", __readmsr(IA32_GS_BASE));		

		__vmx_vmwrite(VMCS_GUEST_ACTIVITY_STATE, 0);
		__vmx_vmwrite(VMCS_GUEST_INTERRUPTIBILITY_STATE, 0);
		__vmx_vmwrite(VMCS_GUEST_PENDING_DEBUG_EXCEPTIONS, 0);
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
		procbased_ctls2.enable_xsaves = true; 
		procbased_ctls2.conceal_vmx_from_pt = true; 
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