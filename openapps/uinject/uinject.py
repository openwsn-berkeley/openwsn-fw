import socket
import struct

# open socket
socket_handler = socket.socket(socket.AF_INET6,socket.SOCK_DGRAM)
socket_handler.bind(('',2000))

while True:
    
    # wait for a request
    request,dist_addr = socket_handler.recvfrom(1024)
    
    hisAddress     = dist_addr[0]
    hisPort        = dist_addr[1]
    
    asn  = struct.unpack('<HHB',request[:5])
    counter  = struct.unpack('<h',request[-2:])[0]
    
    print 'received "{0}" from [{1}]:{2}'.format(counter,hisAddress,hisPort)

