import socket
import struct

# open socket
socket_handler = socket.socket(socket.AF_INET6,socket.SOCK_DGRAM)
socket_handler.bind(('',25000))

while True:
    
    # wait for a request
    request,dist_addr = socket_handler.recvfrom(1280)
    
    hisAddress = dist_addr[0]
    hisPort    = dist_addr[1]
    counter    = len(request)
    good       = 0 
    for i in range(counter):
        if i % 10 == ord(request[i]):
            good += 1
    
    print 'received {0} of {1} from [{2}]:{3}'.format(good,counter,hisAddress,hisPort)
