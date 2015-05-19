import socket
import struct

# open socket
socket_handler = socket.socket(socket.AF_INET6,socket.SOCK_DGRAM)
socket_handler.bind(('',2000))

last_counter = None

while True:
    
    # wait for a request
    request,dist_addr = socket_handler.recvfrom(1024)
    
    hisAddress     = dist_addr[0]
    hisPort        = dist_addr[1]
    counter        = struct.unpack('<h',request)[0]
    
    if last_counter!=None:
        if counter-last_counter!=1:
            print 'MISSING!!'
    last_counter = counter
    
    print 'received "{0}" from [{1}]:{2}'.format(counter,hisAddress,hisPort)
    
    # send back a reply
    socket_handler.sendto('poipoi',(hisAddress,hisPort))
    
    print 'reply sent.'

