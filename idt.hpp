#pragma once
#include "hv_types.hpp"
#include "segment_intrin.h"
#pragma section(".idt", read, write)

namespace idt
{
	__declspec(allocate(".idt")) inline hv::idt_entry_t table[256];
	auto create_entry(void* address) -> hv::idt_entry_t;
}