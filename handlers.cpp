#include "handlers.hpp"

namespace handle
{
	auto xsetbv(hv::pguest_registers regs) -> bool
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
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			exception::injection(interruption_type::hardware_exception,
				EXCEPTION_GP_FAULT, { true, g_vcpu.error_code });
			return false;
		}
		return true;
	}

	auto rdmsr(hv::pguest_registers regs) -> bool
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
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			exception::injection(interruption_type::hardware_exception,
				EXCEPTION_GP_FAULT, { true, g_vcpu.error_code });
			return false;
		}
		return true;
	}

	auto wrmsr(hv::pguest_registers regs) -> bool
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
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			exception::injection(interruption_type::hardware_exception,
				EXCEPTION_GP_FAULT, { true, g_vcpu.error_code });
			return false;
		}
		return true;
	}

	auto vmcall(hv::pguest_registers regs) -> bool
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
				return false;
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
		}
		else
		{
			exception::injection(interruption_type::hardware_exception, EXCEPTION_INVALID_OPCODE);
			return false;
		}
		return true;
	}
}