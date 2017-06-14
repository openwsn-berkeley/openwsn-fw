/**
\defgroup uexpiration uexpiration

\brief UDP Expiration application

\author Shalu R <shalur@cdac.in>, June 2017
\author Lijo Thomas <lijo@cdac.in>, June 2017
*/

uexpiration is a simple application to test the implementation of the draft on
packet expiration time for time synchronized multi-hop networks.
Refer : <https://tools.ietf.org/id/draft-lijo-6lo-expiration-time-03.txt>.

uexpiration sends out a udp request to a initiate packet transfers from a
remote node running uexpiration.c program on port 5. The configurable parameters include 
inter-packet intervals, maximum delay, number of packets, and drop flag.

If the drop flag is set to 1, the packet will be dropped in the network if
the packet expiration time is reached. Otherwise, the packet will be forwarded
to the application.

Usage : python uexpiration.py <dest_ip> <pkt_interval> <no_of_pkts>
                              <delay_in_ms> <drop_flag>

Example : The following command initiates a udp flow from bbbb::1415:92cc:0:4
at 1pkt/5 sec intervals for 10 packets with max_delay of 1 sec. The packet will
be dropped on packet expiry.

#python uexpiration.py bbbb::1415:92cc:0:4 5000 10 1000 1.

The expected output is

Starting Packet Expiration Test App...
Request deadline info []:21568->[bbbb::1415:92cc:0:4]:5

Sender node IP             : bbbb::1415:92cc:0:4 
Pkt Interval               : 5000 ms 
Number of pkts             : 10
Max permissible pkt delay  : 1000 ms
Drop flag                  : 1


Starting Deadline Application...
Received data packet [bbbb::1415:92cc:0:4]:5->[]:21568
Received data packet [bbbb::1415:92cc:0:4]:5->[]:21568
...

no reply

...

