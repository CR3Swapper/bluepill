#include "vmxexit_handler.h"

auto exit_handler(hv::pguest_registers regs) -> void
{
	u64 exit_reason;
	__vmx_vmread(VMCS_EXIT_REASON, &exit_reason);

	switch (exit_reason)
	{
	case VMX_EXIT_REASON_EXECUTE_CPUID:
	{
		if (regs->rcx == 0xC0FFEE)
		{
			__try
			{
				*(u8*)0x0 = 0xDE;
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				regs->rax = 0xC0FFEE;
				break;
			}
		}
		else
		{
			int result[4];
			__cpuid(result, regs->rax);
			regs->rax = result[0];
			regs->rbx = result[1];
			regs->rcx = result[2];
			regs->rdx = result[3];
		}
		break;
	}
	case VMX_EXIT_REASON_EXECUTE_XSETBV:
	{
		hv::msr_split value{};
		value.high = regs->rdx;
		value.low = regs->rax;

		__try
		{
			/*
				EXCEPTION WARNING:
				#GP		If the current privilege level is not 0.
						If an invalid XCR is specified in ECX.
						If the value in EDX:EAX sets bits that are reserved in the XCR specified by ECX.
						If an attempt is made to clear bit 0 of XCR0.
						If an attempt is made to set XCR0[2:1] to 10b.

				#UD		If CPUID.01H:ECX.XSAVE[bit 26] = 0.
						If CR4.OSXSAVE[bit 18] = 0.
						If the LOCK prefix is used.
			*/
			_xsetbv(regs->rcx, value.value);
			break;
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			vmentry_interrupt_information interrupt{};
			interrupt.flags = interruption_type::hardware_exception;
			interrupt.vector = EXCEPTION_GP_FAULT;
			interrupt.valid = true;
			__vmx_vmwrite(VMCS_CTRL_VMENTRY_INTERRUPTION_INFORMATION_FIELD, interrupt.flags);
		}
		return; // dont advance rip...
	}
	case VMX_EXIT_REASON_EXECUTE_RDMSR:
	{
		__try
		{
			/*
				EXCEPTION WARNING:
				#GP(0)	If the current privilege level is not 0.
						If the value in ECX specifies a reserved or unimplemented MSR address.
				#UD	If the LOCK prefix is used.
			*/
			const auto result =
				hv::msr_split{ __readmsr(regs->rcx) };

			regs->rdx = result.high;
			regs->rax = result.low;
			break;
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			vmentry_interrupt_information interrupt{};
			interrupt.flags = interruption_type::hardware_exception;
			interrupt.vector = EXCEPTION_GP_FAULT;
			interrupt.valid = true;
			__vmx_vmwrite(VMCS_CTRL_VMENTRY_INTERRUPTION_INFORMATION_FIELD, interrupt.flags);
		}
		return; // dont advance rip...
	}
	case VMX_EXIT_REASON_EXECUTE_WRMSR:
	{
		hv::msr_split value;
		value.low = regs->rax;
		value.high = regs->rdx;

		__try
		{
			__writemsr(regs->rcx, value.value);
			break;
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			/*
				EXCEPTION WARNING:
				#GP(0)	If the current privilege level is not 0.
						If the value in ECX specifies a reserved or unimplemented MSR address.
				#UD	If the LOCK prefix is used.
			*/
			vmentry_interrupt_information interrupt{};
			interrupt.flags = interruption_type::hardware_exception;
			interrupt.vector = EXCEPTION_GP_FAULT;
			interrupt.valid = true;
			__vmx_vmwrite(VMCS_CTRL_VMENTRY_INTERRUPTION_INFORMATION_FIELD, interrupt.flags);
		}
		return; // dont advance rip...
	}
	case VMX_EXIT_REASON_EXECUTE_INVD:
	{
		// couldnt find the intrin for this so i just made one...
		// probably could have used __wbinvd?
		__invd();
		break;
	}
	case VMX_EXIT_REASON_EXECUTE_VMWRITE:
	case VMX_EXIT_REASON_EXECUTE_VMREAD:
	case VMX_EXIT_REASON_EXECUTE_VMPTRST:
	case VMX_EXIT_REASON_EXECUTE_VMPTRLD:
	case VMX_EXIT_REASON_EXECUTE_VMCLEAR:
	case VMX_EXIT_REASON_EXECUTE_VMXOFF:
	case VMX_EXIT_REASON_EXECUTE_VMXON:
	case VMX_EXIT_REASON_EXECUTE_VMCALL:
	{
		vmentry_interrupt_information interrupt{};
		interrupt.flags = interruption_type::hardware_exception;
		interrupt.vector = EXCEPTION_INVALID_OPCODE;
		interrupt.valid = true;
		__vmx_vmwrite(VMCS_CTRL_VMENTRY_INTERRUPTION_INFORMATION_FIELD, interrupt.flags);
		return; // dont advance rip...
	}
	default:
		// TODO: check out the vmexit reason and add support for it...
		__debugbreak();
		break;
	}

	size_t rip, exec_len;
	__vmx_vmread(VMCS_GUEST_RIP, &rip);
	__vmx_vmread(VMCS_VMEXIT_INSTRUCTION_LENGTH, &exec_len);
	__vmx_vmwrite(VMCS_GUEST_RIP, rip + exec_len);
}