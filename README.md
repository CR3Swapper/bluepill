<div align="center">
  <img width="1000" height="auto" src="https://imgur.com/b1bYNZU.png"/>
  <p>Figure 1. First ever vmexit...</p>
</div>

# Bluepill

Bluepill is an Intel type-2 research hypervisor written with no access to github.com. This project is purely for educational purposes and is designed to run on Windows 10 systems.
This project uses WDK and thus Windows Kernel functions to facilitate vmxlaunch.

### VMCS

This section of the readme just contains note and a list of things i stumbled on and took me a while to figure out and fix.

### VMCS Controls

* One of the mistakes I made early on was setting bits high after applying high/low MSR values. For example my xeons dont support Intel Processor Trace (Intel PT) and I was setting `entry_ctls.conceal_vmx_from_pt = true` after applying the MSR high/low masks. This caused vmxerror #7 (invalid vmcs controls). Now i set the bit high before i apply the high/low bit mask so if my hypervisor runs on a cpu that has Intel PT support it will be concealed from Intel PT.

Dump of VMCS control fields can be found [here](https://githacks.org/_xeroxz/bluepill/-/blob/master/VMCS.md). This is not required, but for learning its nice to
see exactly what the MSR masks are, and what VMCS field's are enabled after you apply high/low bit masks.