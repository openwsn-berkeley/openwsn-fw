from coap import coap

MOTE_IP = 'bbbb::1415:92cc:0:2'
UDP_PORT = 61618  # can't be the port used in OV

c = coap.coap(udpPort=UDP_PORT)

# read status of debug LED
p = c.GET('coap://[{0}]/l'.format(MOTE_IP))
print(chr(p[0]))

# toggle debug LED
_ = c.PUT('coap://[{0}]/l'.format(MOTE_IP), payload=[ord('2')])

# read status of debug LED
p = c.GET('coap://[{0}]/l'.format(MOTE_IP))
print(chr(p[0]))

input("Done. Press enter to close.")
