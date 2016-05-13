#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import struct
import socket
import sched, time
from threading import Timer
import calendar
import OpenHdlc as h

try:
   import serial
except ImportError:
   pass


class ConnectivityCoordinator():

    def __init__(self):
        self.hdlc = h.OpenHdlc()
        self.lastRxByte  = self.hdlc.HDLC_FLAG
        self.inputBuf    = ''
        self.busyReceiving = False

        self.f = open('connecitivity'+str(calendar.timegm(time.gmtime())), 'w')

        banner  = []
        banner += [""]
        banner += [" ___                 _ _ _  ___  _ _ "]
        banner += ["| . | ___  ___ ._ _ | | | |/ __>| \ |"]
        banner += ["| | || . \/ ._>| ' || | | |\__ \|   |"]
        banner += ["`___'|  _/\___.|_|_||__/_/ <___/|_\_|"]
        banner += ["     |_|                  openwsn.org"]
        banner += [""]
        banner  = '\n'.join(banner)
        print 'Creating Connectivity Coordinator\n'
        print banner


    def startWorking(self):
        self.mote = None
        time_read = calendar.timegm(time.gmtime())
        print time_read
        #connect to the serial port

        #============================ configuration and connection ===================================

        self.mote_connect(serialport="/dev/ttyUSB0", baudrate='115200')

        self.startPeriodicEpochTransmission()
        self.startReceiving()

    def mote_connect(self, serialport = None, baudrate = '115200'):
        try:
            self.mote = serial.Serial(serialport, baudrate)

        except Exception as err:
            print "{0}".format(err)
            raw_input('Press Enter to close.')
            sys.exit(1)

#============================ read ============================================

    def startReceiving(self):
        while True:
            # read byte from serial
            byte  = self.mote.read(1)

            # Check start of frame
            if ((not self.busyReceiving) and
                (self.lastRxByte == self.hdlc.HDLC_FLAG) and
                (byte != self.hdlc.HDLC_FLAG)):
            
                self.busyReceiving = True
                self.inputBuf      = self.hdlc.HDLC_FLAG
                self.inputBuf     += byte

            # Check middle of frame
            elif ((self.busyReceiving) and 
                  (byte != self.hdlc.HDLC_FLAG)):
                self.inputBuf += byte

            # Check end of frame
            elif ((self.busyReceiving) and
                  (byte == self.hdlc.HDLC_FLAG)):
                self.busyReceiving = False
                self.inputBuf += byte
                tempBuf = self.inputBuf
                print ",".join(hex(ord(c)) for c in tempBuf)
                try:
                    self.inputBuf = self.hdlc.dehdlcify(self.inputBuf)
                except h.HdlcException as err:
                    print err
                else:
                    if (len(self.inputBuf) == 18):
                        (type,addr0,addr1,addr2,addr3,addr4,addr5,addr6,addr7,seqNum,wraps,tsrecieved,rssi,lqi) = struct.unpack('>BBBBBBBBBHBIBB',self.inputBuf)
                        #TODO print the buffer into a file
                        print type,addr0,addr1,addr2,addr3,addr4,addr5,addr6,addr7,seqNum,wraps,tsrecieved,rssi,lqi
                        self.f.write("{0},{1}:{2}:{3}:{4}:{5}:{6}:{7}:{8},{9},{10},{11},{12},{13}\n".format(type,addr0,addr1,addr2,addr3,addr4,addr5,addr6,addr7,seqNum,wraps,tsrecieved,rssi,lqi))

                    else:
                        print "wrong length {0}".format(len(self.inputBuf))

            self.lastRxByte = byte

    def startPeriodicEpochTransmission(self):

        #self.s = sched.scheduler(time.time, time.sleep)
        #self.s.enter(10, 1, self.writeEpoch, ())
        Timer(1, self.writeEpoch, ()).start()

    def writeEpoch(self):
        time_read = calendar.timegm(time.gmtime())
        
        data = [0xEE] + [(time_read >> (8 * i)) & 0xFF for i in range(3,-1,-1)]
        frame = ''.join([chr(b) for b in data])
        

        output = self.hdlc.hdlcify(frame)

        #write it unsigned int *4bytes* big endian >
        self.mote.write(output);

        #self.s.enter(10, 1, self.writeEpoch, ())
        Timer(1, self.writeEpoch, ()).start()


#============================ main ============================================



#============================ main ============================================

if __name__=="__main__":
    conn = ConnectivityCoordinator()
    conn.startWorking()
