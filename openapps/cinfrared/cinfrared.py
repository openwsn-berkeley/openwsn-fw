from coap import coap

MOTE_IP = 'bbbb:0:0:0:12:4b00:60d:9f66'
UDP_PORT = 61618  # can't be the port used in OV

c = coap.coap(udpPort=UDP_PORT)

# infrared turnoff
p = c.PUT('coap://[{0}]/ir'.format(MOTE_IP), payload=[1])

while True:
    user = input("Done. Press q to close. ")
    if user == 'q':
        print('bye bye.')
        c.close()
        break
