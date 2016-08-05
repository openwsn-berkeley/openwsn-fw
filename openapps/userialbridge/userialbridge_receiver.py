'''
Run this script on the computer with the Openvisualizer running.
'''

import socket

# open socket
sock = socket.socket(
    socket.AF_INET6,    # IPv6
    socket.SOCK_DGRAM,  # UDP
)

# bind socket
sock.bind(('',2001))

while True:
    # read from socket
    (msgRx,addr) = sock.recvfrom(1024)
    
    # print
    print 'received "{0}" from {1}'.format(
       '-'.join(['%02x'%ord(b) for b in msgRx]),
       addr
    )

