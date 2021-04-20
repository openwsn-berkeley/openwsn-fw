from coap import coap

MOTE_IP = 'bbbb::1415:92cc:0:2'
UDP_PORT = 61618 # can't be the port used in OV

c = coap.coap(udpPort=UDP_PORT)

# read the information about the board status
p = c.GET('coap://[{0}]/.well-known/core'.format(MOTE_IP))
print(''.join([chr(b) for b in p]))

input("Done. Press enter to close.")

c.close()
