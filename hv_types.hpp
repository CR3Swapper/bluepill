#pragma once
#include <ntifs.h>
#include <intrin.h>
#include "ia32.hpp"

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
		u64 rax;
	} guest_registers, * pguest_registers;

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
		pvmcs_ctx vmcs;
		u64 vmcs_phys;

		pvmxon_region_ctx vmxon;
		u64 vmxon_phys;

		u64 host_stack;
	} vcpu_ctx, * pvcpu_ctx;

	typedef struct _vmx_ctx
	{
		u32 vcpu_num;
		pvcpu_ctx* vcpus;
	} vmx_ctx, *pvmx_ctx;

	typedef struct _segment_info_ctx
	{
		vmx_segment_access_rights rights;
		u64 limit;
		u64 base_addr;
	} segment_info_ctx, *psegment_info_ctx;
}