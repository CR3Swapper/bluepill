#pragma once
#include <windows.h>
#include <string_view>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <functional>
#include "../util/util.hpp"

namespace vdm
{
	// change this to whatever you want :^)
	constexpr std::pair<const char*, const char*> syscall_hook = { "NtShutdownSystem", "ntdll.dll" };
	inline std::atomic<bool> is_page_found = false;
	inline std::atomic<void*> syscall_address = nullptr;
	inline std::uint16_t nt_page_offset;
	inline std::uint32_t nt_rva;
	inline std::uint8_t* ntoskrnl;

	using read_phys_t = std::function<bool(void* addr, void* buffer, std::size_t size)>;
	using write_phys_t = std::function<bool(void* addr, void* buffer, std::size_t size)>;

	class vdm_ctx
	{
	public:
		explicit vdm_ctx(read_phys_t& read_func, write_phys_t& write_func);
		void rkm(void* dst, void* src, std::size_t size);
		void wkm(void* dst, void* src, std::size_t size);

		auto get_peprocess(std::uint32_t pid) -> PEPROCESS;
		auto get_dirbase(std::uint32_t pid) -> std::uintptr_t;
		auto get_base_address(std::uint32_t pid) -> std::uintptr_t;

		template <class T, class ... Ts>
		std::invoke_result_t<T, Ts...> syscall(void* addr, Ts ... args) const
		{
			static const auto proc =
				GetProcAddress(
					LoadLibraryA(syscall_hook.second),
					syscall_hook.first
				);

			static std::mutex syscall_mutex;
			syscall_mutex.lock();

			// jmp [rip+0x0]
			std::uint8_t jmp_code[] =
			{
				0xff, 0x25, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00,
				0x00, 0x00
			};

			std::uint8_t orig_bytes[sizeof jmp_code];
			*reinterpret_cast<void**>(jmp_code + 6) = addr;
			read_phys(vdm::syscall_address.load(), orig_bytes, sizeof orig_bytes);

			// execute hook...
			write_phys(vdm::syscall_address.load(), jmp_code, sizeof jmp_code);
			auto result = reinterpret_cast<T>(proc)(args ...);
			write_phys(vdm::syscall_address.load(), orig_bytes, sizeof orig_bytes);

			syscall_mutex.unlock();
			return result;
		}

		template <class T>
		auto rkm(std::uintptr_t addr) -> T
		{
			T buffer;
			rkm((void*)&buffer, (void*)addr, sizeof T);
			return buffer;
		}

		template <class T>
		void wkm(std::uintptr_t addr, const T& value)
		{
			wkm((void*)addr, (void*)&value, sizeof T);
		}

		read_phys_t read_phys;
		write_phys_t write_phys;
	private:
		void locate_syscall(std::uintptr_t begin, std::uintptr_t end) const;
		bool valid_syscall(void* syscall_addr) const;
	};
}