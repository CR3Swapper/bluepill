#include "command.hpp"

namespace command
{
	auto get(u64 dirbase, u64 command_ptr) -> vmcall_command_t
	{
		const auto virt_map =
			mm::map_virt(dirbase, command_ptr);

		if (!virt_map)
			return {};

		return *reinterpret_cast<pvmcall_command_t>(virt_map);
	}

	auto set(u64 dirbase, u64 command_ptr, const vmcall_command_t& vmcall_command) -> void
	{
		const auto virt_map =
			mm::map_virt(dirbase, command_ptr);

		if (!virt_map)
			return;

		*reinterpret_cast<pvmcall_command_t>(virt_map) = vmcall_command;
	}
}