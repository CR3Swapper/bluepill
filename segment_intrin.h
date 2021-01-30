#pragma once
#include "hv_types.hpp"

extern "C" u16 readfs(void);
extern "C" u16 readgs(void);
extern "C" u16 reades(void);
extern "C" u16 readds(void);
extern "C" u16 readss(void);
extern "C" u16 readcs(void);
extern "C" u16 readtr(void);
extern "C" u16 readldt(void);

namespace segment
{
	auto get_info(segment_descriptor_register_64 gdt_value, segment_selector segment_selector) -> hv::segment_info_ctx;
	auto get_access_rights(segment_descriptor_64* segment_descriptor) -> vmx_segment_access_rights;
}