#pragma once
#include "mm.hpp"

namespace command
{
	enum class vmcall_option
	{
		translate,
		copy_virt,
		write_phys,
		read_phys,
		dirbase
	};

	typedef struct _vmcall_command_t
	{
		bool present;
		bool result;
		vmcall_option option;

		union
		{
			struct
			{
				u64 dirbase;
				u64 virt_addr;
				u64 phys_addr;
			} translate;

			struct
			{
				u64 virt_src;
				u64 dirbase_src;
				u64 virt_dest;
				u64 dirbase_dest;
				u64 size;
			} copy_virt;

			struct
			{
				u64 virt_src;
				u64 dirbase_src;
				u64 phys_dest;
				u64 size;
			} write_phys;

			struct
			{
				u64 phys_src;
				u64 dirbase_dest;
				u64 virt_dest;
				u64 size;
			} read_phys;

			u64 dirbase;
		};

	} vmcall_command_t, * pvmcall_command_t;

	auto get(u64 dirbase, u64 command_ptr)->vmcall_command_t;
	auto set(u64 dirbase, u64 command_ptr, const vmcall_command_t& vmcall_command) -> void;
}