#include "mm.hpp"

namespace mm
{
    auto translate(virt_addr_t virt_addr) -> u64
    {
        return {};
    }

    auto translate(virt_addr_t virt_addr, u64 pml4_phys, map_type type) -> u64
    {
        return {};
    }

    auto map_page(u64 phys_addr, map_type type) -> u64
    {
        cpuid_eax_01 cpuid_value;
        virt_addr_t result{ vmxroot_pml4 };
        __cpuid((int*)&cpuid_value, 1);

        result.pt_index = (cpuid_value
            .cpuid_additional_information
            .initial_apic_id * 2)
            + (unsigned)type;

        reinterpret_cast<ppte>(vmxroot_pml4)
            [result.pt_index].pfn = phys_addr >> 12;

        __invlpg(result.value);
        result.offset = virt_addr_t{ (void*)phys_addr }.offset;
        return reinterpret_cast<u64>(result.value);
    }
}