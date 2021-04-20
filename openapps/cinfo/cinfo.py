from coap import coap

MOTE_IP = 'bbbb::1415:92cc:0:2'
UDP_PORT = 61618  # can't be the port used in OV

c = coap.coap(udpPort=UDP_PORT)

# read the information about the board status
p = c.GET('coap://[{0}]/i'.format(MOTE_IP))
print(''.join([chr(b) for b in p]))

while True:
    recv_in = input("Done. Press q to close. ")
    if recv_in == 'q':
        print('bye bye.')
        c.close()
        break
