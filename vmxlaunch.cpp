#include "vmxlaunch.hpp"

auto vmxlaunch::init_vmcs(cr3 cr3_value) -> void
{
	const auto vcpu = 
		vmxon::g_vmx_ctx->vcpus[
			KeGetCurrentProcessorNumber()];

	__vmx_vmclear(&vcpu->vmcs_phys);
	__vmx_vmptrld(&vcpu->vmcs_phys);

	segment_descriptor_register_64 gdt_value;
	_sgdt(&gdt_value);

	const auto [tr_descriptor, tr_rights, tr_limit, tr_base] =
		gdt::get_info(gdt_value, segment_selector{ readtr() });

	// copy windows TSS and windows GDT... 
	// change TSS base to new TSS...
	hv::segment_descriptor_addr_t tss{ &vcpu->tss };
	memcpy(&vcpu->tss, (void*)tr_base, sizeof hv::tss64);
	memcpy(vcpu->gdt, (void*)gdt_value.base_address, PAGE_SIZE);

	vcpu->tss.interrupt_stack_table[idt::ist_idx::pf] =
		reinterpret_cast<u64>(ExAllocatePool(NonPagedPool, 
			PAGE_SIZE * HOST_STACK_PAGES)) + (PAGE_SIZE * HOST_STACK_PAGES);

	vcpu->tss.interrupt_stack_table[idt::ist_idx::gp] =
		reinterpret_cast<u64>(ExAllocatePool(NonPagedPool,
			PAGE_SIZE * HOST_STACK_PAGES)) + (PAGE_SIZE * HOST_STACK_PAGES);

	vcpu->tss.interrupt_stack_table[idt::ist_idx::de] =
		reinterpret_cast<u64>(ExAllocatePool(NonPagedPool,
			PAGE_SIZE * HOST_STACK_PAGES)) + (PAGE_SIZE * HOST_STACK_PAGES);

	vcpu->tss.interrupt_stack_table[idt::ist_idx::nmi] =
		reinterpret_cast<u64>(ExAllocatePool(NonPagedPool,
			PAGE_SIZE * HOST_STACK_PAGES)) + (PAGE_SIZE * HOST_STACK_PAGES);

	const auto tr_idx = segment_selector{ readtr() }.idx;
	vcpu->gdt[tr_idx].base_address_upper = tss.upper;
	vcpu->gdt[tr_idx].base_address_high = tss.high;
	vcpu->gdt[tr_idx].base_address_middle = tss.middle;
	vcpu->gdt[tr_idx].base_address_low = tss.low;

	vmcs::setup_host(&vmxexit_handler, 
		vcpu->host_stack, cr3_value, (u64)vcpu->gdt);

	vmcs::setup_guest();
	vmcs::setup_controls();
}

auto vmxlaunch::launch() -> void
{
	const auto vmlaunch_result = vmxlaunch_processor();
	if (vmlaunch_result != VMX_LAUNCH_SUCCESS)
	{
		u64 vmxerror;
		rflags flags{ vmlaunch_result };
		__vmx_vmread(VMCS_VM_INSTRUCTION_ERROR, &vmxerror);

		DBG_PRINT("vmxerror: %d\n", vmxerror);
		DBG_PRINT("eflags.zero_flag: %d\n", flags.zero_flag);
		DBG_PRINT("eflags.carry_flag: %d\n", flags.carry_flag);
	}
}