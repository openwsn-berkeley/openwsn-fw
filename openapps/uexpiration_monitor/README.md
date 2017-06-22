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

Note : Enable the flag 'DEADLINE_OPTION_ENABLED' in opendefs.h.

Usage : python uexpiration_monitor.py <dest_ip> <probe_interval(sec)>

#python uexpiration_monitor.py bbbb::1415:92cc:0:2 1

Typical output looks like this.

Starting monitoring delay application...
[]:21569->[bbbb::1415:92cc:0:2]:3

Monitoring node IP        : bbbb::1415:92cc:0:2 
Probe interval            : 1 sec 


Delay Experienced : 5   (70    ms)	Time Left : 66  (924   ms)
Delay Experienced : 4   (56    ms)	Time Left : 67  (938   ms)
Delay Experienced : 63  (882   ms)	Time Left : 0   (0     ms)
Delay Experienced : 2   (28    ms)	Time Left : 69  (966   ms)
Delay Experienced : 84  (1176  ms)	Time Left : 0   (0     ms)
Delay Experienced : 1   (14    ms)	Time Left : 70  (980   ms)
Delay Experienced : 6   (84    ms)	Time Left : 65  (910   ms)
Delay Experienced : 11  (154   ms)	Time Left : 60  (840   ms)
Delay Experienced : 38  (532   ms)	Time Left : 33  (462   ms)
Delay Experienced : 8   (112   ms)	Time Left : 14  (196   ms)

...

no reply

...

