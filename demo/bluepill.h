#pragma once
#include <Windows.h>
#include <intrin.h>
#define VMCALL_KEY 0xC0FFEE

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;
using u64 = unsigned long long;
using u128 = __m128;

using s8 = char;
using s16 = short;
using s32 = int;
using s64 = long long;

namespace bluepill
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

	// vmcall into the hypervisor...
	extern "C" u64 hypercall(u64 key, pvmcall_command_t command);

	// get vmexiting logical processors pml4...
	auto get_dirbase() -> u64;

	// read/write physical memory...
	auto read_phys(void* dest, void* phys_src, u64 size) -> bool;
	auto write_phys(void* phys_dest, void* src, u64 size) -> bool;

	// translate virtual to physical...
	auto translate(void* dirbase, void* virt_addr)->u64;

	// copy virtual memory between two address spaces... page protections are ignored...
	auto copy_virt(void* dirbase_src, void* virt_src, void* dirbase_dest, void* virt_dest, u64 size) -> bool;
}