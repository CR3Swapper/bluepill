#pragma once
#include "hv_types.hpp"
#include "segment_intrin.h"
#include "vmxexit_handler.h"

namespace vmcs
{
	auto setup_host(void* host_rip, u64 host_rsp) -> void;
	auto setup_guest() -> void;
	auto setup_controls() -> void;
}