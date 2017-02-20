import socket
import struct

# open socket
socket_handler = socket.socket(socket.AF_INET6,socket.SOCK_DGRAM)
socket_handler.bind(('',2002))

while True:
    
    # wait for a request
    request,dist_addr = socket_handler.recvfrom(1024)
    
    hisAddress     = dist_addr[0]
    hisPort        = dist_addr[1]
    
    useShared = struct.unpack('<h',request[-17:-15])[0]
    useTx  = struct.unpack('<h',request[-15:-13])[0]
    useRx  = struct.unpack('<h',request[-13:-11])[0]
    emptyCells  = struct.unpack('<h',request[-11:-9])[0]
    queueuse  = struct.unpack('<h',request[-9:-7])[0]
    
    #humidity = struct.unpack('<H',request[-7:-2])    
    #light = struct.unpack('<H',request[-7:-2])
    #temp = struct.unpack('<H',request[-7:-2])
    asn  = struct.unpack('<HHB',request[-7:-2])
    counter  = struct.unpack('<h',request[-2:])[0]
    
    print 'received "counter={0}, queueuse={3}, emptyCells={4}, useRx={5}, useTx={6}, useShared={7}" from [{1}]:{2}'.format(counter,hisAddress,hisPort,queueuse,emptyCells,useRx,useTx,useShared)

