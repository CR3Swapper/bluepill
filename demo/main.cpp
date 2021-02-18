#include "bluepill.h"
#include "vdm_ctx/vdm_ctx.hpp"

auto __cdecl main(int argc, char** argv) -> void
{
	vdm::read_phys_t _read_phys =
		[&](void* addr, void* buffer, std::size_t size) -> bool
	{
		return bluepill::read_phys(
			reinterpret_cast<u64>(addr), buffer, size);
	};

	vdm::write_phys_t _write_phys =
		[&](void* addr, void* buffer, std::size_t size) -> bool
	{
		return bluepill::write_phys(
			reinterpret_cast<u64>(addr), buffer, size);
	};

	vdm::vdm_ctx vdm(_read_phys, _write_phys);
	const auto ntoskrnl_base =
		reinterpret_cast<void*>(
			util::get_kmodule_base("ntoskrnl.exe"));

	const auto ntoskrnl_memcpy =
		util::get_kmodule_export("ntoskrnl.exe", "memcpy");

	std::printf("[+] %s physical address -> 0x%p\n", vdm::syscall_hook.first, vdm::syscall_address.load());
	std::printf("[+] %s page offset -> 0x%x\n", vdm::syscall_hook.first, vdm::nt_page_offset);
	std::printf("[+] ntoskrnl base address -> 0x%p\n", ntoskrnl_base);
	std::printf("[+] ntoskrnl memcpy address -> 0x%p\n", ntoskrnl_memcpy);

	short mz_bytes = 0;
	vdm.syscall<decltype(&memcpy)>(
		ntoskrnl_memcpy,
		&mz_bytes,
		ntoskrnl_base,
		sizeof mz_bytes
	);

	std::printf("[+] kernel MZ -> 0x%x\n", mz_bytes);
	std::getchar();

	const auto explorer_pid = util::get_pid("explorer.exe");
	const auto explorer_dirbase = vdm.get_dirbase(explorer_pid);
	const auto explorer_base = vdm.get_base_address(explorer_pid);

	std::printf("explorer.exe pid -> %d\n", explorer_pid);
	std::printf("explorer.exe dirbase -> 0x%p\n", explorer_dirbase);
	std::printf("explorer.exe base address -> 0x%p\n", explorer_base);
	std::printf("explorer.exe MZ -> 0x%x\n", bluepill::rpm<short>(explorer_dirbase, explorer_base));
	std::getchar();
}