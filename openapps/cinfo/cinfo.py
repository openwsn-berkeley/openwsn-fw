import os
import sys
here = sys.path[0]
print here
sys.path.insert(0,os.path.join(here,'..','..','..','coap'))

from coap import coap
import signal

MOTE_IP = 'bbbb::1415:92cc:0:2'

c = coap.coap()

# read the information about the board status
p = c.GET('coap://[{0}]/i'.format(MOTE_IP))
print ''.join([chr(b) for b in p])


while True:
        input = raw_input("Done. Press q to close. ")
        if input=='q':
            print 'bye bye.'
            #c.close()
            os.kill(os.getpid(), signal.SIGTERM)
