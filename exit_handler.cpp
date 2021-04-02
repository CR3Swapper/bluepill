#include "vmxexit_handler.h"

auto vmresume_failure() -> void
{
	size_t value;
	__vmx_vmread(VMCS_VM_INSTRUCTION_ERROR, &value);
	dbg::print("> vmresume error... reason = 0x%x\n", value);
}

auto exit_handler(hv::pguest_registers regs) -> void
{
	u64 exit_reason;
	__vmx_vmread(VMCS_EXIT_REASON, &exit_reason);

	switch (exit_reason)
	{
	case VMX_EXIT_REASON_EXECUTE_CPUID:
	{
		int result[4];
		__cpuid(result, regs->rax);
		regs->rax = result[0];
		regs->rbx = result[1];
		regs->rcx = result[2];
		regs->rdx = result[3];
		goto advance_rip;
	}
	case VMX_EXIT_REASON_NMI_WINDOW:
	{
		exception::injection(interruption_type::non_maskable_interrupt, EXCEPTION_NMI);

		// turn off NMI window exiting since we handled the NMI...
		ia32_vmx_procbased_ctls_register procbased_ctls;
		__vmx_vmread(VMCS_CTRL_PROCESSOR_BASED_VM_EXECUTION_CONTROLS, &procbased_ctls.flags);

		procbased_ctls.nmi_window_exiting = false;
		__vmx_vmwrite(VMCS_CTRL_PROCESSOR_BASED_VM_EXECUTION_CONTROLS, procbased_ctls.flags);
		goto dont_advance;
	}
	case VMX_EXIT_REASON_EXECUTE_XSETBV:
	{
		if (handle::xsetbv(regs))
			goto advance_rip;
		else
			goto dont_advance;
	}
	case VMX_EXIT_REASON_EXECUTE_RDMSR:
	{
		if (handle::rdmsr(regs))
			goto advance_rip;
		else
			goto dont_advance;
	}
	case VMX_EXIT_REASON_EXECUTE_WRMSR:
	{
		if (handle::wrmsr(regs))
			goto advance_rip;
		else
			goto dont_advance;
	}
	case VMX_EXIT_REASON_EXECUTE_VMCALL:
	{
		if (handle::vmcall(regs))
			goto advance_rip;
		else
			goto dont_advance;
	}
	case VMX_EXIT_REASON_EXECUTE_INVD:
	{
		__wbinvd();
		goto advance_rip;
	}
	case VMX_EXIT_REASON_EXECUTE_VMWRITE:
	case VMX_EXIT_REASON_EXECUTE_VMREAD:
	case VMX_EXIT_REASON_EXECUTE_VMPTRST:
	case VMX_EXIT_REASON_EXECUTE_VMPTRLD:
	case VMX_EXIT_REASON_EXECUTE_VMCLEAR:
	case VMX_EXIT_REASON_EXECUTE_VMXOFF:
	case VMX_EXIT_REASON_EXECUTE_VMXON:
	case VMX_EXIT_REASON_EXECUTE_VMFUNC:
	{
		exception::injection(interruption_type::hardware_exception, EXCEPTION_INVALID_OPCODE);
		goto dont_advance;
	}
	default:
		// TODO: check out the vmexit reason and add support for it...
		__debugbreak();
	}
	
advance_rip:
	size_t rip, exec_len;
	__vmx_vmread(VMCS_GUEST_RIP, &rip);
	__vmx_vmread(VMCS_VMEXIT_INSTRUCTION_LENGTH, &exec_len);
	__vmx_vmwrite(VMCS_GUEST_RIP, rip + exec_len);

	// since we are advancing RIP, also check if TF = 1, if so, set pending #DB...
	// otherwise this #DB will fire on the wrong instruction... please refer to:
	// https://howtohypervise.blogspot.com/2019/01/a-common-missight-in-most-hypervisors.html
	exception::handle_debug();

dont_advance:
	return;
}