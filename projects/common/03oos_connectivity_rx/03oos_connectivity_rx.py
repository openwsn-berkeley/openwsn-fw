#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import getopt
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

    def __init__(self,argv):

        self.serialPort ="/dev/ttyUSB0"
        self.outputfile = 'connectivity'

        self.parseParams(argv)

        self.hdlc = h.OpenHdlc()
        self.lastRxByte  = self.hdlc.HDLC_FLAG
        self.inputBuf    = ''
        self.busyReceiving = False


        port = self.serialPort.split("/")
        print port[2]
        print "****************"
        self.f = open(self.outputfile+str(calendar.timegm(time.gmtime()))+port[2]+".csv", 'w')

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

    def parseParams(self,args):
         try:
             opts, args = getopt.getopt(args,"p:f:",["serial=","ofile="])
         except getopt.GetoptError as err:
             print err
             print 'Wrong params. Use: 03oos_connectivity_rx.py -p <serial_port> -f <outputfile>'
             sys.exit(2)
         for opt, arg in opts:
             if opt == '-h':
                print '03oos_connectivity_rx.py -p <serial_port> -f <outputfile>'
                sys.exit()
             elif opt in ("-p", "--serial"):
                self.serialPort = arg
             elif opt in ("-f", "--ofile"):
                self.outputfile = arg
         print 'Serial port is "', self.serialPort
         print 'Output file is "', self.outputfile

    def startWorking(self):
        self.mote = None
        time_read = calendar.timegm(time.gmtime())
        print time_read
        #connect to the serial port

        #============================ configuration and connection ===================================

        self.mote_connect(serialport=self.serialPort, baudrate='115200')

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
                        #0xaa,0x0,0x12,0x4b,0x0,0x6,0xd:0x98,0x13,0x2,0xf9,0x0,0x0,0x0,0x0,0x0,0xe6,0x6c,0x65,0xec,0x7e

                        (type,addr0,addr1,addr2,addr3,addr4,addr5,addr6,addr7,seqNum,wraps,tsrecieved,rssi,lqi) = struct.unpack('>BBBBBBBBBHBIbB',self.inputBuf)
                        #TODO print the buffer into a file
                        print hex(type),hex(addr0),hex(addr1),hex(addr2),hex(addr3),hex(addr4),hex(addr5),hex(addr6),hex(addr7),seqNum,wraps,tsrecieved,rssi,lqi
                        self.f.write("{0},{1}:{2}:{3}:{4}:{5}:{6}:{7}:{8},{9},{10},{11},{12},{13}\n".format(hex(type),hex(addr0),hex(addr1),hex(addr2),hex(addr3),hex(addr4),hex(addr5),hex(addr6),hex(addr7),seqNum,wraps,tsrecieved,rssi,lqi))
                        self.f.flush()
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
    conn = ConnectivityCoordinator(sys.argv[1:])
    conn.startWorking()
