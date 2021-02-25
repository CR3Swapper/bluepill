#include "vmcs.hpp"

namespace vmcs
{
	auto setup_host(void* host_rip, u64 host_rsp, cr3 cr3_value, u64 gdt_base) -> void
	{
		__vmx_vmwrite(VMCS_HOST_CR0, __readcr0());
		__vmx_vmwrite(VMCS_HOST_CR3, cr3_value.flags);
		__vmx_vmwrite(VMCS_HOST_CR4, __readcr4());

		__vmx_vmwrite(VMCS_HOST_RSP, host_rsp + (PAGE_SIZE * HOST_STACK_PAGES));
		__vmx_vmwrite(VMCS_HOST_RIP, reinterpret_cast<u64>(host_rip));

		__vmx_vmwrite(VMCS_HOST_GDTR_BASE, gdt_base);
		__vmx_vmwrite(VMCS_HOST_IDTR_BASE, reinterpret_cast<u64>(idt::table));

		segment_selector es{ reades() };
		es.request_privilege_level = NULL;
		es.table = NULL;
		__vmx_vmwrite(VMCS_HOST_ES_SELECTOR, es.flags);

		segment_selector cs{ readcs() };
		cs.request_privilege_level = NULL;
		cs.table = NULL;
		__vmx_vmwrite(VMCS_HOST_CS_SELECTOR, cs.flags);

		segment_selector ds{ readds() };
		ds.request_privilege_level = NULL;
		ds.table = NULL;
		__vmx_vmwrite(VMCS_HOST_DS_SELECTOR, ds.flags);

		segment_selector fs{ readfs() };
		fs.request_privilege_level = NULL;
		fs.table = NULL;
		__vmx_vmwrite(VMCS_HOST_FS_SELECTOR, fs.flags);
		__vmx_vmwrite(VMCS_HOST_FS_BASE, __readmsr(IA32_FS_BASE));

		segment_selector gs{ readgs() };
		gs.request_privilege_level = NULL;
		gs.table = NULL;
		__vmx_vmwrite(VMCS_HOST_GS_SELECTOR, gs.flags);
		__vmx_vmwrite(VMCS_HOST_GS_BASE, __readmsr(IA32_GS_BASE));

		segment_selector ss{ readss() };
		ss.request_privilege_level = NULL;
		ss.table = NULL;
		__vmx_vmwrite(VMCS_HOST_SS_SELECTOR, ss.flags);

		segment_selector tr{ readtr() };
		tr.request_privilege_level = NULL;
		tr.table = NULL;
		__vmx_vmwrite(VMCS_HOST_TR_SELECTOR, tr.flags);
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
		__vmx_vmwrite(VMCS_GUEST_GDTR_BASE, gdt_value.base_address);
		__vmx_vmwrite(VMCS_GUEST_GDTR_LIMIT, gdt_value.limit);

		__vmx_vmwrite(VMCS_GUEST_IDTR_BASE, idt_value.base_address);
		__vmx_vmwrite(VMCS_GUEST_IDTR_LIMIT, idt_value.limit);

		__vmx_vmwrite(VMCS_GUEST_RFLAGS, __readeflags());
		__vmx_vmwrite(VMCS_GUEST_DR7, __readdr(7));

		const auto [es_descriptor, es_rights, es_limit, es_base] =
			gdt::get_info(gdt_value, segment_selector{ reades() });

		__vmx_vmwrite(VMCS_GUEST_ES_BASE, es_base);
		__vmx_vmwrite(VMCS_GUEST_ES_LIMIT, es_limit);
		__vmx_vmwrite(VMCS_GUEST_ES_SELECTOR, reades());
		__vmx_vmwrite(VMCS_GUEST_ES_ACCESS_RIGHTS, es_rights.flags);

		const auto [fs_descriptor, fs_rights, fs_limit, fs_base] =
			gdt::get_info(gdt_value, segment_selector{ readfs() });

		__vmx_vmwrite(VMCS_GUEST_FS_BASE, fs_base);
		__vmx_vmwrite(VMCS_GUEST_FS_LIMIT, fs_limit);
		__vmx_vmwrite(VMCS_GUEST_FS_SELECTOR, readfs());
		__vmx_vmwrite(VMCS_GUEST_FS_ACCESS_RIGHTS, fs_rights.flags);

		const auto [gs_descriptor, gs_rights, gs_limit, gs_base] =
			gdt::get_info(gdt_value, segment_selector{ readgs() });

		__vmx_vmwrite(VMCS_GUEST_GS_BASE, gs_base);
		__vmx_vmwrite(VMCS_GUEST_GS_LIMIT, gs_limit);
		__vmx_vmwrite(VMCS_GUEST_GS_SELECTOR, readgs());
		__vmx_vmwrite(VMCS_GUEST_GS_ACCESS_RIGHTS, gs_rights.flags);

		const auto [ss_descriptor, ss_rights, ss_limit, ss_base] =
			gdt::get_info(gdt_value, segment_selector{ readss() });

		__vmx_vmwrite(VMCS_GUEST_SS_BASE, ss_base);
		__vmx_vmwrite(VMCS_GUEST_SS_LIMIT, ss_limit);
		__vmx_vmwrite(VMCS_GUEST_SS_SELECTOR, readss());
		__vmx_vmwrite(VMCS_GUEST_SS_ACCESS_RIGHTS, ss_rights.flags);

		const auto [cs_descriptor, cs_rights, cs_limit, cs_base] =
			gdt::get_info(gdt_value, segment_selector{ readcs() });

		__vmx_vmwrite(VMCS_GUEST_CS_BASE, cs_base);
		__vmx_vmwrite(VMCS_GUEST_CS_LIMIT, cs_limit);
		__vmx_vmwrite(VMCS_GUEST_CS_SELECTOR, readcs());
		__vmx_vmwrite(VMCS_GUEST_CS_ACCESS_RIGHTS, cs_rights.flags);

		const auto [ds_descriptor, ds_rights, ds_limit, ds_base] =
			gdt::get_info(gdt_value, segment_selector{ readds() });

		__vmx_vmwrite(VMCS_GUEST_DS_BASE, ds_base);
		__vmx_vmwrite(VMCS_GUEST_DS_LIMIT, ds_limit);
		__vmx_vmwrite(VMCS_GUEST_DS_SELECTOR, readds());
		__vmx_vmwrite(VMCS_GUEST_DS_ACCESS_RIGHTS, ds_rights.flags);

		const auto [tr_descriptor, tr_rights, tr_limit, tr_base] =
			gdt::get_info(gdt_value, segment_selector{ readtr() });

		__vmx_vmwrite(VMCS_GUEST_TR_BASE, tr_base);
		__vmx_vmwrite(VMCS_GUEST_TR_LIMIT, tr_limit);
		__vmx_vmwrite(VMCS_GUEST_TR_SELECTOR, readtr());
		__vmx_vmwrite(VMCS_GUEST_TR_ACCESS_RIGHTS, tr_rights.flags);

		const auto [ldt_descriptor, ldt_rights, ldt_limit, ldt_base] =
			gdt::get_info(gdt_value, segment_selector{ readldt() });

		__vmx_vmwrite(VMCS_GUEST_LDTR_BASE, ldt_base);
		__vmx_vmwrite(VMCS_GUEST_LDTR_LIMIT, ldt_limit);
		__vmx_vmwrite(VMCS_GUEST_LDTR_SELECTOR, readldt());
		__vmx_vmwrite(VMCS_GUEST_LDTR_ACCESS_RIGHTS, ldt_rights.flags);

		__vmx_vmwrite(VMCS_GUEST_ACTIVITY_STATE, 0);
		__vmx_vmwrite(VMCS_GUEST_GS_BASE, __readmsr(IA32_GS_BASE));
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

			msr_fix_value.flags = __readmsr(IA32_VMX_TRUE_PROCBASED_CTLS);
			procbased_ctls.activate_secondary_controls = true;
			procbased_ctls.flags &= msr_fix_value.allowed_1_settings;
			procbased_ctls.flags |= msr_fix_value.allowed_0_settings;
			__vmx_vmwrite(VMCS_CTRL_PROCESSOR_BASED_VM_EXECUTION_CONTROLS, procbased_ctls.flags);

			msr_fix_value.flags = __readmsr(IA32_VMX_TRUE_ENTRY_CTLS);
			entry_ctls.ia32e_mode_guest = true;
			entry_ctls.conceal_vmx_from_pt = true;
			entry_ctls.flags &= msr_fix_value.allowed_1_settings;
			entry_ctls.flags |= msr_fix_value.allowed_0_settings;
			__vmx_vmwrite(VMCS_CTRL_VMENTRY_CONTROLS, entry_ctls.flags);

			msr_fix_value.flags = __readmsr(IA32_VMX_TRUE_EXIT_CTLS);
			exit_ctls.host_address_space_size = true;
			exit_ctls.flags &= msr_fix_value.allowed_1_settings;
			exit_ctls.flags |= msr_fix_value.allowed_0_settings;
			__vmx_vmwrite(VMCS_CTRL_VMEXIT_CONTROLS, exit_ctls.flags);
		}
		else
		{
			msr_fix_value.flags = __readmsr(IA32_VMX_PINBASED_CTLS);
			pinbased_ctls.flags &= msr_fix_value.allowed_1_settings;
			pinbased_ctls.flags |= msr_fix_value.allowed_0_settings;
			__vmx_vmwrite(VMCS_CTRL_PIN_BASED_VM_EXECUTION_CONTROLS, pinbased_ctls.flags);

			msr_fix_value.flags = __readmsr(IA32_VMX_PROCBASED_CTLS);
			procbased_ctls.activate_secondary_controls = true;
			procbased_ctls.flags &= msr_fix_value.allowed_1_settings;
			procbased_ctls.flags |= msr_fix_value.allowed_0_settings;
			__vmx_vmwrite(VMCS_CTRL_PROCESSOR_BASED_VM_EXECUTION_CONTROLS, procbased_ctls.flags);

			msr_fix_value.flags = __readmsr(IA32_VMX_ENTRY_CTLS);
			entry_ctls.ia32e_mode_guest = true;
			entry_ctls.conceal_vmx_from_pt = true;
			entry_ctls.flags &= msr_fix_value.allowed_1_settings;
			entry_ctls.flags |= msr_fix_value.allowed_0_settings;
			__vmx_vmwrite(VMCS_CTRL_VMENTRY_CONTROLS, entry_ctls.flags);

			msr_fix_value.flags = __readmsr(IA32_VMX_EXIT_CTLS);
			exit_ctls.host_address_space_size = true;
			exit_ctls.conceal_vmx_from_pt = true;
			exit_ctls.flags &= msr_fix_value.allowed_1_settings;
			exit_ctls.flags |= msr_fix_value.allowed_0_settings;
			__vmx_vmwrite(VMCS_CTRL_VMEXIT_CONTROLS, exit_ctls.flags);
		}

		msr_fix_value.flags = __readmsr(IA32_VMX_PROCBASED_CTLS2);
		procbased_ctls2.enable_rdtscp = true;
		procbased_ctls2.enable_xsaves = true;
		procbased_ctls2.conceal_vmx_from_pt = true; 
		procbased_ctls2.flags &= msr_fix_value.allowed_1_settings;
		procbased_ctls2.flags |= msr_fix_value.allowed_0_settings;
		__vmx_vmwrite(VMCS_CTRL_SECONDARY_PROCESSOR_BASED_VM_EXECUTION_CONTROLS, procbased_ctls2.flags);
	}
}