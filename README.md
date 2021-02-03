<div align="center">
  <img width="1000" height="auto" src="https://imgur.com/b1bYNZU.png"/>
  <p>Figure 1. First ever vmexit...</p>
</div>

# Bluepill

Bluepill is an Intel type-2 research hypervisor written with no access to github.com. This project is purely for educational purposes and is designed to run on Windows 10 systems.
This project uses WDK and thus Windows Kernel functions to facilitate vmxlaunch.

### VMCS

Dump of VMCS control fields can be found [here](https://githacks.org/_xeroxz/bluepill/-/blob/master/VMCS.md). This is not required, but for learning its nice to
see exactly what the MSR masks are, and what VMCS field's are enabled after you apply high/low bit masks.