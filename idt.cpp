#include "idt.hpp"

auto seh_handler(hv::pidt_regs_t regs) -> void
{
    const auto rva = regs->rip - reinterpret_cast<u64>(idt::image_base);
    const auto nt_headers = reinterpret_cast<IMAGE_NT_HEADERS64*>(
        reinterpret_cast<u64>(idt::image_base) + 
            reinterpret_cast<IMAGE_DOS_HEADER*>(idt::image_base)->e_lfanew);

    const auto exception =
        &nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION];

    const auto functions =
        reinterpret_cast<RUNTIME_FUNCTION*>(
            reinterpret_cast<u64>(idt::image_base) + exception->VirtualAddress);

    for (auto idx = 0; idx < exception->Size / sizeof(RUNTIME_FUNCTION); ++idx) 
    {
        const auto function = &functions[idx];
        if (!(rva >= function->BeginAddress && rva < function->EndAddress)) 
            continue;

        const auto unwind_info = 
            reinterpret_cast<UNWIND_INFO*>(
                reinterpret_cast<u64>(idt::image_base) + function->UnwindData);

        if (!(unwind_info->Flags & UNW_FLAG_EHANDLER)) 
            continue;

        const auto scope_table =
            reinterpret_cast<SCOPE_TABLE*>(
                reinterpret_cast<u64>(&unwind_info->UnwindCode[
                    (unwind_info->CountOfCodes + 1) & ~1]) + sizeof(u32));

        for (auto entry = 0; entry < scope_table->Count; ++entry) 
        {
            const auto scope_record = &scope_table->ScopeRecords[entry];
            if (rva >= scope_record->BeginAddress && rva < scope_record->EndAddress) 
            {
                regs->rip = reinterpret_cast<u64>(idt::image_base) + scope_record->JumpTarget;
                return;
            }
        }
    }
}

namespace idt
{
	auto create_entry(hv::idt_addr_t idt_handler, u8 ist_index) -> hv::idt_entry_t
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
}