#pragma once
#include "hv_types.hpp"
#include "debug.hpp"
#include "mm.hpp"
#include "vmxon.hpp"
#include "command.hpp"
#include "exception.hpp"

extern "C" auto vmxexit_handler() -> void;
extern "C" auto vmresume_failure() -> void;
extern "C" auto exit_handler(hv::pguest_registers regs) -> void;