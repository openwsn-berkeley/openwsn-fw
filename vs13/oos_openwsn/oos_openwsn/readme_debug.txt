Building oos_openwsn.pyd
============

Run gnu target before compiling the project:

	scons board=python toolchain=gcc oos_openwsn

Open Visual studio project oos_openwsn.sln and build the project.

After building, the oos_openwsn.pyd file will be copied to:

	firmware\openos\projects\common\

Python Debug
============

If you do not want to debug python code, follow these instructions:

http://stackoverflow.com/questions/11713701/how-to-debug-c-extensions-for-python-on-windows

Otherwise, recompile Python and add python27_d.lib to the project.
python27_d.dll will be required as well.

