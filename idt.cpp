#include "idt.hpp"

auto seh_handler(hv::pidt_regs_t regs) -> void
{
	__debugbreak();

    /*const auto rva = regs->rip - reinterpret_cast<u64>(idt::image_base);

    IMAGE_NT_HEADERS64* nt = (IMAGE_NT_HEADERS64*)(reinterpret_cast<u64>(idt::image_base) + 
        reinterpret_cast<IMAGE_DOS_HEADER>(idt::image_base).e_lfanew);

    IMAGE_DATA_DIRECTORY* exception =
        &nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION];

    RUNTIME_FUNCTION* functions =
        (RUNTIME_FUNCTION*)((UINT64)&__ImageBase + exception->VirtualAddress);

    for (UINT32 i = 0; i < exception->Size / sizeof(RUNTIME_FUNCTION); ++i) {
        RUNTIME_FUNCTION* function = &functions[i];
        if (!(rva >= function->BeginAddress && rva < function->EndAddress)) {
            continue;
        }

        UNWIND_INFO* unwindInfo = (UNWIND_INFO*)((UINT64)&__ImageBase + function->UnwindData);
        if (!(unwindInfo->Flags & UNW_FLAG_EHANDLER)) {
            continue;
        }

        SCOPE_TABLE* scopeTable =
            (SCOPE_TABLE*)((UINT64)&unwindInfo->UnwindCode[(unwindInfo->CountOfCodes + 1) & ~1] +
                sizeof(UINT32));

        for (UINT32 e = 0; e < scopeTable->Count; ++e) {
            SCOPE_RECORD* scopeRecord = &scopeTable->ScopeRecords[e];

            if (rva >= scopeRecord->BeginAddress && rva < scopeRecord->EndAddress) {
                *rip = (UINT64)&__ImageBase + scopeRecord->JumpTarget;
                return;
            }
        }
    }
    */
}

namespace idt
{
	auto create_entry(hv::idt_addr_t idt_handler, u8 ist_index) -> hv::idt_entry_t
	{
		hv::idt_entry_t result{};
		result.segment_selector = readcs();
		result.gate_type = SEGMENT_DESCRIPTOR_TYPE_INTERRUPT_GATE;
		result.present = true;
		result.ist_index = NULL;
		result.dpl = 0;

		result.offset_high = idt_handler.offset_high;
		result.offset_middle = idt_handler.offset_middle;
		result.offset_low = idt_handler.offset_low;
		return result;
	}
}