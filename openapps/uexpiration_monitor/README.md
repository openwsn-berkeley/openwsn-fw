/**
\defgroup uexpiration uexpiration

\brief UDP Expiration monitor application

\author Shalu R <shalur@cdac.in>, June 2017
\author Lijo Thomas <lijo@cdac.in>, June 2017
*/

uexpiration_monitor is a simple application to monitor the performance of the
network in meeting the packet deadline. The application is based on the
the draft on packet expiration time for time synchronized multi-hop
networks.  
Refer : <https://tools.ietf.org/id/draft-lijo-6lo-expiration-time-03.txt>.

uexpiration_monitor.py is a host side application which sends out a udp request
to a remote 6tisch mote running a monitor application on port 3, uexpiration_monitor.c.
The uexpiration_monitor.c responds by sending back the time information of the
most recent packet that the mote has forwarded. The time information comprises
delay experienced by the packet and the remaining time before its expiry.

Usage : python uexpiration_monitor.py <dest_ip> <probe_interval(sec)>

#python uexpiration_monitor.py bbbb::1415:92cc:0:2 1 

Typical output looks like this.


Starting monitoring delay application...
[]:21569->[bbbb::1415:92cc:0:2]:3

Monitoring node IP        : bbbb::1415:92cc:0:2 
Probe interval            : 1 sec 

Delay Experienced : 19  (266   ms) 		Time Left : 12  (168   ms)
Delay Experienced : 48  (672   ms) 		Time Left : 52  (728   ms)
Delay Experienced : 19  (266   ms) 		Time Left : 12  (168   ms)
Delay Experienced : 13  (182   ms) 		Time Left : 87  (1218  ms)
Delay Experienced : 18  (252   ms) 		Time Left : 82  (1148  ms)
Delay Experienced : 122 (1708  ms) 		Time Left : 0   (0     ms)
Delay Experienced : 17  (238   ms) 		Time Left : 83  (1162  ms)
Delay Experienced : 55  (770   ms) 		Time Left : 45  (630   ms)
...

no reply

...

