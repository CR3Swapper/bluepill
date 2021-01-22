#include "vmxlaunch.hpp"

auto driver_unload(
	PDRIVER_OBJECT driver_object
) -> void
{
	// TODO vmcall and ask HV to vmxoff...
	KeIpiGenericCall(
		[](ULONG_PTR) -> ULONG_PTR 
		{
			__vmx_off();
			hv::cr4_t cr4 = { __readcr4() };
			cr4.vmx_enable = false;
			__writecr4(cr4.flags);

			hv::ia32_feature_control_msr_t feature_msr = { __readmsr(IA32_FEATURE_CONTROL) };
			feature_msr.bits.vmxon_outside_smx = false;
			feature_msr.bits.lock = false;
			__writemsr(IA32_FEATURE_CONTROL, feature_msr.control);
			return NULL;
		}, NULL
	);

	for (auto idx = 0u; idx < vmxon::g_vmx_ctx->vcpu_num; ++idx)
	{
		MmFreeContiguousMemory(vmxon::g_vmx_ctx->vcpus[idx]->vmcs);
		MmFreeContiguousMemory(vmxon::g_vmx_ctx->vcpus[idx]->vmxon);

		ExFreePool((void*)vmxon::g_vmx_ctx->vcpus[idx]->host_stack);
		ExFreePool(vmxon::g_vmx_ctx->vcpus[idx]);
	}

	ExFreePool(vmxon::g_vmx_ctx->vcpus);
	ExFreePool(vmxon::g_vmx_ctx);
}

auto driver_entry(
	PDRIVER_OBJECT driver_object,
	PUNICODE_STRING registry_path
) -> NTSTATUS
{
	vmxon::g_vmx_ctx = 
		reinterpret_cast<hv::pvmx_ctx>(
			ExAllocatePool(NonPagedPool, sizeof hv::vmx_ctx));

	// setup vcpu structures (vmx on region and vmcs...)
	vmxon::create_vcpus(vmxon::g_vmx_ctx);

	// enable vmx operation on all cores...
	KeIpiGenericCall((PKIPI_BROADCAST_WORKER)&vmxon::init_vmxon, NULL);

	// setup VMCS for all logical cores...
	KeIpiGenericCall((PKIPI_BROADCAST_WORKER)&vmxlaunch::init_vmcs, NULL);

	// vmxlaunch for all cores...
	// KeIpiGenericCall((PKIPI_BROADCAST_WORKER)&vmxlaunch::launch, NULL);

	driver_object->DriverUnload = &driver_unload;
	return STATUS_SUCCESS;
}