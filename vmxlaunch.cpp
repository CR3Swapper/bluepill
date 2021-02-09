#include "vmxlaunch.hpp"

auto vmxlaunch::init_vmcs(cr3 cr3_value) -> void
{
	const auto vcpu = 
		vmxon::g_vmx_ctx->vcpus[
			KeGetCurrentProcessorNumber()];

	__vmx_vmclear(&vcpu->vmcs_phys);
	__vmx_vmptrld(&vcpu->vmcs_phys);

	vmcs::setup_host(&vmxexit_handler, vcpu->host_stack, cr3_value);
	vmcs::setup_guest();
	vmcs::setup_controls();
}

auto vmxlaunch::launch() -> void
{
	const auto vmlaunch_result = vmxlaunch_processor();
	DBG_PRINT("vmxlaunch for processor: %d\n", KeGetCurrentProcessorNumber());
	DBG_PRINT("		- vmxlaunch result: 0x%x\n", vmlaunch_result);

	if (vmlaunch_result != VMX_LAUNCH_SUCCESS)
	{
		u64 vmxerror;
		rflags flags{ vmlaunch_result };
		__vmx_vmread(VMCS_VM_INSTRUCTION_ERROR, &vmxerror);

		DBG_PRINT("vmxerror: %d\n", vmxerror);
		DBG_PRINT("eflags.zero_flag: %d\n", flags.zero_flag);
		DBG_PRINT("eflags.carry_flag: %d\n", flags.carry_flag);
	}
}