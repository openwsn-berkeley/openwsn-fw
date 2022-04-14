#!/usr/bin/env python
# -*- coding: utf-8 -*-

# COMMAND LINE ARGS SPECIFICATION
# Example format: python 01bsp_radio_rx.py arg1
# arg1 = serial port path.
#   Ex: MacOS: '/dev/tty.usbserial-14147301'
#   Ex: Linux: '/dev/ttyUSB1'
#   Ex: Windows: 'COM1'
# todo add arg2 spec

from __future__ import print_function # use python3 print from python2

import sys
import struct
import socket
import platform
from datetime import datetime
import serial
from colorama import Fore, Back, Style # color support in terminal output
from colorama import init
init()

banner  = []
banner += [""]
banner += [" ___                 _ _ _  ___  _ _ "]
banner += ["| . | ___  ___ ._ _ | | | |/ __>| \ |"]
banner += ["| | || . \/ ._>| ' || | | |\__ \|   |"]
banner += ["`___'|  _/\___.|_|_||__/_/ <___/|_\_|"]
banner += ["     |_|                  openwsn.org"]
banner += [""]
banner  = '\n'.join(banner)
print(banner)

XOFF           = 0x13
XON            = 0x11
XONXOFF_ESCAPE = 0x12
XONXOFF_MASK   = 0x10

MAX_NUM_PACKET = 256 

def mote_connect(motename=None , serialport= None, baudrate='115200'):
    try:
        if (motename):
            mote = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
            mote.connect((motename,20000))
        else:
            mote = serial.Serial(serialport, baudrate)
        return mote
    except Exception as err:
        print(str(err))
        raw_input('Press Enter to close.')
        sys.exit(1)
    

#============================ configuration and connection ===================================

# read in the command line args
all_args = sys.argv[1:] # removes arg that is just the name of this Python file

if len(all_args) != 2:
    print('wrong number of command line args. see the top of this python file for usage specification')
    quit()

mote = mote_connect(serialport=all_args[0])
encode_type = all_args[1]

#============================ read ============================================

CRC_LEN = 2

rawFrame         = []
rawFrame_decoded = []
previousFrame    = 0
frameCounter     = 0
xonxoffEscaping  = False

scum_pkt_size = 20 + CRC_LEN
additional_pkt_info_size = 5 # for rxpk_len,rxpk_rssi,rxpk_lqi,rxpk_crc, rxpk_freq_offset
neg_1_size = 3 # data sent by uart has -1, -1, -1 sent once information is fully sent. This is an end flag.
uart_pkt_size = scum_pkt_size + additional_pkt_info_size + neg_1_size

print("LOOKING FOR PACKETS OF LENGTH (excluding additional 2 bits of CRC):", scum_pkt_size - 2)

while True:
    byte  = mote.read(1)
    rawFrame += [ord(byte)]
    
    if rawFrame[-neg_1_size:]==[0xff]*neg_1_size and len(rawFrame)>=uart_pkt_size:
        rawFrame_decoded = rawFrame

        to_decode = rawFrame_decoded[-uart_pkt_size :-neg_1_size]
        #print(to_decode)

        # packet items 0-21, rxpk_len,rxpk_rssi,rxpk_lqi,rxpk_crc, rxpk_freq_offset
        uart_rx = struct.unpack('>' + 'B' * scum_pkt_size +  'BbBBb', ''.join([chr(b) for b in to_decode]))

        pkt = uart_rx[0:scum_pkt_size]

        #print(to_decode)

        (rxpk_len,rxpk_rssi,rxpk_lqi,rxpk_crc, rxpk_freq_offset) = uart_rx[scum_pkt_size:scum_pkt_size + additional_pkt_info_size]

        if rxpk_len != scum_pkt_size:
            continue

        print(Fore.MAGENTA + datetime.now().strftime("%m/%d/%Y %H:%M:%S.%f")[:-3] + ' ', end='')

        print(Fore.RED + 'len={0:<3} rssi={1:<3} lqi={2:<1} crc={3:<1} freq_offset={4:<4}'.format(
            rxpk_len,
            rxpk_rssi,
            rxpk_lqi,
            rxpk_crc,
            rxpk_freq_offset,
        ), end='')

        print("pkt " + "0-" + str(scum_pkt_size - 1) + ": ", end='')

        if encode_type == 'ASCII':
            print(Fore.GREEN + 'ASCII: ', end='')
            for i in range(scum_pkt_size - CRC_LEN):
                print(chr(pkt[i]), end='')
        elif encode_type == 'RAW':
            print(Fore.GREEN + '   RAW: ', end='')
            for i in range(scum_pkt_size - CRC_LEN):
                print('{0:<3}'.format(pkt[i]) + ' ', end='')

        print(Style.RESET_ALL)
        
#        if rxpk_len>127:
            #print "ERROR: frame too long.\a"
#            a = 0 # just placeholder
#        else:
#            if previousFrame>rxpk_num:
#                output = "frameCounter={0:<3}, PDR={1}%".format(frameCounter, frameCounter*100/MAX_NUM_PACKET)
                #print output
#                frameCounter  = 0
#                with open('log.txt','a') as f:
#                    f.write(output+'\n')

#            frameCounter += 1
#            previousFrame = rxpk_num
        
        rawFrame         = []
        rawFrame_decoded = []
