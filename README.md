omap_jbx_kernel
===============

JBX-Kernel for Motorola XT910/XT912 OMAP4 (D-WiZ JBX Rom Series - Battery Saving Technologies)

This Kernel is based on the Motorola 3.0.8 Kernel for Kexec which was initiated by the STS-Dev-Team.
See this link for the original source: https://github.com/STS-Dev-Team/kernel_mapphone_kexec
See credits below

There are two different versions, JBX-Kernel and JBX-Kernel Battery Saver Edition



Some features of JBX-Kernel:

- Underclocked (stable 100mhz min frequency)
- Undervolted (~10mV)
- Modified Smartreflex driver with Custom Sensor Values to Override factory defaults. 
  This allows us to overclock stable to ~1,5ghz!
- Smartreflex tunables control interface
- fsync Control
- Gamma Control
- Custom Voltage Control
- Live Overclock Interface
- GPU Overclock
- GPU frequency control (and sysfs interface)
- Custom OPP-Table
- Reduced latency
- Tweaked governors
- Support for Trickster Mod App (from Bigeyes0x0)
- ZRAM
- Increased R/W Ahead
- IO-Schedulers: Row, VR, Fifo, Sio, CFQ, Deadline, Noop
- Governors: Pegasusq, Sacuractive, Interactive, InteractiveX, Conservative, Ondemand, Userspace, Hotplug, Performance
- For OCE: 300 mhz min freq - Highspeed OPP 1,35 ghz (stabe OC up to ~1,5ghz)
- For BSE: 100 mhz min freq - 1 ghz max freq
- Wifi power management
- Watchdog reduced consumption
- USB MTP / MASS STORAGE Serial FIXED

- Overclocking Kernel Modules based on Milestone Overclock (Ported by Whirleyes) [NOT INCLUDED IN BUILD BECAUSE WE HAVE LIVE OC]
	---> /drivers/extra/ [include this folder in Makefile to build. Needs precompiled kernel!!)


...more to come


JBX-Kernel BSE hast almost the same features and 
configuration except voltage, max frequency policy
and some other values to reach more power saving.



WIP:

- Fix Touch Wake
- Removed Temp Control (is only neccessary for OMAP4460+)





*Credits to 

- Kholk & [mbm] for Kexec Initial Release
- Hashcode & dhacker29 (STS-Dev-Team) for fixing Kexec, Base Kernel release and all their great work!
- Bigeyes0x0 for Trickster Mod App and support
- Biotik / Bionx NX - Catalyst/JBX-CoDevelopment
- XDA-Developers - great community!

