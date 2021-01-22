#pragma once
#include "hv_types.hpp"

namespace vmxon
{
	auto create_vmxon_region(hv::pvcpu_ctx vcpu_ctx) -> void;
	auto create_vmcs(hv::pvcpu_ctx vcpu_ctx) -> void;
	auto create_vcpus(hv::pvmx_ctx vmx_ctx) -> void;
	auto init_vmxon() -> void;

	// vmxroot global object... contains all vcpu information...
	inline hv::pvmx_ctx g_vmx_ctx;
}