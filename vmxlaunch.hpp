#pragma once
#include "segment_intrin.h"
#include "vmxexit_handler.h"
#include "vmxon.hpp"
#include "vmcs.hpp"
#include "mm.hpp"

#define VMX_LAUNCH_SUCCESS 0xC0FFEE
extern "C" u32 vmxlaunch_processor(void);

namespace vmxlaunch
{
	auto init_vmcs(cr3 cr3_value) -> void;
	auto launch() -> void;
}