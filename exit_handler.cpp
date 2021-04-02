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
	// shouldnt get an exit when the LP is already executing an NMI...
	// so it should be safe to inject an NMI here...
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
			goto advance_rip;
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			exception::injection(interruption_type::hardware_exception,
				EXCEPTION_GP_FAULT, { true, g_vcpu.error_code });
			goto dont_advance;
		}
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
			goto advance_rip;
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			exception::injection(interruption_type::hardware_exception,
				EXCEPTION_GP_FAULT, { true, g_vcpu.error_code });
			goto dont_advance;
		}
	}
	case VMX_EXIT_REASON_EXECUTE_WRMSR:
	{
		hv::msr_split value;
		value.low = regs->rax;
		value.high = regs->rdx;

		__try
		{
			/*
				EXCEPTION WARNING:
				#GP(0)	If the current privilege level is not 0.
						If the value in ECX specifies a reserved or unimplemented MSR address.
				#UD	If the LOCK prefix is used.
			*/
			__writemsr(regs->rcx, value.value);
			goto advance_rip;
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			exception::injection(interruption_type::hardware_exception,
				EXCEPTION_GP_FAULT, { true, g_vcpu.error_code });
			goto dont_advance;
		}
	}
	case VMX_EXIT_REASON_EXECUTE_INVD:
	{
		__wbinvd();
		goto advance_rip;
	}
	case VMX_EXIT_REASON_EXECUTE_VMCALL:
	{
		if (regs->rcx == VMCALL_KEY)
		{
			cr3 dirbase;
			__vmx_vmread(VMCS_GUEST_CR3, &dirbase.flags);

			auto command = command::get(
				dirbase.pml4_pfn << 12, regs->rdx);

			if (!command.present)
			{
				exception::injection(interruption_type::hardware_exception, EXCEPTION_INVALID_OPCODE);
				goto dont_advance;
			}

			switch (command.option)
			{
			case command::vmcall_option::copy_virt:
			{
				command.result =
					mm::copy_virt(
						command.copy_virt.dirbase_src,
						command.copy_virt.virt_src,
						command.copy_virt.dirbase_dest,
						command.copy_virt.virt_dest,
						command.copy_virt.size);
				break;
			}
			case command::vmcall_option::translate:
			{
				command.translate.phys_addr =
					mm::translate(mm::virt_addr_t{
						command.translate.virt_addr },
						command.translate.dirbase);

				// true if address is not null...
				command.result = command.translate.phys_addr;
				break;
			}
			case command::vmcall_option::read_phys:
			{
				command.result =
					mm::read_phys(
						command.read_phys.dirbase_dest,
						command.read_phys.phys_src,
						command.read_phys.virt_dest,
						command.read_phys.size);
				break;
			}
			case command::vmcall_option::write_phys:
			{
				command.result =
					mm::write_phys(
						command.write_phys.dirbase_src,
						command.write_phys.phys_dest,
						command.write_phys.virt_src,
						command.write_phys.size);
				break;
			}
			case command::vmcall_option::dirbase:
			{
				command.result = true;
				command.dirbase = dirbase.pml4_pfn << 12;
				break;
			}
			default:
				// check to see why the option was invalid...
				__debugbreak();
			}

			command::set(dirbase.pml4_pfn << 12, regs->rdx, command);
			goto advance_rip;
		}
		else
		{
			exception::injection(interruption_type::hardware_exception, EXCEPTION_INVALID_OPCODE);
			goto dont_advance;
		}
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