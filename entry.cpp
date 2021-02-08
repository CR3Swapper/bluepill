#include "vmxlaunch.hpp"

auto driver_unload(
	PDRIVER_OBJECT driver_object
) -> void
{
	// test to see if invalid opcode happens...
	__vmx_off();
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
	KeIpiGenericCall((PKIPI_BROADCAST_WORKER)&vmxlaunch::launch, NULL);

	driver_object->DriverUnload = &driver_unload;
	return STATUS_SUCCESS;
}