#pragma once
#include "handlers.hpp"
#include "debug.hpp"
#include "mm.hpp"

extern "C" auto vmxexit_handler() -> void;
extern "C" auto vmresume_failure() -> void;
extern "C" auto exit_handler(hv::pguest_registers regs) -> void;