#pragma once
#include "hv_types.hpp"

namespace exception
{
	using ecode_t = struct { bool valid; u64 value; };
	auto injection(interruption_type type, u8 vector, ecode_t error_code = {}) -> void;

	// https://howtohypervise.blogspot.com/2019/01/a-common-missight-in-most-hypervisors.html
	auto handle_debug() -> void;
}