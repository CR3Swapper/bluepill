#pragma once
#include "hv_types.hpp"
#include "invd.hpp"

extern "C" auto vmxexit_handler() -> void;
extern "C" auto exit_handler(hv::pguest_registers regs) -> void;