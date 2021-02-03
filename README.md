<div align="center">
  <img width="1000" height="auto" src="https://imgur.com/b1bYNZU.png"/>
  <p>Figure 1. First ever vmexit...</p>
</div>

# Bluepill

Bluepill is an Intel type-2 research hypervisor written with no access to github.com. This project is purely for educational purposes and is designed to run on Windows 10 systems.
This project uses WDK and thus Windows Kernel functions to facilitate vmxlaunch.

### VMCS War Stories

Dump of VMCS control fields can be found [here](https://githacks.org/_xeroxz/bluepill/-/blob/master/VMCS.md). This is not required, but for learning its nice to
see exactly what the MSR masks are, and what VMCS field's are enabled after you apply high/low bit masks. When I first configured the VMCS control field(s), I was setting whatever bits I thought I needed high after 
applying VMX reserved bit masks. 

```cpp
msr_fix_value.flags = __readmsr(IA32_VMX_PROCBASED_CTLS2);
procbased_ctls2.flags &= msr_fix_value.allowed_1_settings;
procbased_ctls2.flags |= msr_fix_value.allowed_0_settings;

// dont do this! for example my xeons dont support xsave/xrstor instruction...
// nor do my xeons have processor tracing support...
procbased_ctls2.enable_rdtscp = true;
procbased_ctls2.enable_xsaves = true; 
procbased_ctls2.conceal_vmx_from_pt = true; 
__vmx_vmwrite(VMCS_CTRL_SECONDARY_PROCESSOR_BASED_VM_EXECUTION_CONTROLS, procbased_ctls2.flags);
```

This was causing vmxerror #7 (control field misconfiguration). Also I found out my xeons dont support xsave, nor do they 
support [processor tracing](https://software.intel.com/content/www/us/en/develop/blogs/processor-tracing.html).

Instead set bits high before you apply the mask... brutal.

```cpp
msr_fix_value.flags = __readmsr(IA32_VMX_PROCBASED_CTLS2);
procbased_ctls2.enable_rdtscp = true;
procbased_ctls2.enable_xsaves = true; // although my xeons dont support xsave... other cpus do!
procbased_ctls2.conceal_vmx_from_pt = true; // although my xeons dont support processor tracing... other cpus do!
procbased_ctls2.flags &= msr_fix_value.allowed_1_settings;
procbased_ctls2.flags |= msr_fix_value.allowed_0_settings;
__vmx_vmwrite(VMCS_CTRL_SECONDARY_PROCESSOR_BASED_VM_EXECUTION_CONTROLS, procbased_ctls2.flags);
```