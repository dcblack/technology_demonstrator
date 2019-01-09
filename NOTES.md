Notes on tools
==============

These are some general notes that may be useful when bringing up the software.

General
-------
On MacOSX:

- Obtain and install [macports](http://www.macports.org)
- `port install screen`, will provide a terminal emulator
- `port install clang-3.3`, will provide C++11 support

On CentOS Linux:

- See etc/devtools.repo, which may need installing under /etc/yum.repos.d
- `yum install devtoolset-1.1`, which will provide GCC 4.7 supporting C++11
- `yum install screen`, which will provide a terminal emulator


COMPILING HINTS
---------------

Get and install arm-xilinx-linux-gnueabi via Xilinx version 14.4 or higher
Vivado toolset with the SDK

- Uses Code Sourcery cross-compiler for ARM on Xilinx with the LINUX libraries

Get and activate LLVM clang++ compiler for MacOSX 10.8 (Mountain Lion x86_64)

- `port select gcc mp-clang3.3`

Activate GCC g++ 4.7 compiler for Centos Linux 5.8

- `source /opt/centos/devtoolset-1.0/enable` will setup search paths

Be sure to remove any of the following from your environment (they are bad for
your sanity):

- `unsetenv GCC_EXEC_PREFIX`
- `unsetenv C_INCLUDE_PATH`
- `unsetenv CPLUS_INCLUDE_PATH`
- `unsetenv LD_LINK_LIBRARY`
- `unsetenv LD_LIBRARY_PATH`
- `unsetenv DYLD_LIBRARY_PATH`

The makefiles provided assume GNU make (aka gmake) version 3.82 or better.

RUNNING HINTS
-------------

### Connecting via Ethernet and copying via SCP ###

When zedboard powers up using the default linux image, it provides ftp, telnet
and ssh access via TCP address 192.168.1.10.

If you are unable to connect to the ethernet port, you can change it via the
UART port using:

    zynq> `ifconfig eth0 ###.###.###.###`

Telnet to the zedboard from the Linux SystemC host and then use netstat on the
host system to determine the return address. You will need two terminal windows
on the host machine. The following assumes the default TCP/IP # for the ZedBoard
(192.168.1.10):

From 1st window

```
    % telnet 192.168.1.10
    / # passwd
    Changing password for root
    New password: S3Cr3t!
    Retype password: S3Cr3t!
    Password for root changed by root
    / # exit  #<<<=== (after step 2 below is completed)
    %
```

From 2nd window

```
    % netstat -p TCP | grep 192.168.1.10
    Active Internet connections
    Proto Recv-Q Send-Q Local Address          Foreign Address      (state)    
    tcp4       0      0 **192.168.1.12.51080** 192.168.1.10.telnet  ESTABLISHED
    tcp4       0      0  192.168.207.242.51071 204.152.18.206.https ESTABLISHED
    ^C
    % scp software.zed root@192.168.1.10:/ #<<<=== copy software onto zedboard
    root@192.168.1.10's password: R*****!
    software.zed                                              100%   65KB  64.9KB/s   00:00    
    %
```

### Connecting to the USB UART ###

After connecting the USB UART, 'ls /dev' and look for the TTY representing the
zedboard connection.  On MacOSX, it is called '/dev/tty.usbmodem14221'. On
CentOS, it is called '/dev/ttyACM0'.

Connect to zedboard with `screen DEVICE_PATH 115200`.

- On MacOSX: `screen /dev/tty.usbmodem1a12131 115200`
- On Linux:  `screen /dev/tty.

Hit the return key or power cycle. You should see something like:

    U-Boot 2011.03-dirty (Jul 11 2012 - 16:07:00)
    
    DRAM:  512 MiB
    MMC:   SDHCI: 0
    Using default environment
    
    In:    serial
    Out:   serial
    Err:   serial
    Net:   zynq_gem
    Hit any key to stop autoboot:  3...2...1...0
    Copying Linux from SD to RAM...
    Device: SDHCI
    Manufacturer ID: 12
    OEM: 3456
    Name: SMI   
    Tran Speed: 25000000
    Rd Block Len: 512
    SD version 1.10
    High Capacity: Yes
    Capacity: 3967811584
    Bus Width: 1-bit
    reading zImage
    
    2479640 bytes read
    reading devicetree_ramdisk.dtb
    
    5817 bytes read
    reading ramdisk8M.image.gz
    
    3694108 bytes read
    ## Starting application at 0x00008000 ...
    Uncompressing Linux... done, booting the kernel.
    [0.000000] Booting Linux on physical CPU 0
    [0.000000] Linux version 3.3.0-digilent-12.07-zed-beta (tinghui.wang@DIGILENT_LINUX) (gcc version 4.6.1 (Sourcery CodeBench Lite 2011.09-50) ) #2 SMP PREEMPT Thu Jul 12 21:01:42 PDT 2012
    [0.000000] CPU: ARMv7 Processor [413fc090] revision 0 (ARMv7), cr=18c5387d
    [0.000000] CPU: PIPT / VIPT nonaliasing data cache, VIPT aliasing instruction cache
    [0.000000] Machine: Xilinx Zynq Platform, model: Xilinx Zynq ZED
    [0.000000] bootconsole [earlycon0] enabled
    [0.000000] Memory policy: ECC disabled, Data cache writealloc
    [0.000000] BUG: mapping for 0xf8f00000 at 0xfe00c000 out of vmalloc space
    [0.000000] BUG: mapping for 0xe0000000 at 0xfe000000 out of vmalloc space
    [0.000000] BUG: mapping for 0xffff1000 at 0xfe200000 out of vmalloc space
    [0.000000] PERCPU: Embedded 7 pages/cpu @c1489000 s5696 r8192 d14784 u32768
    [0.000000] Built 1 zonelists in Zone order, mobility grouping on.  Total pages: 125824
    [0.000000] Kernel command line: console=ttyPS0,115200 root=/dev/ram rw initrd=0x800000,8M earlyprintk rootfstype=ext4 rootwait devtmpfs.mount=0
    [0.000000] PID hash table entries: 2048 (order: 1, 8192 bytes)
    [0.000000] Dentry cache hash table entries: 65536 (order: 6, 262144 bytes)
    [0.000000] Inode-cache hash table entries: 32768 (order: 5, 131072 bytes)
    [0.000000] Memory: 240MB 256MB = 496MB total
    [0.000000] Memory: 489856k/489856k available, 34432k reserved, 0K highmem
    [0.000000] Virtual kernel memory layout:
    [0.000000]     vector  : 0xffff0000 - 0xffff1000   (   4 kB)
    [0.000000]     fixmap  : 0xfff00000 - 0xfffe0000   ( 896 kB)
    [0.000000]     vmalloc : 0xe0800000 - 0xfd000000   ( 456 MB)
    [0.000000]     lowmem  : 0xc0000000 - 0xe0000000   ( 512 MB)
    [0.000000]     pkmap   : 0xbfe00000 - 0xc0000000   (   2 MB)
    [0.000000]     modules : 0xbf000000 - 0xbfe00000   (  14 MB)
    [0.000000]       .text : 0xc0008000 - 0xc042f040   (4253 kB)
    [0.000000]       .init : 0xc0430000 - 0xc0456640   ( 154 kB)
    [0.000000]       .data : 0xc0458000 - 0xc0485dc0   ( 184 kB)
    [0.000000]        .bss : 0xc0485de4 - 0xc049d734   (  95 kB)
    [0.000000] Preemptible hierarchical RCU implementation.
    [0.000000] 	Verbose stalled-CPUs detection is disabled.
    [0.000000] NR_IRQS:128
    [0.000000] xlnx,ps7-ttc-1.00.a #0 at 0xe0800000, irq=43
    [0.000000] Console: colour dummy device 80x30
    [0.000000] Calibrating delay loop... 1594.16 BogoMIPS (lpj=7970816)
    [0.090000] pid_max: default: 32768 minimum: 301
    [0.090000] Mount-cache hash table entries: 512
    [0.090000] CPU: Testing write buffer coherency: ok
    [0.090000] CPU0: thread -1, cpu 0, socket 0, mpidr 80000000
    [0.100000] smp_twd: clock not found: -2
    [0.100000] Calibrating local timer... 399.37MHz.
    [0.170000] hw perfevents: enabled with ARMv7 Cortex-A9 PMU driver, 7 counters available
    [0.170000] Setting up static identity map for 0x2f8d48 - 0x2f8d7c
    [0.270000] CPU1: Booted secondary processor
    [0.310000] CPU1: thread -1, cpu 1, socket 0, mpidr 80000001
    [0.310000] Brought up 2 CPUs
    [0.310000] SMP: Total of 2 processors activated (3188.32 BogoMIPS).
    [0.320000] devtmpfs: initialized
    [0.320000] ------------[ cut here ]------------
    [0.320000] WARNING: at arch/arm/mm/dma-mapping.c:198 consistent_init+0x70/0x104()
    [0.330000] Modules linked in:
    [0.330000] [<c0012920>] (unwind_backtrace+0x0/0xe0) from [<c001e924>] (warn_slowpath_common+0x4c/0x64)
    [0.340000] [<c001e924>] (warn_slowpath_common+0x4c/0x64) from [<c001e954>] (warn_slowpath_null+0x18/0x1c)
    [0.350000] [<c001e954>] (warn_slowpath_null+0x18/0x1c) from [<c04345a8>] (consistent_init+0x70/0x104)
    [0.360000] [<c04345a8>] (consistent_init+0x70/0x104) from [<c000858c>] (do_one_initcall+0x90/0x160)
    [0.360000] [<c000858c>] (do_one_initcall+0x90/0x160) from [<c043085c>] (kernel_init+0x84/0x128)
    [0.370000] [<c043085c>] (kernel_init+0x84/0x128) from [<c000dfcc>] (kernel_thread_exit+0x0/0x8)
    [0.380000] ---[ end trace 1b75b31a2719ed1c ]---
    [0.380000] ------------[ cut here ]------------
    [0.390000] WARNING: at arch/arm/mm/dma-mapping.c:198 consistent_init+0x70/0x104()
    [0.390000] Modules linked in:
    [0.390000] [<c0012920>] (unwind_backtrace+0x0/0xe0) from [<c001e924>] (warn_slowpath_common+0x4c/0x64)
    [0.400000] [<c001e924>] (warn_slowpath_common+0x4c/0x64) from [<c001e954>] (warn_slowpath_null+0x18/0x1c)
    [0.410000] [<c001e954>] (warn_slowpath_null+0x18/0x1c) from [<c04345a8>] (consistent_init+0x70/0x104)
    [0.420000] [<c04345a8>] (consistent_init+0x70/0x104) from [<c000858c>] (do_one_initcall+0x90/0x160)
    [0.430000] [<c000858c>] (do_one_initcall+0x90/0x160) from [<c043085c>] (kernel_init+0x84/0x128)
    [0.430000] [<c043085c>] (kernel_init+0x84/0x128) from [<c000dfcc>] (kernel_thread_exit+0x0/0x8)
    [0.440000] ---[ end trace 1b75b31a2719ed1d ]---
    [0.440000] NET: Registered protocol family 16
    [0.460000] L310 cache controller enabled
    [0.460000] l2x0: 8 ways, CACHE_ID 0x410000c8, AUX_CTRL 0x72060000, Cache size: 524288 B
    [0.460000] registering platform device 'pl330' id 0
    [0.470000] registering platform device 'arm-pmu' id 0
    [0.470000] 
    [0.470000] ###############################################
    [0.480000] #                                             #
    [0.480000] #                Board ZED Init               #
    [0.480000] #                                             #
    [0.490000] ###############################################
    [0.490000] 
    [0.500000] hw-breakpoint: found 5 (+1 reserved) breakpoint and 1 watchpoint registers.
    [0.500000] hw-breakpoint: maximum watchpoint size is 4 bytes.
    [0.530000] xslcr xslcr.0: at 0xF8000000 mapped to 0xE0808000
    [0.540000] bio: create slab <bio-0> at 0
    [0.540000] gpiochip_add: registered GPIOs 0 to 245 on device: xgpiops
    [0.540000] xgpiops e000a000.gpio: gpio at 0xe000a000 mapped to 0xe080a000
    [0.830000] xusbps-ehci xusbps-ehci.0: USB 2.0 started, EHCI 1.00
    [0.830000] hub 1-0:1.0: USB hub found
    [0.830000] hub 1-0:1.0: 1 port detected
    [0.840000] Initializing USB Mass Storage driver...
    [0.840000] usbcore: registered new interface driver usb-storage
    [0.850000] USB Mass Storage support registered.
    [0.850000] Xilinx PS USB Device Controller driver (Apr 01, 2011)
    [0.860000] mousedev: PS/2 mouse device common for all mice
    [0.860000] Linux video capture interface: v2.00
    [0.870000] gspca_main: v2.14.0 registered
    [0.870000] usbcore: registered new interface driver uvcvideo
    [0.870000] USB Video Class driver (1.1.1)
    [0.880000] WDT OF probe
    [0.880000] xwdtps f8005000.swdt: Xilinx Watchdog Timer at 0xe081c000 with timeout 10 seconds
    [0.890000] sdhci: Secure Digital Host Controller Interface driver
    [0.890000] sdhci: Copyright(c) Pierre Ossman
    [0.900000] sdhci-pltfm: SDHCI platform and OF driver helper
    [0.900000] mmc0: Invalid maximum block size, assuming 512 bytes
    [0.910000] mmc0: SDHCI controller on e0100000.sdhci [e0100000.sdhci] using ADMA
    [0.910000] usbcore: registered new interface driver usbhid
    [0.920000] usbhid: USB HID core driver
    [0.920000] No connectors reported connected with modes
    [0.930000] adv7511 0-0039: Failed to add route DAI IN->TMDS
    [0.940000] [drm] Cannot find any crtc or sizes - going 1024x768
    [0.940000] drivers/gpu/drm/analog/analog_drm_fbdev.c:analog_drm_fbdev_probe[241]
    [0.960000] asoc: adv7511 <-> 75c00000.axi-spdif-tx mapping ok
    [0.970000] axi-spdif 75c00000.axi-spdif-tx: Failed to set DAI format: -22
    [0.970000] Console: switching to colour frame buffer device 128x48
    [0.980000] ALSA device list:
    [0.980000]   #0: HDMI monitor
    [0.980000] TCP cubic registered
    [0.980000] NET: Registered protocol family 17
    [0.980000] VFP support v0.3: implementor 41 architecture 3 part 30 variant 9 rev 4
    [0.980000] Registering SWP/SWPB emulation handler
    [0.980000] registered taskstats version 1
    [0.980000] drivers/rtc/hctosys.c: unable to open rtc device (rtc0)
    [1.000000] mmc0: new high speed SDHC card at address b368
    [1.000000] mmcblk0: mmc0:b368 SMI   3.69 GiB 
    [1.030000]  mmcblk0: p1
    [1.030000] fb0:  frame buffer device
    [1.030000] drm: registered panic notifier
    [1.030000] [drm] Initialized analog_drm 1.0.0 20110530 on minor 0
    [1.050000] RAMDISK: gzip image found at block 0
    [1.310000] EXT4-fs (ram0): warning: mounting unchecked fs, running e2fsck is recommended
    [1.310000] EXT4-fs (ram0): mounted filesystem without journal. Opts: (null)
    [1.320000] VFS: Mounted root (ext4 filesystem) on device 1:0.
    [1.320000] Freeing init memory: 152K
    Starting rcS...
    ++ Mounting filesystem
    ++ Setting up mdev
    ++ Configure static IP 192.168.1.10
    [1.510000] GEM: lp->tx_bd ffdfb000 lp->tx_bd_dma 18fd7000 lp->tx_skb d8ac17c0
    [1.520000] GEM: lp->rx_bd ffdfc000 lp->rx_bd_dma 18fd8000 lp->rx_skb d8ac18c0
    [1.520000] GEM: MAC 0x00350a00, 0x00002201, 00:0a:35:00:01:22
    [1.530000] GEM: phydev d8b87400, phydev->phy_id 0x1410dd1, phydev->addr 0x0
    [1.530000] eth0, phy_addr 0x0, phy_id 0x01410dd1
    [1.540000] eth0, attach [Marvell 88E1510] phy driver
    ++ Starting telnet daemon
    ++ Starting http daemon
    ++ Starting ftp daemon
    ++ Starting dropbear (ssh) daemon
    ++ Starting OLED Display
    [1.580000] pmodoled-gpio-spi [zed_oled] SPI Probing
    ++ Exporting LEDs & SWs
    rcS Complete
    zynq> 
    zynq> [5.540000] eth0: link up (1000/FULL)
    zynq> ls
    bin/        lib/        lost+found/ proc/       sys/        var/
    dev/        licenses/   mnt/        root/       tmp/
    etc/        linuxrc@    opt/        sbin/       usr/
    mzynq> 

### Loading software via FTP ###

Put the executable from zedboard directory onto the zedboard with ftp as
follows:

    linux % ftp 192.168.1.10
    Connected to 192.168.1.10.
    220 Operation successful
    230 Operation successful
    Remote system type is UNIX.
    Using binary mode to transfer files.
    binary
    200 Operation successful
    prompt
    Interactive mode off.
    hash
    Hash mark printing on (1024 bytes/hash mark).
    pwd
    Remote directory: /
    ftp> bin
    200 Operation successful
    ftp> hash
    Hash mark printing off.
    ftp> hash
    Hash mark printing on (1024 bytes/hash mark).
    ftp> put software.zed
    local: software.zed remote: software.zed
    229 EPSV ok (|||57413|)
    150 Ok to send data
    ###################################################################
    ###################################################################
    ###################################################################
    ###################################################################
    ###################################################################
    ###################################################################
    ###################################################################
    ###################################################################
    ###################################################################
    ###################################################################
    ###################################################################
    ####################################
    226 Operation successful
    797917 bytes sent in 00:00 (1.94 MiB/s)
    ftp> quit
    221 Operation successful
    linux %

### Safe Power off ###

Be sure to poweroff cleanly:

    zynq> poweroff
    zynq> Starting rcK...
      ++ Stopping OLED Display
    [  542.820000] pmodoled-gpio-spi [zed_oled] spi_remove: Device Removed
      ++ Unmounting filesystem
    rcK Complete
    The system is going down NOW!
    Sent SIGTERM to all processes
    Sent SIGKILL to all processes
    Requesting system poweroff
    [  544.940000] System halted.
