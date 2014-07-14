== Release Notes ==
R1 (First Release) - 01 Jul. 2014

== Wiki page ==
Please, refer to the official OpenWSN site for more details:
https://openwsn.atlassian.net/secure/Dashboard.jspa


=== What is this module for? ===

This OpenWSN Security Module is an extension of standard OpenWSN-fw module, 
including security features. With this module, each packet sent by a node 
to an other node is encrypted and authenticated with a unique key statically
set in each of the nodes.


=== How to integrate this code with OpenWSN-fw module? ===

Download it from the Repository and use it as the whole project.


==== Build the code and flashing on a mote ======

After having imported it, you can open your C compiler and build the code, typing:

build=telosb --oos_openwsn

Then you can flash the resulting firmware on a mote opening a terminal, gaining
the administrator rights and typing:

python bsl --telosb -c /dev/ttyUSB0 -r -e -I -p 03oos_openwsn_prog.ihex


Enjoy it!
