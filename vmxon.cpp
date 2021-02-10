#include "vmxon.hpp"

namespace vmxon
{
	auto create_vmxon_region(hv::pvcpu_ctx vcpu_ctx) -> void
	{
		PHYSICAL_ADDRESS mem_range;
		mem_range.QuadPart = ~0ull;

		hv::vmx_basic_msr_t vmx_basic;
		vmx_basic.control = __readmsr(IA32_VMX_BASIC);

		vcpu_ctx->vmxon =
			reinterpret_cast<hv::pvmxon_region_ctx>(
				MmAllocateContiguousMemory(PAGE_SIZE, mem_range));

		vcpu_ctx->vmxon_phys =
			MmGetPhysicalAddress(vcpu_ctx->vmxon).QuadPart;

		RtlSecureZeroMemory(
			vcpu_ctx->vmxon, PAGE_SIZE);

		vcpu_ctx->vmxon->header
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

		vcpu_ctx->vmcs =
			reinterpret_cast<hv::pvmcs_ctx>(
				MmAllocateContiguousMemory(PAGE_SIZE, mem_range));

		vcpu_ctx->vmcs_phys =
			MmGetPhysicalAddress(vcpu_ctx->vmcs).QuadPart;

		RtlSecureZeroMemory(
			vcpu_ctx->vmcs, PAGE_SIZE);

		vcpu_ctx->vmcs->header
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

		vmx_ctx->vcpus = 
			reinterpret_cast<hv::pvcpu_ctx*>(
				ExAllocatePool(NonPagedPool, 
					sizeof(hv::pvcpu_ctx) * vmx_ctx->vcpu_count));

		for (auto idx = 0u; idx < g_vmx_ctx->vcpu_count; ++idx)
		{
			vmx_ctx->vcpus[idx] =
				reinterpret_cast<hv::pvcpu_ctx>(
					ExAllocatePool(NonPagedPool, sizeof hv::vcpu_ctx));

			vmx_ctx->vcpus[idx]->host_stack = 
				reinterpret_cast<u64>(
					ExAllocatePool(NonPagedPool,
						PAGE_SIZE * HOST_STACK_PAGES));

			RtlZeroMemory(reinterpret_cast<void*>(
				vmx_ctx->vcpus[idx]->host_stack), PAGE_SIZE * HOST_STACK_PAGES);

			create_vmxon_region(vmx_ctx->vcpus[idx]);
			create_vmcs(vmx_ctx->vcpus[idx]);
		}
	}

	auto init_vmxon() -> void
	{
		hv::ia32_feature_control_msr_t feature_msr = { 0 };
		hv::cr_fixed_t cr_fixed;
		hv::cr0_t cr0 = { 0 };
		hv::cr4_t cr4 = { 0 };

		feature_msr.control = __readmsr(IA32_FEATURE_CONTROL);
		feature_msr.bits.vmxon_outside_smx = true;
		feature_msr.bits.lock = true;
		__writemsr(IA32_FEATURE_CONTROL, feature_msr.control);

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

		cr4.flags = __readcr4();
		cr4.vmx_enable = true;
		__writecr4(cr4.flags);

		const auto vmxon_result = 
			__vmx_on((unsigned long long*)
				&vmxon::g_vmx_ctx->vcpus[
					KeGetCurrentProcessorNumber()]->vmxon_phys);
	}
}