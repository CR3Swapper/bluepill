#include "bluepill.h"

namespace bluepill
{
	auto get_dirbase() -> u64
	{
		vmcall_command_t command{};
		memset(&command, NULL, sizeof command);

		command.present = true;
		command.option = vmcall_option::dirbase;

		// can throw invalid opcode if hypervisor is not loaded...
		__try
		{
			hypercall(VMCALL_KEY, &command);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			return {};
		}
		return command.dirbase;
	}

	auto translate(u64 dirbase, void* virt_addr) -> u64
	{
		if (!dirbase || !virt_addr)
			return false;

		vmcall_command_t command{};
		memset(&command, NULL, sizeof command);

		command.present = true;
		command.option = vmcall_option::translate;
		command.translate.dirbase = dirbase;
		command.translate.virt_addr = reinterpret_cast<u64>(virt_addr);

		// can throw invalid opcode if hypervisor is not loaded...
		__try
		{
			hypercall(VMCALL_KEY, &command);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			return {};
		}
		return command.translate.phys_addr;
	}

	auto read_phys(u64 phys_src, void* virt_dest, u64 size) -> bool
	{
		if (!phys_src || !virt_dest || !size)
			return false;

		vmcall_command_t command{};
		memset(&command, NULL, sizeof command);

		command.present = true;
		command.option = vmcall_option::read_phys;
		command.read_phys.virt_dest = reinterpret_cast<u64>(virt_dest);
		command.read_phys.phys_src = phys_src;
		command.read_phys.dirbase_dest = get_dirbase();
		command.read_phys.size = size;

		// can throw invalid opcode if hypervisor is not loaded...
		__try
		{
			hypercall(VMCALL_KEY, &command);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			return false;
		}
		return command.result;
	}

	auto write_phys(u64 phys_dest, void* src, u64 size) -> bool
	{
		if (!phys_dest || !src || !size)
			return false;

		vmcall_command_t command{};
		memset(&command, NULL, sizeof command);

		command.present = true;
		command.option = vmcall_option::write_phys;
		command.write_phys.virt_src = reinterpret_cast<u64>(src);
		command.write_phys.phys_dest = phys_dest;
		command.write_phys.dirbase_src = get_dirbase();
		command.write_phys.size = size;

		// can throw invalid opcode if hypervisor is not loaded...
		__try
		{
			hypercall(VMCALL_KEY, &command);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			return false;
		}
		return command.result;
	}

	auto copy_virt(u64 dirbase_src, void* virt_src, u64 dirbase_dest, void* virt_dest, u64 size) -> bool
	{
		if (!dirbase_src || !virt_src || !dirbase_dest || !virt_dest)
			return false;

		vmcall_command_t command{};
		memset(&command, NULL, sizeof command);

		command.present = true;
		command.option = vmcall_option::copy_virt;
		command.copy_virt.dirbase_dest = dirbase_dest;
		command.copy_virt.virt_dest = reinterpret_cast<u64>(virt_dest);
		command.copy_virt.dirbase_src = dirbase_src;
		command.copy_virt.virt_src = reinterpret_cast<u64>(virt_src);
		command.copy_virt.size = size;

		// can throw invalid opcode if hypervisor is not loaded...
		__try
		{
			hypercall(VMCALL_KEY, &command);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			return false;
		}
		return command.result;
	}
}