
License and Acknowledgment
--------------------------

The BSL script `iotlab-m3-bsl.py` for iot-lab_M3 nodes is a Python version of the `doFlash.sh` [script](https://github.com/adjih/exp-iotlab/blob/master/tools/misc/doFlash.sh) with additions regarding `openocd` handling. 

The `doFlash.sh` script is itself an adaptation of code from the [IoT-lab team](https://github.com/iot-lab/openlab/tree/master/platform).

The OpenOCD config file `iotlab-m3.cfg` is a copy of [iotlab-m3.cfg](https://github.com/iot-lab/openlab/blob/master/platform/scripts/iotlab-m3.cfg); the same license and copyright applies.

We are grateful to Cedric Adjih and the IoT-lab team, in particular Roger Pissard-Gibollet and Gaetan Harter, for their help and for making this code available.


Program the IoT-lab_M3 in Windows
---------------------------------

1. Download `openocd` Windows binary package from http://openocd.org/
1. Extract to `C:\openocd` (or similar)
1. Add `C:\openocd\bin` (32-bit Windows) or `C:\openocd\bin-x64` (64-bit Windows) to your Windows `PATH` environment variable.
1. Rename the openocd executable from `openocd-xxx-x.x.x.exe` to `openocd.exe`. The OpenWSN bootloader Python script expects `openocd.exe` to be found in the `PATH`. You can test all is OK by typing `openocd.exe` in a Windows command prompt.
1. Positions the switch on the IoT-lab_M3 mote in the position *away* from the USB connector.
1. Connect the IoT-lab_M3 mote to the USB port of your Windows computer.
1. Download Zadig from http://zadig.akeo.ie/. This will use to install the `libusb-win32` drivers openocd requires.
1. Open the Zadig executable. Click `Options` > `List All Devices`.
1. Two entries corresponding to the same `IoT-lab_M3` node should appear: 
    1. `M3 (Interface 0)`
    1. `M3 (Interface 1)`
1. For interfacing with openocd, one of the two ports must be associated with the OpenOCD `usb-win32_ft2232` driver 
available in the OpenOCD installation (see [tutorial](https://github.com/hikob/openlab/wiki/Installation-Notes-for-Windows-Users)). We will use `M3 (Interface 0)`.
1. Select `M3 (Interface 0)` from the drop down list in Zadig. Select `libusb-win32 (xxx)` driver and click on the button to replace/install the driver for this interface. Once this is done successfully, openocd in Windows can interact with IoT-lab_M3 node and flash it.
1. To reprogram the mote, use the usual OpenWSN build scripts. Navigate to the `openwsn-fw/` directory and type `scons board=iot-lab_M3 toolchain=armgcc bootload=0`


NOTE 1: openocd does not support flashing using typical `/dev/ttyUSB*` or `COMX` interfaces. The Python script will flash the first IoT-lab_M3 node connected to the PC, independent from the `bootload=` argument value.  

NOTE 2: As several version of [OpenOCD binary](http://www.freddiechopin.info/en/download/category/4-openocd) were teseted locally, the latest two version of OpenOCD (0.9.0 and 0.8.0) doesn't work with iot-lab_M3. Version 0.6.1 and 0.7.0 works well.

Program the IoT-lab_M3 in Linux
-------------------------------

1. install openocd by issuing 'sudo apt-get install openocd'
2. To flash, use the same OpenWSN workflow as in the Windows instructions above.
