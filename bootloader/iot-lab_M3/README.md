-------------------------------------------------------------------------------
License and Acknowledgment:
-------------------------------------------------------------------------------
BSL script 'iotlab-m3-bsl.py' for iot-lab_M3 nodes is a Python version of 
'doFlash.sh' script from 
[https://github.com/adjih/exp-iotlab/blob/master/tools/misc/doFlash.sh] with
additions regarding the openocd handling. 'doFlash.sh' script is itself an 
adaptation of code from 
[https://github.com/iot-lab/openlab/tree/master/platform].

OpenOCD config file 'iotlab-m3.cfg' is a copy of
[https://github.com/iot-lab/openlab/blob/master/platform/scripts/iotlab-m3.cfg]
, and the same license and copyright applies.

We are grateful to Cedric Adjih and the Iot-Lab team, in particular
Roger Pissard-Gibollet and Gaetan Harter, for their help and for making this
code available.

-------------------------------------------------------------------------------
Windows environment config in order to program (i.e. flash) iot-lab_M3.
-------------------------------------------------------------------------------

1. Download openocd Windows binary package from http://openocd.org/

2. Extract to C:\openocd or similar

3. Add to PATH the openocd directory containing binaries (C:\openocd\bin for 32
bit or C:\openocd\bin-x64 for 64 bit machines).  

4. Rename openocd executable from openocd-xxx-x.x.x.exe to openocd.exe. OpenWSN
python script expects openocd.exe to be found in the PATH. Test by issuing 
openocd.exe in a cmd prompt.

5. Connect the iot-lab_M3 node to the USB port.

6. Download Zadig from http://zadig.akeo.ie/ which we will use to install 
libusb-win32 drivers that openocd requires.

7. Open the Zadig executable. Click Options -> List All Devices

8. Two entries corresponding to the same iot-lab_M3 node should appear: 
1) M3 (Interface 0); 2) M3 (Interface 1). For interfacing with openocd, one of 
the two ports must be associated with the OpenOCD usb-win32_ft2232 driver 
available in the OpenOCD installation [1]. We will use M3 (Interface 0). 
Select M3 (Interface 0) from the drop down list in Zadig. Select 
libusb-win32 (xxx) driver and click on the button to replace/install the 
driver for this interface. Once this is done successfully, openocd in Windows 
can interact with IoT-lab_M3 node and flash it. 

9. To flash, use the OpenWSN workflow in openwsn-fw/ dir:
scons board=iot-lab_M3 toolchain=armgcc bootload=0

NOTE: openocd does not support flashing using typical /dev/ttyUSB* or COMX 
interfaces. The Python script will flash the first *iot-lab_M3* node 
connected to the PC, independent of the bootload argument value.  


[1] https://github.com/hikob/openlab/wiki/Installation-Notes-for-Windows-Users]

-------------------------------------------------------------------------------
Linux:
-------------------------------------------------------------------------------
1. install openocd by issuing 'sudo apt-get install openocd'
2. To flash, use the OpenWSN workflow as in step 9 of the Windows manual.

