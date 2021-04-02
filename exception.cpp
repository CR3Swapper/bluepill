#include "exception.hpp"

namespace exception
{
	auto handle_debug() -> void
	{
		rflags g_rflags;
		ia32_debugctl_register debugctl;

		__vmx_vmread(VMCS_GUEST_RFLAGS, &g_rflags.flags);
		__vmx_vmread(VMCS_GUEST_DEBUGCTL, &debugctl.flags);

		// should also check: if ((g_rflags.trap_flag && (debugctl.btf && instruction.type == branching))
		if (g_rflags.trap_flag && !debugctl.btf)
		{
			vmx_exit_qualification_debug_exception pending_db;
			__vmx_vmread(VMCS_GUEST_PENDING_DEBUG_EXCEPTIONS, &pending_db.flags);
			pending_db.single_instruction = true;
			__vmx_vmwrite(VMCS_GUEST_PENDING_DEBUG_EXCEPTIONS, pending_db.flags);
		}

		vmx_interruptibility_state interrupt_state;
		__vmx_vmread(VMCS_GUEST_INTERRUPTIBILITY_STATE,
			reinterpret_cast<size_t*>(&interrupt_state.flags));

		// not going to clear blocked by NMI or 
		// SMI stuff as IRETQ should unblock that... 
		// im not emulating IRETQ instruction either...
		interrupt_state.blocking_by_mov_ss = false;
		interrupt_state.blocking_by_sti = false;
		__vmx_vmwrite(VMCS_GUEST_INTERRUPTIBILITY_STATE, interrupt_state.flags);
	}

	auto injection(interruption_type type, u8 vector, ecode_t error_code) -> void
	{
		vmentry_interrupt_information interrupt{};
		interrupt.interruption_type = type;
		interrupt.vector = vector;
		interrupt.valid = true;

		if (error_code.valid)
		{
			interrupt.deliver_error_code = error_code.valid;
			__vmx_vmwrite(VMCS_VMEXIT_INTERRUPTION_ERROR_CODE, error_code.valid);
		}
		__vmx_vmwrite(VMCS_CTRL_VMENTRY_INTERRUPTION_INFORMATION_FIELD, interrupt.flags);
	}
}