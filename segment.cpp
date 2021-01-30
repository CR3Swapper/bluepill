#include "segment_intrin.h"

namespace segment
{
	auto get_access_rights(segment_descriptor_64* segment_descriptor) -> vmx_segment_access_rights
	{
		vmx_segment_access_rights result;
		result.granularity = segment_descriptor->granularity;
		result.type = segment_descriptor->type;
		result.descriptor_type = segment_descriptor->descriptor_type;
		result.present = segment_descriptor->present;
		result.long_mode = segment_descriptor->long_mode;
		result.available_bit = segment_descriptor->system;
		result.default_big = segment_descriptor->default_big;
		return result;
	}

	auto get_info(segment_descriptor_register_64 gdt_value, segment_selector segment_selector) -> hv::segment_info_ctx
	{
		hv::segment_info_ctx segment_info;
		const auto segment_descriptor = 
			reinterpret_cast<segment_descriptor_64*>(
				gdt_value.base_address + (segment_selector.index << SEGMENT_SELECTOR_INDEX_BIT));

		segment_info.limit = __segmentlimit(segment_selector.flags);
		segment_info.rights = get_access_rights(segment_descriptor);

		segment_info.base_addr = (u32)((segment_descriptor->base_address_high << SEGMENT__BASE_ADDRESS_HIGH_BIT) |
			(segment_descriptor->base_address_middle << SEGMENT__BASE_ADDRESS_MIDDLE_BIT) |
				(segment_descriptor->base_address_low));

		segment_info.base_addr &= 0xFFFFFFFF;
		if (!segment_descriptor->descriptor_type)
			segment_info.base_addr |= ((u64)segment_descriptor->base_address_upper << SEGMENT__BASE_ADDRESS_SHIFT);

		return segment_info;
	}
}