<div align="center">
  <img width="1000" height="auto" src="https://imgur.com/b1bYNZU.png"/>
  <p>Figure 1. First ever vmexit...</p>
</div>

# Bluepill

Bluepill is an Intel type-2 research hypervisor written with no access to github.com. This project is purely for educational purposes and is designed to run on Windows 10 systems.
This project uses WDK and thus Windows Kernel functions to facilitate vmxlaunch.

### VMCS

Dump of VMCS fields can be found [here](https://githacks.org/_xeroxz/bluepill/-/blob/master/VMCS.md). This is not required, but for learning its nice to
see exactly what the masks are, what VMCS field's there are and what exactly you need to vmxlaunch successfully.