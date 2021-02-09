#include "idt.hpp"

void seh_handler(hv::pidt_regs_t regs)
{
	return;
}

namespace idt
{
	auto create_entry(hv::idt_addr_t idt_handler) -> hv::idt_entry_t
	{
		hv::idt_entry_t result{};
		result.segment_selector = readcs();
		result.gate_type = SEGMENT_DESCRIPTOR_TYPE_INTERRUPT_GATE;
		result.present = true;

		result.offset_high = idt_handler.offset_high;
		result.offset_middle = idt_handler.offset_middle;
		result.offset_low = idt_handler.offset_low;
		return result;
	}

	auto init() -> u64
	{
		idt::table[general_protection] = create_entry(hv::idt_addr_t{ __gp_handler });
		idt::table[page_fault] = create_entry(hv::idt_addr_t{ __pf_handler });
		idt::table[divide_error] = create_entry(hv::idt_addr_t{ __de_handler });
		return reinterpret_cast<u64>(idt::table);
	}
}