#include "idt.hpp"

void seh_handler(hv::pidt_regs_t regs)
{
	// probably not going to work since software interrupts are disabled?
	__debugbreak();
	return;
}

namespace idt
{
	auto create_entry(hv::idt_addr_t idt_handler, u8 ist_index) -> hv::idt_entry_t
	{
		hv::idt_entry_t result{};
		result.segment_selector = readcs();
		result.gate_type = SEGMENT_DESCRIPTOR_TYPE_INTERRUPT_GATE;
		result.present = true;
		result.ist_index = ist_index;
		result.dpl = 0;

		result.offset_high = idt_handler.offset_high;
		result.offset_middle = idt_handler.offset_middle;
		result.offset_low = idt_handler.offset_low;
		return result;
	}
}