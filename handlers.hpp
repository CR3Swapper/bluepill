#pragma once
#include "vmxon.hpp"
#include "hv_types.hpp"
#include "command.hpp"
#include "exception.hpp"

namespace handle
{
	auto xsetbv(hv::pguest_registers regs) -> bool;
	auto rdmsr(hv::pguest_registers regs) -> bool;
	auto wrmsr(hv::pguest_registers regs) -> bool;
	auto vmcall(hv::pguest_registers regs) -> bool;
}