#pragma once
#include "segment_intrin.h"
#include "vmxexit_handler.h"
#include "vmxon.hpp"
#include "vmcs.hpp"

#define VMX_LAUNCH_SUCCESS 0xC0FFEE
extern "C" u32 vmxlaunch_processor(void);

namespace vmxlaunch
{
	auto init_vmcs() -> void;
	auto launch() -> void;
}