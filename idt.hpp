#pragma once
#include "hv_types.hpp"
#include "segment_intrin.h"
#include "debug.hpp"

#pragma section(".idt", read, write)
extern "C" void __gp_handler(void);
extern "C" void __pf_handler(void);
extern "C" void seh_handler(hv::pidt_regs_t regs);

namespace idt
{
	__declspec(allocate(".idt")) 
	inline hv::idt_entry_t table[256];
	enum ist_idx : u8 { de = 4, pf = 5, gp = 6};

	inline void* image_base = nullptr; // used for SEH...
	auto create_entry(hv::idt_addr_t idt_handler, u8 ist_index) -> hv::idt_entry_t;
}