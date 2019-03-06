import socket
import struct

# open socket
socket_handler = socket.socket(socket.AF_INET6,socket.SOCK_DGRAM)
socket_handler.bind(('',61617))

while True:
    
    # wait for a request
    request,dist_addr = socket_handler.recvfrom(1024)
    
    hisAddress     = dist_addr[0]
    hisPort        = dist_addr[1]
    
    asn      = struct.unpack('<HHB',request[-14:-9])
    counter  = struct.unpack('<h',request[-9:-7])
    
    print 'received "{0}" at asn {1} from [{2}]:{3}'.format(counter, asn, hisAddress,hisPort)

