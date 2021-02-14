#pragma once
#include <stdarg.h>
#include <stddef.h>
#include "hv_types.hpp"
#define PORT_NUM 0x3E8

namespace dbg
{
	constexpr char alphabet[] = "0123456789ABCDEF";
	auto debug_print_decimal(long long number) -> void;
	auto debug_print_hex(u64 number, const bool show_zeros) -> void;
	auto print(const char* format, ...) -> void;
}