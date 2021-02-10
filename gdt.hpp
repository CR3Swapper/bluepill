#pragma once
#include "hv_types.hpp"
#pragma section(".gdt", read, write)

namespace gdt
{
	// index used for GDT per-core in vmxroot...
	enum idx : u8 { es, ds, cs, gs, fs, ss, tr, ldt };

	auto get_info(const segment_descriptor_register_64& gdt_value, segment_selector segment) -> hv::segment_info_ctx;
	auto get_access_rights(segment_descriptor_64* segment_descriptor) -> vmx_segment_access_rights;
}