#include "vmxlaunch.hpp"
#include "idt.hpp"

auto drv_entry(PDRIVER_OBJECT driver_object, PUNICODE_STRING registry_path) -> NTSTATUS
{
	vmxon::g_vmx_ctx = 
		reinterpret_cast<hv::pvmx_ctx>(
			ExAllocatePool(NonPagedPool, sizeof hv::vmx_ctx));

	// setup vcpu structures (vmx on region and vmcs...)
	vmxon::create_vcpus(vmxon::g_vmx_ctx);

	cr3 cr3_value;
	cr3_value.flags = __readcr3();
	cr3_value.address_of_page_directory =
		(MmGetPhysicalAddress(mm::pml4).QuadPart >> 12);

	memset(mm::pml4, NULL, sizeof mm::pml4);
	mm::pml4[PML4_SELF_REF].pfn = cr3_value.address_of_page_directory;
	mm::pml4[PML4_SELF_REF].present = true;
	mm::pml4[PML4_SELF_REF].rw = true;
	mm::pml4[PML4_SELF_REF].user_supervisor = false;

	PHYSICAL_ADDRESS current_pml4;
	current_pml4.QuadPart =
		(cr3{ __readcr3() }.address_of_page_directory << 12);

	const auto kernel_pml4 =
		reinterpret_cast<mm::ppml4e>(
			MmGetVirtualForPhysical(current_pml4));

	// vmxroot will have the same "address space" as the current one being executed in...
	memcpy(&mm::pml4[255], &kernel_pml4[255], sizeof(mm::pml4e) * 255);

	// setup mapping ptes to be present, writeable, executable, and user supervisor false...
	for (auto idx = 0u; idx < 254; ++idx)
	{
		reinterpret_cast<mm::ppte>(mm::pml4)[idx].present = true;
		reinterpret_cast<mm::ppte>(mm::pml4)[idx].rw = true;
	}

	// setup IDT for host....
	segment_descriptor_register_64 idt_value;
	__sidt(&idt_value);

	// copy the guest IDT entries...
	memcpy(idt::table, (void*)idt_value.base_address, idt_value.limit);

	// change gp, pf, and de to vmxroot handlers...
	idt::table[general_protection] = idt::create_entry(hv::idt_addr_t{ __gp_handler }, idt::ist_idx::gp);
	idt::table[page_fault] = idt::create_entry(hv::idt_addr_t{ __pf_handler }, idt::ist_idx::pf);
	idt::table[divide_error] = idt::create_entry(hv::idt_addr_t{ __de_handler }, idt::ist_idx::de);

	// used for SEH in vmxroot fault handler...
	idt::image_base = driver_object->DriverStart;

	// enable vmx operation on all cores...
	KeIpiGenericCall((PKIPI_BROADCAST_WORKER)&vmxon::init_vmxon, NULL);

	// setup VMCS for all logical cores...
	KeIpiGenericCall((PKIPI_BROADCAST_WORKER)&vmxlaunch::init_vmcs, cr3_value.flags);

	// vmxlaunch for all cores...
	KeIpiGenericCall((PKIPI_BROADCAST_WORKER)&vmxlaunch::launch, NULL);
	return STATUS_SUCCESS;
}