#pragma once
#include "hv_types.hpp"

#define PML4_SELF_REF 255
#pragma section(".pml4", read, write)

namespace mm
{
    typedef union _virt_addr_t
    {
        void* value;
        struct
        {
            u64 offset_4kb : 12;
            u64 pt_index : 9;
            u64 pd_index : 9;
            u64 pdpt_index : 9;
            u64 pml4_index : 9;
            u64 reserved : 16;
        };

        struct
        {
            u64 offset_2mb : 21;
            u64 pd_index : 9;
            u64 pdpt_index : 9;
            u64 pml4_index : 9;
            u64 reserved : 16;
        };

        struct
        {
            u64 offset_1gb : 30;
            u64 pdpt_index : 9;
            u64 pml4_index : 9;
            u64 reserved : 16;
        };

    } virt_addr_t, * pvirt_addr_t;

    typedef union _pml4e
    {
        u64 value;
        struct
        {
            u64 present : 1;          // Must be 1, region invalid if 0.
            u64 rw : 1;        // If 0, writes not allowed.
            u64 user_supervisor : 1;   // If 0, user-mode accesses not allowed.
            u64 PageWriteThrough : 1; // Determines the memory type used to access PDPT.
            u64 page_cache : 1; // Determines the memory type used to access PDPT.
            u64 accessed : 1;         // If 0, this entry has not been used for translation.
            u64 Ignored1 : 1;
            u64 page_size : 1;         // Must be 0 for PML4E.
            u64 Ignored2 : 4;
            u64 pfn : 36; // The page frame number of the PDPT of this PML4E.
            u64 Reserved : 4;
            u64 Ignored3 : 11;
            u64 nx : 1; // If 1, instruction fetches not allowed.
        };
    } pml4e, * ppml4e;

    typedef union _pdpte
    {
        u64 value;
        struct
        {
            u64 present : 1;          // Must be 1, region invalid if 0.
            u64 rw : 1;        // If 0, writes not allowed.
            u64 user_supervisor : 1;   // If 0, user-mode accesses not allowed.
            u64 PageWriteThrough : 1; // Determines the memory type used to access PD.
            u64 page_cache : 1; // Determines the memory type used to access PD.
            u64 accessed : 1;         // If 0, this entry has not been used for translation.
            u64 Ignored1 : 1;
            u64 page_size : 1;         // If 1, this entry maps a 1GB page.
            u64 Ignored2 : 4;
            u64 pfn : 36; // The page frame number of the PD of this PDPTE.
            u64 Reserved : 4;
            u64 Ignored3 : 11;
            u64 nx : 1; // If 1, instruction fetches not allowed.
        };
    } pdpte, * ppdpte;

    typedef union _pde
    {
        u64 value;
        struct
        {
            u64 present : 1;          // Must be 1, region invalid if 0.
            u64 rw : 1;        // If 0, writes not allowed.
            u64 user_supervisor : 1;   // If 0, user-mode accesses not allowed.
            u64 PageWriteThrough : 1; // Determines the memory type used to access PT.
            u64 page_cache : 1; // Determines the memory type used to access PT.
            u64 accessed : 1;         // If 0, this entry has not been used for translation.
            u64 Ignored1 : 1;
            u64 page_size : 1; // If 1, this entry maps a 2MB page.
            u64 Ignored2 : 4;
            u64 pfn : 36; // The page frame number of the PT of this PDE.
            u64 Reserved : 4;
            u64 Ignored3 : 11;
            u64 nx : 1; // If 1, instruction fetches not allowed.
        };
    } pde, * ppde;

    typedef union _pte
    {
        u64 value;
        struct
        {
            u64 present : 1;          // Must be 1, region invalid if 0.
            u64 rw : 1;        // If 0, writes not allowed.
            u64 user_supervisor : 1;   // If 0, user-mode accesses not allowed.
            u64 PageWriteThrough : 1; // Determines the memory type used to access the memory.
            u64 page_cache : 1; // Determines the memory type used to access the memory.
            u64 accessed : 1;         // If 0, this entry has not been used for translation.
            u64 Dirty : 1;            // If 0, the memory backing this page has not been written to.
            u64 PageAccessType : 1;   // Determines the memory type used to access the memory.
            u64 Global : 1;           // If 1 and the PGE bit of CR4 is set, translations are global.
            u64 Ignored2 : 3;
            u64 pfn : 36; // The page frame number of the backing physical page.
            u64 reserved : 4;
            u64 Ignored3 : 7;
            u64 ProtectionKey : 4;  // If the PKE bit of CR4 is set, determines the protection key.
            u64 nx : 1; // If 1, instruction fetches not allowed.
        };
    } pte, * ppte;

    enum class map_type{ dest, src };
    inline const ppml4e vmxroot_pml4 = reinterpret_cast<ppml4e>(0x7fbfdfeff000);

    // make sure this is 4kb aligned or you are going to be meeting allah...
	__declspec(allocate(".pml4")) 
    inline pml4e pml4[512];

    // translate vmxroot address's...
    auto translate(virt_addr_t virt_addr) -> u64;
    
    // translate guest virtual addresses...
    auto translate(virt_addr_t virt_addr, u64 pml4_phys, map_type type = map_type::src) -> u64;

    // map a page into vmxroot address space...
    auto map_page(u64 phys_addr, map_type type) -> u64;

    // map a page (4kb) from another address into vmxroot...
    auto map_virt(u64 dirbase, u64 virt_addr, map_type map_type)->u64;

    // copy virtual memory without changing cr3... this maps the physical memory into vmxroot 
    // address space and copies the memory directly between the physical pages... the memory must be paged in...
    auto copy_virt(u64 dirbase_src, u64 virt_src, u64 dirbase_dest, u64 virt_dest, u64 size) -> bool;
}