#include "vmxlaunch.hpp"

auto vmxlaunch::init_vmcs() -> void
{
	__vmx_vmclear(&vmxon::g_vmx_ctx->vcpus[
		KeGetCurrentProcessorNumber()]->vmcs_phys);

	__vmx_vmptrld(&vmxon::g_vmx_ctx->vcpus[
		KeGetCurrentProcessorNumber()]->vmcs_phys);

	// setup host VMCS fields...
	__vmx_vmwrite(VMCS_HOST_CS_SELECTOR, readcs() & 0xF8);
	__vmx_vmwrite(VMCS_HOST_DS_SELECTOR, readds() & 0xF8);
	__vmx_vmwrite(VMCS_HOST_ES_SELECTOR, reades() & 0xF8);
	__vmx_vmwrite(VMCS_HOST_GS_SELECTOR, readgs() & 0xF8);
	__vmx_vmwrite(VMCS_HOST_SS_SELECTOR, readss() & 0xF8);
	__vmx_vmwrite(VMCS_HOST_FS_SELECTOR, readfs() & 0xF8);

	// TODO IDT, TR, GDT (base and limit for each segment register), and LDT...

	__vmx_vmwrite(VMCS_HOST_CR0, __readcr0());
	__vmx_vmwrite(VMCS_HOST_CR3, __readcr3());
	__vmx_vmwrite(VMCS_HOST_CR4, __readcr4());

	__vmx_vmwrite(VMCS_HOST_RIP, 
		reinterpret_cast<u64>(&::vmxexit_handler));

	__vmx_vmwrite(VMCS_HOST_RSP, 
		vmxon::g_vmx_ctx->vcpus[
			KeGetCurrentProcessorNumber()]->host_stack);

	// setup guest VMCS fields...
	__vmx_vmwrite(VMCS_GUEST_CS_SELECTOR, readcs());
	__vmx_vmwrite(VMCS_GUEST_DS_SELECTOR, readds());
	__vmx_vmwrite(VMCS_GUEST_ES_SELECTOR, reades());
	__vmx_vmwrite(VMCS_GUEST_GS_SELECTOR, readgs());
	__vmx_vmwrite(VMCS_GUEST_SS_SELECTOR, readss());
	__vmx_vmwrite(VMCS_GUEST_FS_SELECTOR, readfs());
	__vmx_vmwrite(VMCS_GUEST_VMCS_LINK_POINTER, ~0ULL);

	ia32_vmx_exit_ctls_register exit_ctls;
	exit_ctls.save_ia32_efer = true;
	exit_ctls.conceal_vmx_from_pt = false;
	exit_ctls.host_address_space_size = true;
	exit_ctls.load_ia32_efer = false; // TODO readup on this...
	exit_ctls.load_ia32_pat = true; // TODO ask daax about this...
	exit_ctls.save_debug_controls = true;
	exit_ctls.load_ia32_perf_global_ctrl = true;
	exit_ctls.save_vmx_preemption_timer_value = true;
	__vmx_vmwrite(VMCS_CTRL_VMEXIT_CONTROLS, exit_ctls.flags);

	ia32_vmx_procbased_ctls2_register procbased_ctls;
	procbased_ctls.apic_register_virtualization = false;
	procbased_ctls.conceal_vmx_from_pt = true;
	procbased_ctls.descriptor_table_exiting = false;
	procbased_ctls.enable_encls_exiting = false;
	procbased_ctls.enable_ept = false; 
	procbased_ctls.enable_invpcid = false;
	procbased_ctls.enable_pml = false;
	procbased_ctls.enable_rdtscp = false; // might need to enable this...
	procbased_ctls.enable_vm_functions = false; 
	procbased_ctls.enable_vpid = true; // TODO read up on this...
	procbased_ctls.enable_xsaves = false; // TODO not sure if i need this enabled...
	procbased_ctls.ept_violation = false; 
	procbased_ctls.mode_based_execute_control_for_ept = false; 
	procbased_ctls.pause_loop_exiting = false; 
	procbased_ctls.rdrand_exiting = false; 
	procbased_ctls.rdseed_exiting = false; 
	procbased_ctls.unrestricted_guest = false; // TODO read up on this...
	procbased_ctls.use_tsc_scaling = true; // TODO read up on this...
	procbased_ctls.virtualize_apic_accesses = false;
	procbased_ctls.virtualize_x2apic_mode = false;
	procbased_ctls.virtual_interrupt_delivery = false;
	procbased_ctls.vmcs_shadowing = false;
	procbased_ctls.wbinvd_exiting = false; //TODO not sure if i need this to be true...
	__vmx_vmwrite(VMCS_CTRL_SECONDARY_PROCESSOR_BASED_VM_EXECUTION_CONTROLS, procbased_ctls.flags);

	ia32_vmx_pinbased_ctls_register pinbased_ctls;
	pinbased_ctls.activate_vmx_preemption_timer = false;
	pinbased_ctls.external_interrupt_exiting = false;
	pinbased_ctls.nmi_exiting = false;
	pinbased_ctls.process_posted_interrupts = false;
	pinbased_ctls.virtual_nmi = false;
	__vmx_vmwrite(VMCS_CTRL_PIN_BASED_VM_EXECUTION_CONTROLS, pinbased_ctls.flags);

	ia32_vmx_entry_ctls_register entry_ctls;
	entry_ctls.conceal_vmx_from_pt = true;
	entry_ctls.deactivate_dual_monitor_treatment = false;
	entry_ctls.entry_to_smm = false; // TODO ask daax about this...
	entry_ctls.ia32e_mode_guest = true;
	entry_ctls.load_cet_state = true; // TODO ask daax...
	entry_ctls.load_debug_controls = true; // TODO ask daax about this...
	entry_ctls.load_ia32_bndcfgs = false; // TODO ask daax about this...
	entry_ctls.load_ia32_efer = true; // TODO ask daax about this...
	entry_ctls.load_ia32_pat = true;
	entry_ctls.load_ia32_perf_global_ctrl = true; // TODO ask daax...
	entry_ctls.load_ia32_rtit_ctl = true; // TODO ask daax...
	__vmx_vmwrite(VMCS_CTRL_VMENTRY_CONTROLS, entry_ctls.flags);
}

auto vmxlaunch::launch() -> void
{
	DBG_PRINT("vmxlaunch for processor: %d\n", KeGetCurrentProcessorNumber());
	DBG_PRINT("		- vmxlaunch result (0 == success): %d\n", __vmx_vmlaunch());
}