#include "vmxexit_handler.h"

auto get_command(u64 dirbase, u64 command_ptr) -> vmcall_command_t
{
	const auto virt_map =
		mm::map_virt(dirbase, command_ptr);

	if (!virt_map)
		return {};

	return *reinterpret_cast<pvmcall_command_t>(virt_map);
}

auto set_command(u64 dirbase, u64 command_ptr, const vmcall_command_t& vmcall_command) -> void
{
	const auto virt_map = 
		mm::map_virt(dirbase, command_ptr);

	if (!virt_map)
		return;

	*reinterpret_cast<pvmcall_command_t>(virt_map) = vmcall_command;
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
		break;
	}
	case VMX_EXIT_REASON_NMI_WINDOW:
	{
		vmentry_interrupt_information interrupt{};
		interrupt.interruption_type = interruption_type::non_maskable_interrupt;
		interrupt.vector = EXCEPTION_NMI;
		interrupt.valid = true;

		__vmx_vmwrite(VMCS_CTRL_VMENTRY_INTERRUPTION_INFORMATION_FIELD, interrupt.flags);
		__vmx_vmwrite(VMCS_VMEXIT_INTERRUPTION_ERROR_CODE, NULL);

		ia32_vmx_procbased_ctls_register procbased_ctls;
		ia32_vmx_pinbased_ctls_register pinbased_ctls;

		__vmx_vmread(VMCS_CTRL_PROCESSOR_BASED_VM_EXECUTION_CONTROLS, &procbased_ctls.flags);
		__vmx_vmread(VMCS_CTRL_PIN_BASED_VM_EXECUTION_CONTROLS, &pinbased_ctls.flags);

		procbased_ctls.nmi_window_exiting = false;
		pinbased_ctls.virtual_nmi = false;

		__vmx_vmwrite(VMCS_CTRL_PROCESSOR_BASED_VM_EXECUTION_CONTROLS, procbased_ctls.flags);
		__vmx_vmwrite(VMCS_CTRL_PIN_BASED_VM_EXECUTION_CONTROLS, pinbased_ctls.flags);
		return; // dont advance rip...
	}
	case VMX_EXIT_REASON_EXCEPTION_OR_NMI:
	{
		ia32_vmx_procbased_ctls_register procbased_ctls;
		ia32_vmx_pinbased_ctls_register pinbased_ctls;

		__vmx_vmread(VMCS_CTRL_PROCESSOR_BASED_VM_EXECUTION_CONTROLS, &procbased_ctls.flags);
		__vmx_vmread(VMCS_CTRL_PIN_BASED_VM_EXECUTION_CONTROLS, &pinbased_ctls.flags);

		procbased_ctls.nmi_window_exiting = true;
		pinbased_ctls.virtual_nmi = true;

		__vmx_vmwrite(VMCS_CTRL_PROCESSOR_BASED_VM_EXECUTION_CONTROLS, procbased_ctls.flags);
		__vmx_vmwrite(VMCS_CTRL_PIN_BASED_VM_EXECUTION_CONTROLS, pinbased_ctls.flags);
		return; // dont advance rip...
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
			interrupt.interruption_type = interruption_type::hardware_exception;
			interrupt.vector = EXCEPTION_GP_FAULT;

			interrupt.valid = true;
			interrupt.deliver_error_code = true;

			__vmx_vmwrite(VMCS_CTRL_VMENTRY_INTERRUPTION_INFORMATION_FIELD, interrupt.flags);
			__vmx_vmwrite(VMCS_VMEXIT_INTERRUPTION_ERROR_CODE, g_vcpu->error_code);
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
			interrupt.interruption_type = interruption_type::hardware_exception;
			interrupt.vector = EXCEPTION_GP_FAULT;

			interrupt.valid = true;
			interrupt.deliver_error_code = true;

			__vmx_vmwrite(VMCS_CTRL_VMENTRY_INTERRUPTION_INFORMATION_FIELD, interrupt.flags);
			__vmx_vmwrite(VMCS_VMEXIT_INTERRUPTION_ERROR_CODE, g_vcpu->error_code);
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
			/*
				EXCEPTION WARNING:
				#GP(0)	If the current privilege level is not 0.
						If the value in ECX specifies a reserved or unimplemented MSR address.
				#UD	If the LOCK prefix is used.
			*/
			__writemsr(regs->rcx, value.value);
			break;
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			vmentry_interrupt_information interrupt{};
			interrupt.interruption_type = interruption_type::hardware_exception;
			interrupt.vector = EXCEPTION_GP_FAULT;

			interrupt.valid = true;
			interrupt.deliver_error_code = true;

			__vmx_vmwrite(VMCS_CTRL_VMENTRY_INTERRUPTION_INFORMATION_FIELD, interrupt.flags);
			__vmx_vmwrite(VMCS_VMEXIT_INTERRUPTION_ERROR_CODE, g_vcpu->error_code);
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
	case VMX_EXIT_REASON_EXECUTE_VMCALL:
	{
		if (regs->rcx == VMCALL_KEY)
		{
			cr3 dirbase;
			__vmx_vmread(VMCS_GUEST_CR3, &dirbase.flags);

			auto command = get_command(
				dirbase.pml4_pfn << 12, regs->rdx);

			if (command.present)
			{
				switch (command.option)
				{
				case vmcall_option::copy_virt:
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
				case vmcall_option::translate:
				{
					command.translate.phys_addr = 
						mm::translate(mm::virt_addr_t{ 
							command.translate.virt_addr }, 
							command.translate.dirbase);

					// true if address is not null...
					command.result = command.translate.phys_addr;
					break;
				}
				case vmcall_option::read_phys:
				{
					command.result = 
						mm::read_phys(
							command.read_phys.dirbase_dest, 
							command.read_phys.phys_src, 
							command.read_phys.virt_dest, 
							command.read_phys.size);
					break;
				}
				case vmcall_option::write_phys:
				{
					command.result = 
						mm::write_phys(
							command.write_phys.dirbase_src, 
							command.write_phys.phys_dest, 
							command.write_phys.virt_src, 
							command.write_phys.size);
					break;
				}
				case vmcall_option::dirbase:
				{
					command.result = true;
					command.dirbase = dirbase.pml4_pfn << 12;
					break;
				}
				default:
					// check to see why the option was invalid...
					__debugbreak();
					break;
				}

				set_command(dirbase.pml4_pfn << 12, regs->rdx, command);
			}
		}
		else
		{
			vmentry_interrupt_information interrupt{};
			interrupt.interruption_type = interruption_type::hardware_exception;
			interrupt.vector = EXCEPTION_INVALID_OPCODE;
			interrupt.valid = true;

			__vmx_vmwrite(VMCS_CTRL_VMENTRY_INTERRUPTION_INFORMATION_FIELD, interrupt.flags);
			__vmx_vmwrite(VMCS_VMEXIT_INTERRUPTION_ERROR_CODE, NULL);
			return; // dont advance rip...
		}
		break;
	}
	case VMX_EXIT_REASON_EXECUTE_VMWRITE:
	case VMX_EXIT_REASON_EXECUTE_VMREAD:
	case VMX_EXIT_REASON_EXECUTE_VMPTRST:
	case VMX_EXIT_REASON_EXECUTE_VMPTRLD:
	case VMX_EXIT_REASON_EXECUTE_VMCLEAR:
	case VMX_EXIT_REASON_EXECUTE_VMXOFF:
	case VMX_EXIT_REASON_EXECUTE_VMXON:
	{
		vmentry_interrupt_information interrupt{};
		interrupt.interruption_type = interruption_type::hardware_exception;
		interrupt.vector = EXCEPTION_INVALID_OPCODE;
		interrupt.valid = true;

		__vmx_vmwrite(VMCS_CTRL_VMENTRY_INTERRUPTION_INFORMATION_FIELD, interrupt.flags);
		__vmx_vmwrite(VMCS_VMEXIT_INTERRUPTION_ERROR_CODE, NULL);
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