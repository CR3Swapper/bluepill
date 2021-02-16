#include "bluepill.h"
#include "vdm_ctx/vdm_ctx.hpp"

auto __cdecl main(int argc, char** argv) -> void
{
	vdm::read_phys_t _read_phys =
		[&](void* addr, void* buffer, std::size_t size) -> bool
	{
		return bluepill::read_phys(buffer, addr, size);
	};

	vdm::write_phys_t _write_phys =
		[&](void* addr, void* buffer, std::size_t size) -> bool
	{
		return bluepill::write_phys(addr, buffer, size);
	};
	
	const auto dirbase = 
		reinterpret_cast<void*>(
			bluepill::get_dirbase());

	std::printf("current dirbase -> 0x%p\n", dirbase);
	std::getchar();

	const auto nt_shutdown_phys =
		bluepill::translate(dirbase,
			util::get_kmodule_export("ntoskrnl.exe",
				vdm::syscall_hook.second));

	std::printf("NtShutdownSystem translated (phys) -> 0x%p\n", nt_shutdown_phys);
	std::getchar();

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
}