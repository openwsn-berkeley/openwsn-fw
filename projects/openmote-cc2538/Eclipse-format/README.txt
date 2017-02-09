OpenMote-CC2538 Eclipse project: howto
--------------------------------------


Software required:
---------------------

- GNU ARM Gcc toolchain - https://launchpad.net/gcc-arm-embedded
- Eclipse CDT Kepler - http://www.eclipse.org/downloads/packages/eclipse-ide-cc-developers/keplersr2
- SEGGER jlink software - https://www.segger.com/jlink-software.html


Eclipse J-Link Debugging plug-in intallation:
---------------------------------

- Run Eclipse
- Choose your favourite workspace
- Close the initial Welcome screen
- Go to Help -> Install New Software
- Add into "Work with:" the following repository: "GNU ARM Eclipse Plug-ins - http://gnuarmeclipse.sourceforge.net/updates"
- Under "GNU ARM C/C++ Cross Development Tools" select GNU ARM C/C++ J-Link Debugging
- Press Finish and go ahead accepting the installation


Configuring the Eclipse environment:
-----------------------------------

- Go to Window -> Preferences -> C/C++ -> Build -> Environment and add a new environment variable SCONS, whose value is "scons" for UNIX/Linux/MACOS users or "scons.bat" for Windows users
- Go to Window -> Preferences -> Run/Debug -> String Substitution and set the jlink_path variable to the path of the SEGGER J-Link installation folder
- For Windows users, it is suggested to embed the J-Link Server into Eclipse: go to Window -> Preferences -> Run/Debug -> String Substitution and set jlink_gdbserver to JLinkGDBServerCL


Importing OpenMote-CC2538 project:
---------------------------------

- Go into ${openwsn-fw}/projects/OpenMote-CC2538 and create a copy of the Eclipse-format folder
- Rename the folder to your favourite name, e.g., OPENWSNECLIPSE
- Into Eclipse, right-click in the Project Explorer and click Import...
- Select General -> Existing Projects into Workspace and press Next
- Select root directory and Browse to ${openwsn-fw}/projects/OpenMote-CC2538/OPENWSNECLIPSE and press Finish


Building and Debugging:
----------------------

- On the right window go to the "Make Target" tab and compile a target, e.g., oos_openwsn
- You can check that building is ok into the bottom window on the Console tab
- Go to Run -> Debug Configurations -> GDB SEGGER J-Link Debugging and go into the sub-configuration "OpenWSN Debug"
- In the Main tab go to C/C++ Application and press "Search Project": you will see the executable you want to flash to the mote, select it and press Debug
- You can now flash the code on your mote and debug the code