omap_jbx_kernel
===============

JBX-Kernel for Motorola XT910/XT912 OMAP4 (D-WiZ JBX Rom Series - Battery Saving Technologies)

This Kernel is based on the Motorola 3.0.8 Kernel for Kexec which was initiated by the STS-Dev-Team.
See this link for the original source: https://github.com/STS-Dev-Team/kernel_mapphone_kexec
See credits below

There are two different versions, JBX-Kernel and JBX-Kernel Battery Saver Edition



Some features of JBX-Kernel:


-    Optimized OPP Table for smooth CPU scaling
-    Frequencies OCE: 300, 600, 800, 1000, 1200, 1350
-    Frequencies BSE: 100, 200, 300, 400, 600, 800, 1000
-    Modifed Smartreflex driver
-    SR Custom Sensor
-    Smartreflex Tuning Interface: Set min calibrated voltage
-    Overriden trim quirks to override factoy defaults and reach stable overclocking up to ~1,5ghz (depends on device, each silicon is different!)
-    OCE: Added a highspeed OPP (1,35ghz: Use Live OC to overclock. Mine can run on 1,485ghz stable.)
-    Dvfs, Emif: Reconfigured
-    Net: Increased IPsec/TCP Up/Down Speed
-    Wifi: Power
-    Decreased Latency
-    Increased RW AHEAD (2048)
-    zRAM supported
-    GPU has 3 scaling steps and OC to 416mhz
-    GPU: Added C4 states for Power Down mode
-    GPU_OC: max 416 mhz
-    Multicore Power Saving Mode Control: Set it to 1 for best results
-    ARCH Dependant Power feature
-    Temp Control (Set your preferred temperature limit for CPU)
-    Gamma Control
-    Screen/Display: Modified OMAPDSS for sharpness and lightning colors
-    OMAPDSS: Aded clocks and 2 OPPs - allows the screen to scale down power and voltage
-    Fsync Control: LEss IO Speed but safes you from loosing data if phone crashes
-    Filesystem, Android Staging, lowmemkiller: Heavy modified for R/W Speed and efficient performance
-    Kernel code generation improved by LZO and compression enhanced
-    Custom Voltage Support (BSE undervolted by default)
-    Live Overclock Support (Overclocking on running device)
-    IO: SIO, Fifo, Row, VR, Noop, Deadline, CFQ
-    CPU: More Governors
-    Deep Idle
-    ARM Topology
-    Many improvement in overall OMAP PM
-    GREAT performance!
-    GREAT battery life!
-    Support for Ezekeel's "eXperience" App (Adds features: Touchwake, Musiccontrol)
-    Support for Trickster Mod Kernel Control App (Free version included)
-    IO-Schedulers: Row, VR, Fifo, SioPlus, CFQ, Deadline, Noop
-    Governors: Pegasusq, Sacuractive, Interactive, InteractiveX, Conservative, Ondemand, Userspace, Hotplug, Performance, Lulzactivew
-    Watchdog reduced consumption
-    NMI Watchdog OFF by default
-    USB MTP / MASS STORAGE Serial FIXED

-    Overclocking Kernel Modules based on Milestone Overclock (Ported by Whirleyes) [NOT INCLUDED IN BUILD BECAUSE WE HAVE LIVE OC]
	---> /drivers/extra/ [include this folder in Makefile to build. Needs precompiled kernel!!)


...more to come


JBX-Kernel BSE hast almost the same features and 
configuration except voltage, max frequency policy
and some other values to reach more power saving.



WIP:

- Fix Touch Wake
- Fix 
- Removed Temp Control (is only neccessary for OMAP4460+)




For releases and support check the XDA-Thread: http://forum.xda-developers.com/showthread.php?t=2223517



*Credits to 

- Kholk & [mbm] for Kexec Initial Release
- Hashcode & dhacker29 (STS-Dev-Team) for fixing Kexec, Base Kernel release and all their great work!
- Ezekeel, Imoseyon - for source code of: Live OC, Custom Coltage, SLQB, GPU OC, Optimized CRC32 Algorithm, Touch Wake, Wheatley, Lazy, Fsync, Gamma Control, more...
- Bigeyes0x0 for Trickster Mod App and support
- Biotik / Bionx NX - Catalyst/JBX-CoDevelopment
- XDA-Developers - great community!

