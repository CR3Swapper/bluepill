#pragma once
#include "hv_types.hpp"

inline auto get_cpu_num() -> u32
{
	cpuid_eax_01 cpuid_value;
	__cpuid((int*)&cpuid_value, 1);

	return cpuid_value
		.cpuid_additional_information
		.initial_apic_id;
}

#define g_vcpu \
	vmxon::g_vmx_ctx->vcpus[get_cpu_num()]

namespace vmxon
{
	auto create_vmxon_region(hv::pvcpu_ctx vcpu_ctx) -> void;
	auto create_vmcs(hv::pvcpu_ctx vcpu_ctx) -> void;
	auto create_vcpus(hv::pvmx_ctx vmx_ctx) -> void;
	auto init_vmxon() -> void;

	// vmxroot global object... contains all vcpu information...
	inline hv::pvmx_ctx g_vmx_ctx;
}