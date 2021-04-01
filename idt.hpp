#pragma once
#include "hv_types.hpp"
#include "segment_intrin.h"
#include "debug.hpp"
#include "vmxon.hpp"

#pragma section(".idt", read, write)
#pragma section(".nmi_stk", read, write)
#pragma section(".pf_stk", read, write)
#pragma section(".de_stk", read, write)
#pragma section(".gp_stk", read, write)

extern "C" void __gp_handler(void);
extern "C" void __pf_handler(void);
extern "C" void __de_handler(void);
extern "C" void __nmi_handler(void);

extern "C" void nmi_handler(void);
extern "C" void seh_handler(hv::pidt_regs_t regs);
extern "C" void seh_handler_ecode(hv::pidt_regs_ecode_t regs);

namespace idt
{
	__declspec(allocate(".nmi_stk")) inline u8 nmi_stk[HOST_STACK_SIZE];
	__declspec(allocate(".pf_stk")) inline u8 pf_stk[HOST_STACK_SIZE];
	__declspec(allocate(".de_stk")) inline u8 de_stk[HOST_STACK_SIZE];
	__declspec(allocate(".gp_stk")) inline u8 gp_stk[HOST_STACK_SIZE];
	__declspec(allocate(".idt")) inline hv::idt_entry_t table[256];
	enum ist_idx : u8 { nmi = 4, de = 5, pf = 6, gp = 7 };

	inline void* image_base = nullptr; // used for SEH...
	auto create_entry(hv::idt_addr_t idt_handler, u8 ist_index) -> hv::idt_entry_t;
}