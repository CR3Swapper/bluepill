#pragma once
#include "hv_types.hpp"
#pragma section(".gdt", read, write)

#define GDT_CS_INDEX 0
#define GDT_DS_INDEX 1
#define GDT_TSS_INDEX 2

namespace gdt
{
	__declspec(allocate(".gdt")) 
	inline segment_descriptor_64 table[3]; // TODO...

	auto init()->void;
	auto get_info(const segment_descriptor_register_64& gdt_value, segment_selector segment_selector)->hv::segment_info_ctx;
	auto get_access_rights(segment_descriptor_64* segment_descriptor)->vmx_segment_access_rights;
}