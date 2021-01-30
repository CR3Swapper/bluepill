#pragma once
#include "segment_intrin.h"
#include "vmxexit_handler.h"
#include "vmxon.hpp"
#include "vmcs.hpp"

namespace vmxlaunch
{
	auto init_vmcs() -> void;
	auto launch() -> void;
}