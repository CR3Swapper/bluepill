#include "segment_intrin.h"

namespace segment
{
	auto get_access_rights(segment_descriptor_64* segment_descriptor) -> vmx_segment_access_rights
	{
		vmx_segment_access_rights result;
		result.flags = NULL;

		result.granularity = segment_descriptor->granularity;
		result.type = segment_descriptor->type;
		result.descriptor_type = segment_descriptor->descriptor_type;
		result.present = segment_descriptor->present;
		result.long_mode = segment_descriptor->long_mode;
		result.available_bit = segment_descriptor->system;
		result.default_big = segment_descriptor->default_big;
		result.descriptor_privilege_level = segment_descriptor->descriptor_privilege_level;
		result.unusable = !segment_descriptor->present;
		return result;
	}

	auto get_info(const segment_descriptor_register_64& gdt_value, segment_selector selector) -> hv::segment_info_ctx
	{
		hv::segment_info_ctx segment_info{};

		const auto segment_descriptor = 
			reinterpret_cast<segment_descriptor_64*>(
				gdt_value.base_address + (selector.index << SEGMENT_SELECTOR_INDEX_BIT));

		// access rights are spread out over the segment 
		// descriptor so those need to picked out and assigned 
		// to the vmx segment access rights variable...
		segment_info.limit = __segmentlimit(selector.flags);
		segment_info.rights = get_access_rights(segment_descriptor);

		// base address of a segment is spread over the segment descriptor in 3 places. 2 parts of the 
		// address are 8 bits each (1 byte each) and the lowest part of the address is 2 bytes (4 bytes in total)...
		// by shifting the values to the correct bit offset and adding them all together we get the address...
		segment_info.base_addr = (u32)((segment_descriptor->base_address_high << SEGMENT__BASE_ADDRESS_HIGH_BIT) +
			(segment_descriptor->base_address_middle << SEGMENT__BASE_ADDRESS_MIDDLE_BIT) +
				(segment_descriptor->base_address_low));

		// Example:
		//	- high bits:	0b1111 0000 0000 0000
		//	- middle bits:	0b0000 1111 0000 0000
		//	- low bits:		0b0000 0000 1011 1000
		//					--------------------- +
		//					0b1111 1111 1011 1000 <==== full address...
		// if you add all of these together you will get the full address...

		// if the base address is 64bits then go ahead 
		// and add the top 32bits onto the address..
		if (!segment_descriptor->descriptor_type)
			segment_info.base_addr += ((u64)segment_descriptor->base_address_upper << SEGMENT__BASE_ADDRESS_SHIFT);

		return segment_info;
	}
}