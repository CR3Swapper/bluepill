#include "vmxlaunch.hpp"

auto vmxlaunch::init_vmcs() -> void
{
	const auto vcpu = 
		vmxon::g_vmx_ctx->vcpus[
			KeGetCurrentProcessorNumber()];

	__vmx_vmclear(&vcpu->vmcs_phys);
	__vmx_vmptrld(&vcpu->vmcs_phys);

	vmcs::setup_host(&vmxexit_handler, vcpu->host_stack);
	vmcs::setup_guest();
	vmcs::setup_controls();
}

auto vmxlaunch::launch() -> void
{
	const auto vmlaunch_result = __vmx_vmlaunch();
	DBG_PRINT("vmxlaunch for processor: %d\n", KeGetCurrentProcessorNumber());
	DBG_PRINT("		- vmxlaunch result (0 == success): %d\n", vmlaunch_result);

	if (vmlaunch_result)
	{
		u64 vmxerror;
		__vmx_vmread(VMCS_VM_INSTRUCTION_ERROR, &vmxerror);
		DBG_PRINT("vmxerror: %d\n", vmxerror);
	}
}