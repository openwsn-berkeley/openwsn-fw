OpenMote-CC2538 Eclipse project: howto
--------------------------------------


Software required:
-----------------

- MSP-430 GCC (mspgcc) toolchain - http://sourceforge.net/projects/mspgcc/files/Windows/mingw32/
   -in ubuntu (apt-get install gcc-msp430)
- mspdebug executable installed in the mspgcc folder (including usb and fet libraries, e.g., MSP430.dll and HIL.dll under Windows)
   -in ubuntu (apt-get install mspdebug)    
- Eclipse CDT Kepler - http://www.eclipse.org/downloads/packages/eclipse-standard-432/keplersr2


Eclipse J-Link Debugging plug-in intallation:
--------------------------------------------

- Run Eclipse
- Choose your favourite workspace
- Close the initial Welcome screen
- Go to Help -> Install New Software
- Select "Kepler - http://download.eclipse.org/releases/kepler" into "Work with:"
- Under "Mobile and Device Management" select C/C++ GDB Hardware Debugging
- Press Next repeatedly and finally Finish, accepting the installation. 


Configuring the Eclipse environment:
-----------------------------------

- Go to Window -> Preferences -> C/C++ -> Build -> Environment and add a new environment variable SCONS, whose value is "scons" for UNIX/Linux/MACOS users or "scons.bat" for Windows users (without quotes)
- Go to Window -> Preferences -> Run/Debug -> String Substitution and set the mspdebug_path variable to the path of the folder where your mspdebug tool is installed (into the toolchain bin installation folder)
- go to Window -> Preferences -> Run/Debug -> String Substitution and set the mspdebug_gdbserver to "mspdebug.exe" (for Windows users) or "mspdebug" (for UNIX/Linux users). Do not put quotes.


Importing GINA or other msp430 projects:
---------------------------------

- Go into ${openwsn-fw}/projects/gina and create a copy of the Eclipse-format folder
- Rename the folder to your favourite name, e.g., OPENWSNECLIPSE
- Into Eclipse, right-click in the Project Explorer and click Import...
- Select General -> Existing Projects into Workspace and press Next
- Select root directory and Browse to ${openwsn-fw}/projects/gina/OPENWSNECLIPSE and press Finish


Building, cleaning, flashing:
----------------------------

- On the right window go to the "Make Target" tab
- Targets without specifications in parenthesis are intended for building only the targets (e.g., bsp_leds, oos_openwsn)
- Targets with the specification (clean) are intended to clean all the object files and clear the executable
- Targets with the specification (flash over fet) are intended to build and flash an optimized code on the motes through the MSP-FET430UIF debugger. TO DEBUG YOU SHOULD NOT USE THIS OPTION
- Targets with the specification (debug) are intended to build a non optimized code that can be debugged (as discussed later). IT DOES NOT FLASH MOTES.


Debugging:
---------

- After compiling a target with the (debug) option, it is possible to debug the code.
- Go to Run -> Debug Configurations -> GDB Hardware Debugging and go into the sub-configuration "mspdebug-gdbclient"
- In the Main tab go to C/C++ Application and press "Search Project": you will see the executable you want to flash to the mote, select it and press Apply. DO NOT PRESS DEBUG, SINCE THE GDB SERVER HAS NOT BEEN STARTED.
- Go to Run -> Debug Configurations -> Launch Group and go into the sub-configuration "OpenWSN Debug"
- You can now flash the code on your mote and debug it