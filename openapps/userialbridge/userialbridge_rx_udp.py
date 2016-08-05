'''
Run this script on the computer with the Openvisualizer running.
'''

import socket
import logging

log = logging.getLogger(__name__)
log.setLevel(logging.DEBUG)
ch = logging.StreamHandler()
ch.setLevel(logging.INFO)
formatter = logging.Formatter('%(asctime)s - %(message)s')
ch.setFormatter(formatter)
log.addHandler(ch)

# open socket
sock = socket.socket(
    socket.AF_INET6,    # IPv6
    socket.SOCK_DGRAM,  # UDP
)

# bind socket
sock.bind(('',3001))

while True:
    # read from socket
    (msgRx,addr) = sock.recvfrom(1024)
    
    # log
    log.info(
        'RX {0}...'.format(
           '-'.join(['%02x'%ord(b) for b in msgRx[:10]])
        )
    )
