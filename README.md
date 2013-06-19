omap4_jbx_kernel
===============

JBX-Kernel for Motorola XT910/XT912 OMAP4 (D-WiZ JBX Rom Series - Battery Saving Technologies) [AOSP, JB]

This Kernel is based on the Motorola 3.0.8 Kernel for Kexec which was initiated by the STS-Dev-Team.
See this link for the original source: https://github.com/STS-Dev-Team/kernel_mapphone_kexec
See credits below




[JBX-Kernel-Hybrid]
-------------------


Some features of JBX-Kernel:


[*]Optimized OPP Table for smooth CPU scaling
[*]Frequencies: 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000, 1100, 1200, 1350
[*]Modifed Smartreflex driver (Custom Sensor for detecting the N-Value. Allows better OC handling, etc)
[*]SR Custom Sensor
[*]Smartreflex Tuning Interface: Set min calibrated voltage
[*]Smartreflex Tuning Interface: Set max calibrated voltage
[*]Overriden trim quirks (experimental port from OMAP4460, no DCC) to override factoy defaults and reach stable overclocking up to ~1,5ghz (depends on device, each silicon is different!)
[*]Overclocking using Live OC (mine runs stable at a maximum frequency of 1,498ghz!)
[*]Dvfs, Emif: Reconfigured
[*]Raised voltage limits for mpu a bit
[*]Raised the temperature limits from 64c* to 74c* (degrees)
[*]Net: Increased IPsec/TCP Up/Down Speed
[*]Using optimized CRC32 algorithm (better code generation)
[*]mm: dirty writeback interval only active during screen off state and interval reduced to 1500csec to safe power
[*]block: strict rq_affinity
[*]Optimized LZO compression
[*]fs: asynchronus I/O latency to solid-state disk 
[*]Net: Wireless: Wifi included - not a module
[*]Net: Wireless: Removed a return rule to get rid of the rx_wake/wl_wake wakelocks
[*]CPU: policy->min set to 100mhz without an "if-rule" to avoid user to be stuck at 100mhz min. Sideeffect is that you have to set the frequency lock in trickster to force the CPU to stay on your preferred min frequency, otherwise it will always jump back to 300mhz.
[*]Wifi: Power Saving Mode by kernel dafault
[*]RW Readahead 128
[*]Watchdogs disabled
[*]ARM: Allow arm_memblock_steal() to remove memory from any RAM region
[*]Touchscreen: Added scalable frequencies, variable clock rates and power down state for display to save power
[*]Devices: Deeper idle states
[*]Increased ducati heap size
[*]zRAM support
[*]GPU has 3 scaling steps and OC to 416mhz
[*]GPU: Added C4 states for Power Down mode
[*]GPU_OC: max 416 mhz
[*]Multicore Power Saving Mode Control
[*]ARCH Dependant Power feature
[*]Temp Control (Modify temperature limit for CPU/MPU)
[*]Gamma Control
[*]Screen/Display: Modified OMAPDSS for sharpness and lightning colors
[*]OMAPDSS: Aded clocks and 2 OPPs - allows the screen to scale down power and voltage
[*]Fsync Control: LEss IO Speed but safes you from loosing data if phone crashes
[*]Filesystem, Android Staging, lowmemkiller: Heavy modified for R/W Speed and efficient performance
[*]Kernel code generation improved by LZO and compression enhanced
[*]Custom Voltage Support (BSE undervolted by default)
[*]Live Overclock Support (Overclocking on running device)
[*]IO: SIOPlus, Fifo, Row, VR, Noop, Deadline, CFQ
[*]CPU: More Governors
[*]Deep Idle
[*]ARM Topology
[*]Many improvement in overall OMAP PM
[*]GREAT performance!
[*]GREAT battery life!
[*]Support for Ezekeel's "eXperience" App (Adds features: Music Control with Vol-Buttons, etc..)
[*]Support for Trickster Mod Kernel Control App (Download from Gplay)

-    Overclocking Kernel Modules based on Milestone Overclock (Ported by Whirleyes) [NOT INCLUDED IN BUILD BECAUSE WE HAVE LIVE OC]
	---> /drivers/extra/ [include this folder in Makefile to build. Needs precompiled kernel!!)


...more to come


WIP:

- Fix Touch Wake
- Fix BLX
- Fix battery (also in Roms, battd)



For releases and support check the XDA-Thread: http://forum.xda-developers.com/showthread.php?t=2223517



*Credits to 

- Kholk & [mbm] for Kexec Initial Release
- Hashcode & dhacker29 (STS-Dev-Team) for fixing Kexec, Base Kernel release and all their great work!
- Ezekeel, Imoseyon - for source code of: Live OC, Custom Coltage, SLQB, GPU OC, Optimized CRC32 Algorithm, Touch Wake, Wheatley, Lazy, Fsync, Gamma Control, more...
- Bigeyes0x0 for Trickster Mod App and support
- Biotik / Bionx NX - Catalyst/JBX-CoDevelopment
- XDA-Developers - great community!

