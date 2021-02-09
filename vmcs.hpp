#pragma once
#include "hv_types.hpp"
#include "segment_intrin.h"
#include "vmxexit_handler.h"
#include "gdt.hpp"
#include "idt.hpp"

namespace vmcs
{
	auto setup_host(void* host_rip, u64 host_rsp, cr3 cr3_value) -> void;
	auto setup_guest() -> void;
	auto setup_controls() -> void;
}