import os
import sys
here = sys.path[0]
print here
sys.path.insert(0,os.path.join(here,'..','..','..','coap'))

from coap import coap
import signal

MOTE_IP = 'bbbb:0:0:0:12:4b00:60d:9f66'
UDPPORT = 61618 # can't be the port used in OV

c = coap.coap(udpPort=UDPPORT)

# infrared turnoff
p = c.PUT(
    'coap://[{0}]/ir'.format(MOTE_IP),
    payload = [1],
)

while True:
    input = raw_input("Done. Press q to close. ")
    if input=='q':
        print 'bye bye.'
        #c.close()
        os.kill(os.getpid(), signal.SIGTERM)
