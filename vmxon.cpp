#include "vmxon.hpp"

namespace vmxon
{
	auto create_vmxon_region(hv::pvcpu_ctx vcpu_ctx) -> void
	{
		PHYSICAL_ADDRESS mem_range;
		mem_range.QuadPart = ~0ull;

		hv::vmx_basic_msr_t vmx_basic;
		vmx_basic.control = __readmsr(IA32_VMX_BASIC);

		vcpu_ctx->vmxon_phys =
			MmGetPhysicalAddress(&vcpu_ctx->vmxon).QuadPart;

		vcpu_ctx->vmxon
			.header
			.bits
			.revision_identifier =
				vmx_basic.bits
					.vmcs_revision_identifier;
	}

	auto create_vmcs(hv::pvcpu_ctx vcpu_ctx) -> void
	{
		PHYSICAL_ADDRESS mem_range;
		mem_range.QuadPart = ~0ull;

		hv::vmx_basic_msr_t vmx_basic;
		vmx_basic.control = __readmsr(IA32_VMX_BASIC);

		vcpu_ctx->vmcs_phys =
			MmGetPhysicalAddress(&vcpu_ctx->vmcs).QuadPart;

		vcpu_ctx->vmcs
			.header
			.bits
			.revision_identifier =
				vmx_basic.bits
					.vmcs_revision_identifier;
	}

	auto create_vcpus(hv::pvmx_ctx vmx_ctx) -> void
	{
		vmx_ctx->vcpu_count = 
			KeQueryActiveProcessorCountEx(
				ALL_PROCESSOR_GROUPS);

		for (auto idx = 0u; idx < vmx_ctx->vcpu_count; ++idx)
		{
			create_vmxon_region(&vmx_ctx->vcpus[idx]);
			create_vmcs(&vmx_ctx->vcpus[idx]);
		}
	}

	auto init_vmxon() -> void
	{
		hv::cr_fixed_t cr_fixed;
		hv::cr0_t cr0 = { 0 };
		hv::cr4_t cr4 = { 0 };

		cr_fixed.all = __readmsr(IA32_VMX_CR0_FIXED0);
		cr0.flags = __readcr0();
		cr0.flags |= cr_fixed.split.low;
		cr_fixed.all = __readmsr(IA32_VMX_CR0_FIXED1);
		cr0.flags &= cr_fixed.split.low;
		__writecr0(cr0.flags);

		cr_fixed.all = __readmsr(IA32_VMX_CR4_FIXED0);
		cr4.flags = __readcr4();
		cr4.flags |= cr_fixed.split.low;
		cr_fixed.all = __readmsr(IA32_VMX_CR4_FIXED1);
		cr4.flags &= cr_fixed.split.low;
		__writecr4(cr4.flags);

		const auto vmxon_result = 
			__vmx_on((unsigned long long*)
				&vmxon::g_vmx_ctx.vcpus[
					KeGetCurrentProcessorNumber()].vmxon_phys);
	}
}