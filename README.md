<div align="center">
  <img width="1000" height="auto" src="https://imgur.com/b1bYNZU.png"/>
  <p>Figure 1. First ever vmexit...</p>
</div>

# Bluepill

Bluepill is an Intel type-2 research hypervisor. This project is purely for educational purposes and is designed to run on Windows 10 systems.
This project uses WDK and thus Windows Kernel functions to facilitate vmxlaunch. 

### Why Write A Hypervisor?

Why write a type-2 (Intel or AMD) hypervisor? "To learn" is the typical response, but to learn what? To learn VMX instructions? To learn how to write a windows kernel driver? To learn how to use windbg? Although all of the prior reasons to write a hypervisor are important, learning how to read technical documents and extract what you need from the reading material is much more valuable than all of the other stuff one might learn while writing a hypervisor. This is best summed up as the old saying goes: 

> “Give a man a fish and you feed him for a day. Teach a man to fish and you feed him for a lifetime”



### VMCS

This section of the readme just contains notes and a list of things I stumbled on and took me a while to figure out and fix.

##### VMCS Controls

* One of the mistakes I made early on was setting bits high after applying high/low MSR values. For example my xeons dont support Intel Processor Trace (Intel PT) and I was setting `entry_ctls.conceal_vmx_from_pt = true` after applying the MSR high/low masks. This caused vmxerror #7 (invalid vmcs controls). Now i set the bit high before i apply the high/low bit mask so if my hypervisor runs on a cpu that has Intel PT support it will be concealed from Intel PT.
* My xeons also dont support xsave/xrstor and I was setting enable_xsave in secondary processor based vmexit controls after applying `IA32_VMX_PROCBASED_CTLS2` high/low bitmask. Which caused vmxerror #7 (invalid vmcs controls).

Dump of VMCS control fields can be found [here](https://githacks.org/_xeroxz/bluepill/-/blob/master/VMCS-CONTROLS.md). This is not required, but for learning its nice to
see exactly what the MSR masks are, and what VMCS field's are enabled after you apply high/low bit masks.

##### VMCS Guest State

* After getting my first vmexit the exit reason was 0x80000021 (invalid guest state). I thought it was segmentation code since I've never done anything with segments before but after a few days of checking every single segment check in chapter 26 section 3, I continued reading the guest requirements in chapter 24  section 4, part 2 goes over non-register states and I was not setting  `VMCS_GUEST_ACTIVITY_STATE` to zero. 

Dump of VMCS guest fields can be found [here](https://githacks.org/_xeroxz/bluepill/-/blob/master/VMCS-GUEST.md). 