#include "idt.hpp"

namespace idt
{
	auto create_entry(void* address) -> hv::idt_entry_t
	{
		hv::idt_addr_t idt_addr{ (u64) address };
		hv::idt_entry_t result{};

		result.dpl = 0;
		result.storage_segment = 0;
		result.segment_selector = readcs();
		result.gate_type = SEGMENT_DESCRIPTOR_TYPE_INTERRUPT_GATE;
		result.present = 1;
		result.offset_high = idt_addr.offset_high;
		result.offset_middle = idt_addr.offset_middle;
		result.offset_low = idt_addr.offset_low;
		return result;
	}
}