#pragma once
#include "hv_types.hpp"
#include "segment_intrin.h"
#pragma section(".idt", read, write)

extern "C" void __gp_handler(void);
extern "C" void __pf_handler(void);
extern "C" void __de_handler(void);
extern "C" void seh_handler(hv::pidt_regs_t regs);

namespace idt
{
	__declspec(allocate(".idt")) 
	inline hv::idt_entry_t table[256];

	auto create_entry(hv::idt_addr_t idt_handler) -> hv::idt_entry_t;
	auto init() -> u64;
}