[![en](https://img.shields.io/badge/lang-en-green.svg)](https://github.com/Starsznayder/USRP-B210-recorder/README.md)
[![pl](https://img.shields.io/badge/lang-pl-red.svg)](https://github.com/Starsznayder/USRP-B210-recorder/README.pl.md)

# B210 Recorder

Simple signal recorder for USRP B210

**Dependencies**

Requires UHD (tested with version 4.1) and Boost (tested with version 1.71).

**Compilation:**

1. You can use the build.sh script in the project's main directory, but you need to set the installation path:
   `PROJ_PATH="/kitty"`

2. Alternatively, you can use any CMAKE-aware tool. In that case, be sure to set the *KITTY_INSTALL_PATH* variable, which indicates the installation path. Also, ensure that the dependencies for the log library (utu) are met, and it should be placed in *KITTY_INSTALL_PATH/include/utu*.

**Usage**

By default, the program installs in the *KITTY_INSTALL_PATH/bin* directory, and configuration files in *KITTY_INSTALL_PATH/bin/ini*. To run the program:
   `./PK_B210`

It may be necessary to set the path to FPGA images, for example:
   `export UHD_IMAGES_DIR=/kitty/rfnoc/share/uhd/images`

**Configuration**

All essential and configurable parameters are in the configuration file *KITTY_INSTALL_PATH/bin/ini/simpleRecorder.ini*, which has two or more sections.

The first sections (*[USRPX]*) concern device configuration, where X is an arbitrary device identifier [0 ... N]. In these sections, the following fields are available:

*  addr - For B210, set the serial and UHD parameters, e.g., `addr=serial=3231C8B,num_recv_frames=512`. You can obtain the serial through *uhd_find_devices*.
*  GainRX - Rx gain in GNUdB
*  FcRx - carrier frequency
*  FsRx - sample rate
*  B - filter bandwidth
*  writePathY - the write path, where Y is an arbitrary channel identifier. The number of writePath fields additionally defines the number of used channels in this device, e.g., 
  `writePath0=/kitty/rec/CH0; writePath1=/kitty/rec/CH1` indicates 2 receiving channels. Removing writePath1 leaves only one channel. Paths can point to different hard drives if there are issues with write speed.
*  dataPartSize - block size of data to be written to disk. A larger block interrupts the stream less frequently but also consumes more RAM. The write buffer size will be 100 * dataPartSize * the number of receiving channels * the size of the complex<int16_t> sample. If the USRP data stream is interrupted, increase the buffer size, or decrease it for very short recordings.

At the end of the file is the rec section, which contains fields:
*  timestamp - leave null
*  timestamp64 - also, do not change
*  notes - recording notes. It will be saved with the recording files (it can contain a description of what is being recorded). **IMPORTANT** Notes can be added during recording by changing the field in this file and saving it. The program reads the configuration file with a one-second interval and detects any changes. Each subsequent note will appear as a new *.note file in all recording directories.
*  syncSource - synchronization source to choose from **[internal external gpsdo]**
*  octoclockAddr - Octoclock address. If Octoclock is used for USRP synchronization and syncSource is set to gpsdo, the GPS time will be obtained from Octoclock at the specified IP address (USRP does not need to have a GPS module; they will work in external mode).
*  targetStartTime - a value greater than the current Unix time in seconds will start the recording at the specified time - enter the Unix time in seconds. If this is 0, the recording will start when the devices are configured + startOffset.
*  startOffset - delay in starting the stream, giving the device time to apply the configuration.

**Synchronization possibilities**
Each USRP device is represented in the configuration file by its own section [USRPX], where X is the consecutive index of devices starting from 0. The following synchronization methods are considered effective:

*  Octoclock + GPS time from Octoclock:

REF and PPS cables from USRPs are connected to Octoclock. Set the [rec].octoclockAddr field to the IP address of Octoclock (default 192.168.10.3), syncSource=gpsdo.

*  Octoclock + GPS time from USRP:

REF and PPS cables from USRPs are connected to Octoclock. Set the [rec].octoclockAddr field to **none**. GPS time will be obtained from the USRP device specified in the **[USRP0]** section, syncSource=gpsdo.

*  Without GPS time but with Octoclock

REF and PPS cables from USRPs are connected to Octoclock. Set the [rec].octoclockAddr field to **none**, syncSource=external.

*  Without any synchronization

Set the [rec].octoclockAddr field to **none**, syncSource=internal.


**Performance**

Performance was measured on an Intel NUC 8 computer with an NVME Samsung PM981 drive. A stream of 60 MS/device was achieved - the USRP itself did not allow more.

#Usage with WSL (Windows Subsystem for Linux)

**WSL Configuration**

Installing the environment and Ubuntu 20.04:
* Run PowerShell as an administrator

`Enable-WindowsOptionalFeature -Online -FeatureName Microsoft-Windows-Subsystem-Linux`

`wsl --install`

* Reboot

 `wsl.exe --install -d Ubuntu-20.04`

* Enter your username and password
* Reboot

`winget install --interactive --exact dorssel.usbipd-win`

[link1](https://github.com/dorssel/usbipd-win)
[link2](https://woshub.com/share-host-usb-devices-windows-wsl-hyper-v/)

#if you can, turn back

**Ubuntu Configuration**

* Start the WSL console
* Install dependencies

`sudo apt install htop ncdu aptitude libboost1.71-all-dev git python3 python3-pip python3-setuptools python3-dev build-essential cmake`

`sudo apt install python-is-python3`

`sudo apt install linux-tools-5.15.0-87-generic hwdata`

`sudo cp -r /usr/lib/linux-tools/5.15.0-87-generic/usbip /usr/local/bin/usbip`

`sudo cp -r /usr/lib/linux-tools/5.15.0-87-generic/usbipd /usr/local/bin/usbipd`

* Install UHD (RFNoC)
[link](https://kb.ettus.com/Getting_Started_with_RFNoC_Development)

`cd`

`sudo pip install git+https://github.com/gnuradio/pybombs.git`

`pybombs recipes add gr-recipes git+https://github.com/gnuradio/gr-recipes.git`

`pybombs recipes add ettus git+https://github.com/EttusResearch/ettus-pybombs.git`

`pybombs --config makewidth=7 prefix init ~/rfnoc -R rfnoc -a rfnoc`

`cd ~/rfnoc`

`./setup_env.sh`


* Compilation of the Recorder

`cd`

`git clone http://192.168.90.95/Kot/rejestratorka-b210.git`

`cd rejestratorka-b210`

`chmod 777 build.sh`

`mkdir ~/kitty`

`mkdir ~/kitty/rec`

`mkdir ~/kitty/rec/CH0`

`mkdir ~/kitty/rec/CH1`

`./build.sh`

**USB Port Forwarding**
* Run PowerShell as an administrator
* Connect the USB B210
* Install the Windows driver [link](https://files.ettus.com/manual/page_transport.html#transport_usb_installwin)

`usbipd wsl list`

* Find the Ettus device

`usbipd wsl attach --busid <BUSID>`

* On Ubuntu (in the WSL console)

`sudo su`

`cd ~/rfnoc/bin`

`./uhd_find_devices`

If no USRP is found, Windows may have switched BUSID in the meantime. In that case, repeat until the BUSID is stable:

`usbipd wsl list`

`usbipd wsl attach --busid <BUSID>`

* Run

`cd ~/kitty/bin`

`sudo su`

`export LD_LIBRARY_PATH=/home/<username>/rfnoc/lib`

`./PK_B210`

It is important to run as root due to USB port permissions.

**Tips**
* Remember to copy the serial returned by `./uhd_find_devices` to the configuration file /home/<USER>/kitty/bin/ini/simpleRecorder.ini
* Set Real Time priority for processes **vmmem** and **usbipd**
* After starting, do not touch the computer; the stream through usbipd is very unstable. It is best to turn off everything that is not needed and set the computer to max performance mode.
* Stretch the disk, e.g., create a very large file and delete it, or record for a long time with a small FS ~5MS and delete the recording (this trick can work wonders!)