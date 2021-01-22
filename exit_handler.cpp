#include "vmxexit_handler.h"

auto exit_handler(hv::pguest_registers regs) -> void
{
	u64 exit_reason;
	__vmx_vmread(VMCS_EXIT_REASON, &exit_reason);

	switch (exit_reason)
	{
	case VMX_EXIT_REASON_EXECUTE_CPUID:
	case VMX_EXIT_REASON_EXECUTE_INVLPG:
	case VMX_EXIT_REASON_EXECUTE_XSETBV:
	case VMX_EXIT_REASON_EXECUTE_HLT:
	case VMX_EXIT_REASON_EXECUTE_VMCALL:
	case VMX_EXIT_REASON_EXECUTE_VMREAD:
	case VMX_EXIT_REASON_EXECUTE_VMWRITE:
	case VMX_EXIT_REASON_EXECUTE_VMPTRLD:
	case VMX_EXIT_REASON_EXECUTE_VMPTRST:
	case VMX_EXIT_REASON_EXECUTE_VMCLEAR:
	case VMX_EXIT_REASON_EXECUTE_RDTSC:
	default:
		__vmx_off();
	}
}