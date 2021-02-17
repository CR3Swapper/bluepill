#pragma once
#include <Windows.h>
#include <winternl.h>

#pragma comment(lib, "ntdll.lib")
#define PAGE_4KB 0x1000

constexpr auto SystemModuleInformation = 11;
typedef struct _RTL_PROCESS_MODULE_INFORMATION
{
	HANDLE Section;
	PVOID MappedBase;
	PVOID ImageBase;
	ULONG ImageSize;
	ULONG Flags;
	USHORT LoadOrderIndex;
	USHORT InitOrderIndex;
	USHORT LoadCount;
	USHORT OffsetToFileName;
	UCHAR FullPathName[256];
} RTL_PROCESS_MODULE_INFORMATION, * PRTL_PROCESS_MODULE_INFORMATION;

typedef struct _RTL_PROCESS_MODULES
{
	ULONG NumberOfModules;
	RTL_PROCESS_MODULE_INFORMATION Modules[1];
} RTL_PROCESS_MODULES, * PRTL_PROCESS_MODULES;

typedef LARGE_INTEGER PHYSICAL_ADDRESS, * PPHYSICAL_ADDRESS;

using PEPROCESS = PVOID;
using PsLookupProcessByProcessId = NTSTATUS(__fastcall*)(
	HANDLE     ProcessId,
	PEPROCESS* Process
);

typedef union
{
    uint64_t flags;
    struct
    {
        uint64_t reserved1 : 3;
        uint64_t page_level_write_through : 1;
        uint64_t page_level_cache_disable : 1;
        uint64_t reserved2 : 7;
        uint64_t pml4_pfn : 36;
        uint64_t reserved3 : 16;
    };
} cr3;