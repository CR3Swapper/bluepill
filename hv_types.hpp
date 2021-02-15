#pragma once
#include <ntifs.h>
#include <intrin.h>
#include "ia32.hpp"
#include <minwindef.h>

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;
using u64 = unsigned long long;
using u128 = __m128;

using s8 = char;
using s16 = short;
using s32 = int;
using s64 = long long;

// didnt find it in intrin.h... ?
extern "C" void _sgdt(void*);
#pragma intrinsic(_sgdt);

#ifdef DBG_PRINT_BOOL
#define DBG_PRINT(format, ...) \
	DbgPrintEx( DPFLTR_SYSTEM_ID, DPFLTR_ERROR_LEVEL, \
	"[hv [core number = %d]]" format, KeGetCurrentProcessorNumber(), __VA_ARGS__)
#else
#define DBG_PRINT(format, ...)
#endif

#define HOST_STACK_PAGES 6
#define HOST_STACK_SIZE PAGE_SIZE * HOST_STACK_PAGES

// Export Directory
#define IMAGE_DIRECTORY_ENTRY_EXPORT         0
// Import Directory
#define IMAGE_DIRECTORY_ENTRY_IMPORT         1
// Resource Directory
#define IMAGE_DIRECTORY_ENTRY_RESOURCE       2
// Exception Directory
#define IMAGE_DIRECTORY_ENTRY_EXCEPTION      3
// Security Directory
#define IMAGE_DIRECTORY_ENTRY_SECURITY       4
// Base Relocation Table
#define IMAGE_DIRECTORY_ENTRY_BASERELOC      5
// Debug Directory
#define IMAGE_DIRECTORY_ENTRY_DEBUG          6
// Description String
#define IMAGE_DIRECTORY_ENTRY_COPYRIGHT      7
// Machine Value (MIPS GP)
#define IMAGE_DIRECTORY_ENTRY_GLOBALPTR      8
// TLS Directory
#define IMAGE_DIRECTORY_ENTRY_TLS            9
// Load Configuration Directory
#define IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG    10

#define UNW_FLAG_NHANDLER  0
#define UNW_FLAG_EHANDLER  1
#define UNW_FLAG_UHANDLER  2
#define UNW_FLAG_CHAININFO 4

typedef struct _IMAGE_DOS_HEADER {  // DOS .EXE header
	USHORT e_magic;         // Magic number
	USHORT e_cblp;          // Bytes on last page of file
	USHORT e_cp;            // Pages in file
	USHORT e_crlc;          // Relocations
	USHORT e_cparhdr;       // Size of header in paragraphs
	USHORT e_minalloc;      // Minimum extra paragraphs needed
	USHORT e_maxalloc;      // Maximum extra paragraphs needed
	USHORT e_ss;            // Initial (relative) SS value
	USHORT e_sp;            // Initial SP value
	USHORT e_csum;          // Checksum
	USHORT e_ip;            // Initial IP value
	USHORT e_cs;            // Initial (relative) CS value
	USHORT e_lfarlc;        // File address of relocation table
	USHORT e_ovno;          // Overlay number
	USHORT e_res[4];        // Reserved words
	USHORT e_oemid;         // OEM identifier (for e_oeminfo)
	USHORT e_oeminfo;       // OEM information; e_oemid specific
	USHORT e_res2[10];      // Reserved words
	LONG   e_lfanew;        // File address of new exe header
} IMAGE_DOS_HEADER, * PIMAGE_DOS_HEADER;

typedef struct _IMAGE_FILE_HEADER {
	short  Machine;
	short  NumberOfSections;
	unsigned TimeDateStamp;
	unsigned PointerToSymbolTable;
	unsigned NumberOfSymbols;
	short  SizeOfOptionalHeader;
	short  Characteristics;
} IMAGE_FILE_HEADER, * PIMAGE_FILE_HEADER;

typedef struct _IMAGE_DATA_DIRECTORY {
	unsigned VirtualAddress;
	unsigned Size;
} IMAGE_DATA_DIRECTORY, * PIMAGE_DATA_DIRECTORY;

typedef struct _IMAGE_OPTIONAL_HEADER64 {
	short                 Magic;
	unsigned char                 MajorLinkerVersion;
	unsigned char                 MinorLinkerVersion;
	unsigned                SizeOfCode;
	unsigned                SizeOfInitializedData;
	unsigned                SizeOfUninitializedData;
	unsigned                AddressOfEntryPoint;
	unsigned                BaseOfCode;
	ULONGLONG            ImageBase;
	unsigned                SectionAlignment;
	unsigned                FileAlignment;
	short                 MajorOperatingSystemVersion;
	short                 MinorOperatingSystemVersion;
	short                 MajorImageVersion;
	short                 MinorImageVersion;
	short                 MajorSubsystemVersion;
	short                 MinorSubsystemVersion;
	unsigned                Win32VersionValue;
	unsigned                SizeOfImage;
	unsigned                SizeOfHeaders;
	unsigned                CheckSum;
	short                 Subsystem;
	short                 DllCharacteristics;
	ULONGLONG            SizeOfStackReserve;
	ULONGLONG            SizeOfStackCommit;
	ULONGLONG            SizeOfHeapReserve;
	ULONGLONG            SizeOfHeapCommit;
	unsigned                 LoaderFlags;
	unsigned                NumberOfRvaAndSizes;
	IMAGE_DATA_DIRECTORY DataDirectory[16];
} IMAGE_OPTIONAL_HEADER64, * PIMAGE_OPTIONAL_HEADER64;

typedef struct _IMAGE_NT_HEADERS64 {
	unsigned                   Signature;
	IMAGE_FILE_HEADER       FileHeader;
	IMAGE_OPTIONAL_HEADER64 OptionalHeader;
} IMAGE_NT_HEADERS64, * PIMAGE_NT_HEADERS64;

typedef struct _IMAGE_IMPORT_DESCRIPTOR {
	union {
		unsigned long   Characteristics;            // 0 for terminating null import descriptor
		unsigned long   OriginalFirstThunk;         // RVA to original unbound IAT (PIMAGE_THUNK_DATA)
	} DUMMYUNIONNAME;
	unsigned long   TimeDateStamp;                  // 0 if not bound,
											// -1 if bound, and real date\time stamp
											//     in IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT (new BIND)
											// O.W. date/time stamp of DLL bound to (Old BIND)

	unsigned long   ForwarderChain;                 // -1 if no forwarders
	unsigned long   Name;
	unsigned long   FirstThunk;                     // RVA to IAT (if bound this IAT has actual addresses)
} IMAGE_IMPORT_DESCRIPTOR;
typedef IMAGE_IMPORT_DESCRIPTOR UNALIGNED* PIMAGE_IMPORT_DESCRIPTOR;

typedef struct _IMAGE_IMPORT_BY_NAME {
	unsigned long    Hint;
	CHAR   Name[1];
} IMAGE_IMPORT_BY_NAME, * PIMAGE_IMPORT_BY_NAME;

typedef struct _IMAGE_THUNK_DATA64 {
	union {
		ULONGLONG ForwarderString;  // PBYTE 
		ULONGLONG Function;         // PDWORD
		ULONGLONG Ordinal;
		ULONGLONG AddressOfData;    // PIMAGE_IMPORT_BY_NAME
	} u1;
} IMAGE_THUNK_DATA64, * PIMAGE_THUNK_DATA64;
typedef PIMAGE_THUNK_DATA64             PIMAGE_THUNK_DATA;

typedef struct _SCOPE_RECORD {
	UINT32 BeginAddress;
	UINT32 EndAddress;
	UINT32 HandlerAddress;
	UINT32 JumpTarget;
} SCOPE_RECORD;

typedef struct _SCOPE_TABLE {
	UINT32 Count;
	SCOPE_RECORD ScopeRecords[1];
} SCOPE_TABLE;

typedef struct _RUNTIME_FUNCTION {
	UINT32 BeginAddress;
	UINT32 EndAddress;
	UINT32 UnwindData;
} RUNTIME_FUNCTION;

#pragma warning(push)
#pragma warning(disable : 4200)
#pragma warning(disable : 4201)
#pragma warning(disable : 4214)
typedef union _UNWIND_CODE {
	UINT8 CodeOffset;
	UINT8 UnwindOp : 4;
	UINT8 OpInfo : 4;
	UINT16 FrameOffset;
} UNWIND_CODE;

typedef struct _UNWIND_INFO {
	UINT8 Version : 3;
	UINT8 Flags : 5;
	UINT8 SizeOfProlog;
	UINT8 CountOfCodes;
	UINT8 FrameRegister : 4;
	UINT8 FrameOffset : 4;
	UNWIND_CODE UnwindCode[1];

	union {
		UINT32 ExceptionHandler;
		UINT32 FunctionEntry;
	};

	UINT32 ExceptionData[];
} UNWIND_INFO;
#pragma warning(pop)

namespace hv
{
	typedef struct _guest_registers
	{
		u128 xmm0;
		u128 xmm1;
		u128 xmm2;
		u128 xmm3;
		u128 xmm4;
		u128 xmm5;
		u128 xmm6;
		u128 xmm7;
		u128 xmm8;
		u128 xmm9;
		u128 xmm10;
		u128 xmm11;
		u128 xmm12;
		u128 xmm13;
		u128 xmm14;
		u128 xmm15;

		u64 padding_8b;

		u64 r15;
		u64 r14;
		u64 r13;
		u64 r12;
		u64 r11;
		u64 r10;
		u64 r9;
		u64 r8;
		u64 rbp;
		u64 rdi;
		u64 rsi;
		u64 rdx;
		u64 rcx;
		u64 rbx;
		u64 rax;
	} guest_registers, * pguest_registers;

	typedef struct _idt_regs_t
	{
		u128 xmm0;
		u128 xmm1;
		u128 xmm2;
		u128 xmm3;
		u128 xmm4;
		u128 xmm5;
		u128 xmm6;
		u128 xmm7;
		u128 xmm8;
		u128 xmm9;
		u128 xmm10;
		u128 xmm11;
		u128 xmm12;
		u128 xmm13;
		u128 xmm14;
		u128 xmm15;

		u64 padding_8b;

		u64 r15;
		u64 r14;
		u64 r13;
		u64 r12;
		u64 r11;
		u64 r10;
		u64 r9;
		u64 r8;
		u64 rbp;
		u64 rdi;
		u64 rsi;
		u64 rdx;
		u64 rcx;
		u64 rbx;
		u64 rax;

		u64 error_code;
		u64 rip;
		u64 cs_selector;
		::rflags rflags;
		u64 rsp;
		u64 ss_selector;
	} idt_regs_t, *pidt_regs_t;

	union msr_split
	{
		u64 value;
		struct
		{
			u64 low : 32;
			u64 high : 32;
		};
	};

	typedef union _idt_entry_t
	{
		u128 flags;
		struct
		{
			u64 offset_low : 16;
			u64 segment_selector : 16;
			u64 ist_index : 3;
			u64 reserved_0 : 5;
			u64 gate_type : 5;
			u64 dpl : 2;
			u64 present : 1;
			u64 offset_middle : 16;
			u64 offset_high : 32;
			u64 reserved_1 : 32;
		};
	} idt_entry_t, *pidt_entry_t;

	union idt_addr_t
	{
		void* addr;
		struct
		{
			u64 offset_low : 16;
			u64 offset_middle : 16;
			u64 offset_high : 32;
		};
	};

	typedef struct _tss64
	{
		u32 reserved;
		u64 privilege_stacks[3];
		u64 reserved_1;
		u64 interrupt_stack_table[7];
		u16 reserved_2;
		u16 iomap_base;
	} tss64, *ptss64;

	union segment_descriptor_addr_t
	{
		void* addr;
		struct
		{
			u64 low : 16;
			u64 middle : 8;
			u64 high : 8;
			u64 upper : 32;
		};
	};

	union ia32_efer_t
	{
		unsigned __int64 control;
		struct
		{
			unsigned __int64 syscall_enable : 1;
			unsigned __int64 reserved_0 : 7;
			unsigned __int64 long_mode_enable : 1;
			unsigned __int64 reserved_1 : 1;
			unsigned __int64 long_mode_active : 1;
			unsigned __int64 execute_disable : 1;
			unsigned __int64 reserved_2 : 52;
		} bits;
	};

	union ia32_feature_control_msr_t
	{
		unsigned __int64 control;
		struct
		{
			unsigned __int64 lock : 1;
			unsigned __int64 vmxon_inside_smx : 1;
			unsigned __int64 vmxon_outside_smx : 1;
			unsigned __int64 reserved_0 : 5;
			unsigned __int64 senter_local : 6;
			unsigned __int64 senter_global : 1;
			unsigned __int64 reserved_1 : 1;
			unsigned __int64 sgx_launch_control_enable : 1;
			unsigned __int64 sgx_global_enable : 1;
			unsigned __int64 reserved_2 : 1;
			unsigned __int64 lmce : 1;
			unsigned __int64 system_reserved : 42;
		} bits;
	};

	union vmx_misc_msr_t
	{
		unsigned __int64 control;
		struct
		{
			unsigned __int64 vmx_preemption_tsc_rate : 5;
			unsigned __int64 store_lma_in_vmentry_control : 1;
			unsigned __int64 activate_state_bitmap : 3;
			unsigned __int64 reserved_0 : 5;
			unsigned __int64 pt_in_vmx : 1;
			unsigned __int64 rdmsr_in_smm : 1;
			unsigned __int64 cr3_target_value_count : 9;
			unsigned __int64 max_msr_vmexit : 3;
			unsigned __int64 allow_smi_blocking : 1;
			unsigned __int64 vmwrite_to_any : 1;
			unsigned __int64 interrupt_mod : 1;
			unsigned __int64 reserved_1 : 1;
			unsigned __int64 mseg_revision_identifier : 32;
		} bits;
	};

	union vmx_pinbased_control_msr_t
	{
		unsigned __int64 control;
		struct
		{
			unsigned __int64 external_interrupt_exiting : 1;
			unsigned __int64 reserved_0 : 2;
			unsigned __int64 nmi_exiting : 1;
			unsigned __int64 reserved_1 : 1;
			unsigned __int64 virtual_nmis : 1;
			unsigned __int64 vmx_preemption_timer : 1;
			unsigned __int64 process_posted_interrupts : 1;
		} bits;
	};

	union vmx_primary_processor_based_control_t
	{
		unsigned __int64 control;
		struct
		{
			unsigned __int64 reserved_0 : 2;
			unsigned __int64 interrupt_window_exiting : 1;
			unsigned __int64 use_tsc_offsetting : 1;
			unsigned __int64 reserved_1 : 3;
			unsigned __int64 hlt_exiting : 1;
			unsigned __int64 reserved_2 : 1;
			unsigned __int64 invldpg_exiting : 1;
			unsigned __int64 mwait_exiting : 1;
			unsigned __int64 rdpmc_exiting : 1;
			unsigned __int64 rdtsc_exiting : 1;
			unsigned __int64 reserved_3 : 2;
			unsigned __int64 cr3_load_exiting : 1;
			unsigned __int64 cr3_store_exiting : 1;
			unsigned __int64 reserved_4 : 2;
			unsigned __int64 cr8_load_exiting : 1;
			unsigned __int64 cr8_store_exiting : 1;
			unsigned __int64 use_tpr_shadow : 1;
			unsigned __int64 nmi_window_exiting : 1;
			unsigned __int64 mov_dr_exiting : 1;
			unsigned __int64 unconditional_io_exiting : 1;
			unsigned __int64 use_io_bitmaps : 1;
			unsigned __int64 reserved_5 : 1;
			unsigned __int64 monitor_trap_flag : 1;
			unsigned __int64 use_msr_bitmaps : 1;
			unsigned __int64 monitor_exiting : 1;
			unsigned __int64 pause_exiting : 1;
			unsigned __int64 active_secondary_controls : 1;
		} bits;
	};

	union vmx_secondary_processor_based_control_t
	{
		unsigned __int64 control;
		struct
		{
			unsigned __int64 virtualize_apic_accesses : 1;
			unsigned __int64 enable_ept : 1;
			unsigned __int64 descriptor_table_exiting : 1;
			unsigned __int64 enable_rdtscp : 1;
			unsigned __int64 virtualize_x2apic : 1;
			unsigned __int64 enable_vpid : 1;
			unsigned __int64 wbinvd_exiting : 1;
			unsigned __int64 unrestricted_guest : 1;
			unsigned __int64 apic_register_virtualization : 1;
			unsigned __int64 virtual_interrupt_delivery : 1;
			unsigned __int64 pause_loop_exiting : 1;
			unsigned __int64 rdrand_exiting : 1;
			unsigned __int64 enable_invpcid : 1;
			unsigned __int64 enable_vmfunc : 1;
			unsigned __int64 vmcs_shadowing : 1;
			unsigned __int64 enable_encls_exiting : 1;
			unsigned __int64 rdseed_exiting : 1;
			unsigned __int64 enable_pml : 1;
			unsigned __int64 use_virtualization_exception : 1;
			unsigned __int64 conceal_vmx_from_pt : 1;
			unsigned __int64 enable_xsave_xrstor : 1;
			unsigned __int64 reserved_0 : 1;
			unsigned __int64 mode_based_execute_control_ept : 1;
			unsigned __int64 reserved_1 : 2;
			unsigned __int64 use_tsc_scaling : 1;
		} bits;
	};

	union vmx_entry_control_t
	{
		unsigned __int64 control;
		struct
		{
			unsigned __int64 reserved_0 : 2;
			unsigned __int64 load_dbg_controls : 1;
			unsigned __int64 reserved_1 : 6;
			unsigned __int64 ia32e_mode_guest : 1;
			unsigned __int64 entry_to_smm : 1;
			unsigned __int64 deactivate_dual_monitor_treament : 1;
			unsigned __int64 reserved_3 : 1;
			unsigned __int64 load_ia32_perf_global_control : 1;
			unsigned __int64 load_ia32_pat : 1;
			unsigned __int64 load_ia32_efer : 1;
			unsigned __int64 load_ia32_bndcfgs : 1;
			unsigned __int64 conceal_vmx_from_pt : 1;
		} bits;
	};

	union cr_fixed_t
	{
		struct
		{
			unsigned long low;
			long high;
		} split;
		struct
		{
			unsigned long low;
			long high;
		} u;
		long long all;
	};

	union cr8_t
	{
		u64 control;
		struct
		{
			u64 task_priority_level : 4;
			u64 reserved : 59;
		} bits;
	};

	typedef union
	{
		struct
		{
			u64 protection_enable : 1;
			u64 monitor_coprocessor : 1;
			u64 emulate_fpu : 1;
			u64 task_switched : 1;
			u64 extension_type : 1;
			u64 numeric_error : 1;
			u64 reserved1 : 10;
			u64 write_protect : 1;
			u64 reserved2 : 1;
			u64 alignment_mask : 1;
			u64 reserved3 : 10;
			u64 not_write_through : 1;
			u64 cache_disable : 1;
			u64 paging_enable : 1;
			u64 reserved4 : 32;
		};
		u64 flags;
	} cr0_t;

	typedef union
	{
		u64 flags;
		struct
		{
			u64 virtual_mode_extensions : 1;
			u64 protected_mode_virtual_interrupts : 1;
			u64 timestamp_disable : 1;
			u64 debugging_extensions : 1;
			u64 page_size_extensions : 1;
			u64 physical_address_extension : 1;
			u64 machine_check_enable : 1;
			u64 page_global_enable : 1;
			u64 performance_monitoring_counter_enable : 1;
			u64 os_fxsave_fxrstor_support : 1;
			u64 os_xmm_exception_support : 1;
			u64 usermode_instruction_prevention : 1;
			u64 reserved1 : 1;
			u64 vmx_enable : 1;
			u64 smx_enable : 1;
			u64 reserved2 : 1;
			u64 fsgsbase_enable : 1;
			u64 pcid_enable : 1;
			u64 os_xsave : 1;
			u64 reserved3 : 1;
			u64 smep_enable : 1;
			u64 smap_enable : 1;
			u64 protection_key_enable : 1;
			u64 reserved4 : 41;
		};
	} cr4_t;

	union vmx_exit_control_t
	{
		unsigned __int64 control;
		struct
		{
			unsigned __int64 reserved_0 : 2;
			unsigned __int64 save_dbg_controls : 1;
			unsigned __int64 reserved_1 : 6;
			unsigned __int64 host_address_space_size : 1;
			unsigned __int64 reserved_2 : 2;
			unsigned __int64 load_ia32_perf_global_control : 1;
			unsigned __int64 reserved_3 : 2;
			unsigned __int64 ack_interrupt_on_exit : 1;
			unsigned __int64 reserved_4 : 2;
			unsigned __int64 save_ia32_pat : 1;
			unsigned __int64 load_ia32_pat : 1;
			unsigned __int64 save_ia32_efer : 1;
			unsigned __int64 load_ia32_efer : 1;
			unsigned __int64 save_vmx_preemption_timer_value : 1;
			unsigned __int64 clear_ia32_bndcfgs : 1;
			unsigned __int64 conceal_vmx_from_pt : 1;
		} bits;
	};

	union vmx_basic_msr_t
	{
		unsigned __int64 control;
		struct
		{
			unsigned __int64 vmcs_revision_identifier : 31;
			unsigned __int64 always_0 : 1;
			unsigned __int64 vmxon_region_size : 13;
			unsigned __int64 reserved_1 : 3;
			unsigned __int64 vmxon_physical_address_width : 1;
			unsigned __int64 dual_monitor_smi : 1;
			unsigned __int64 memory_type : 4;
			unsigned __int64 io_instruction_reporting : 1;
			unsigned __int64 true_controls : 1;
		} bits;
	};

	typedef struct _vmcs_ctx
	{
		union
		{
			unsigned int all;
			struct
			{
				unsigned int revision_identifier : 31;
				unsigned int shadow_vmcs_indicator : 1;
			} bits;
		} header;

		unsigned int abort_indicator;
		char data[0x1000 - 2 * sizeof(unsigned)];
	} vmcs_ctx, *pvmcs_ctx;

	typedef struct _vmxon_region_ctx
	{
		union
		{
			unsigned int all;
			struct
			{
				unsigned int revision_identifier : 31;
			} bits;
		} header;

		char data[0x1000 - 1 * sizeof(unsigned)];
	} vmxon_region_ctx, *pvmxon_region_ctx;

	typedef struct _vcpu_ctx
	{
		pvmxon_region_ctx vmxon;
		pvmcs_ctx vmcs;
		u64 vmcs_phys;
		u64 vmxon_phys;
		u64 host_stack;

		tss64 tss;
		segment_descriptor_64* gdt;
	} vcpu_ctx, * pvcpu_ctx;

	typedef struct _vmx_ctx
	{
		u32 vcpu_count;
		pvcpu_ctx* vcpus;
	} vmx_ctx, *pvmx_ctx;

	typedef struct _segment_info_ctx
	{
		segment_descriptor_64 segment_descriptor;
		vmx_segment_access_rights rights;
		u64 limit;
		u64 base_addr;
	} segment_info_ctx, *psegment_info_ctx;
}