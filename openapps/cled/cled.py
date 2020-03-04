import os
import sys
here = sys.path[0]
print here
sys.path.insert(0,os.path.join(here,'..','..','..','..','..','..','coap'))

from coap import coap

MOTE_IP = 'bbbb::1415:92cc:0:2'
UDPPORT = 61618 # can't be the port used in OV

c = coap.coap(udpPort=UDPPORT)

# read status of debug LED
p = c.GET('coap://[{0}]/l'.format(MOTE_IP))
print chr(p[0])

# toggle debug LED
p = c.PUT(
    'coap://[{0}]/l'.format(MOTE_IP),
    payload = [ord('2')],
)

# read status of debug LED
p = c.GET('coap://[{0}]/l'.format(MOTE_IP))
print chr(p[0])

raw_input("Done. Press enter to close.")
