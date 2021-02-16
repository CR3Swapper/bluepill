#include "bluepill.h"

namespace bluepill
{
	auto get_dirbase() -> u64
	{
		vmcall_command_t command{};
		memset(&command, NULL, sizeof command);

		command.present = true;
		command.option = vmcall_option::dirbase;

		hypercall(VMCALL_KEY, &command);
		return command.dirbase;
	}

	auto translate(void* dirbase, void* virt_addr) -> u64
	{
		vmcall_command_t command{};
		memset(&command, NULL, sizeof command);

		command.present = true;
		command.option = vmcall_option::translate;
		command.translate.dirbase = reinterpret_cast<u64>(dirbase);
		command.translate.virt_addr = reinterpret_cast<u64>(virt_addr);

		hypercall(VMCALL_KEY, &command);
		return command.translate.phys_addr;
	}

	auto read_phys(void* dest, void* phys_src, u64 size) -> bool
	{
		vmcall_command_t command{};
		memset(&command, NULL, sizeof command);

		command.present = true;
		command.option = vmcall_option::read_phys;
		command.read_phys.virt_dest = reinterpret_cast<u64>(dest);
		command.read_phys.phys_src = reinterpret_cast<u64>(phys_src);
		command.read_phys.dirbase_dest = get_dirbase();
		command.read_phys.size = size;

		hypercall(VMCALL_KEY, &command);
		return command.result;
	}

	auto write_phys(void* phys_dest, void* src, u64 size) -> bool
	{
		vmcall_command_t command{};
		memset(&command, NULL, sizeof command);

		command.present = true;
		command.option = vmcall_option::write_phys;
		command.write_phys.virt_src = reinterpret_cast<u64>(src);
		command.write_phys.phys_dest = reinterpret_cast<u64>(phys_dest);
		command.write_phys.dirbase_src = get_dirbase();
		command.write_phys.size = size;

		hypercall(VMCALL_KEY, &command);
		return command.result;
	}

	auto copy_virt(void* dirbase_src, void* virt_src, void* dirbase_dest, void* virt_dest, u64 size) -> bool
	{
		vmcall_command_t command{};
		memset(&command, NULL, sizeof command);

		command.present = true;
		command.copy_virt.dirbase_dest = reinterpret_cast<u64>(dirbase_dest);
		command.copy_virt.virt_dest = reinterpret_cast<u64>(virt_dest);
		command.copy_virt.dirbase_src = reinterpret_cast<u64>(dirbase_src);
		command.copy_virt.virt_src = reinterpret_cast<u64>(virt_src);
		command.copy_virt.size = size;

		hypercall(VMCALL_KEY, &command);
		return command.result;
	}
}