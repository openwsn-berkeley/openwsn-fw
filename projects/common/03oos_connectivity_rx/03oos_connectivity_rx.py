#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import struct
import socket
import sched, time
from threading import Timer
import calendar
import OpenHdlc

try:
   import serial
except ImportError:
   pass


class ConnectivityCoordinator():
    hdlc = OpenHdlc()

    s = sched.scheduler(time.time, time.sleep)

    def __init__(self):
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
        mote = None
        time_read = calendar.timegm(time.gmtime())
        print time_read
        #connect to the serial port

        #============================ configuration and connection ===================================

        #mote = self.mote_connect(serialport="\dev\ttyUSB0", baudrate='115200')

        self.startPeriodicEpochTransmission()
        #self.startReceiving(self, mote):

    def mote_connect(self, motename=None , serialport= None, baudrate='115200'):
        try:
            if (motename):
                mote = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
                mote.connect((motename,20000))
            else:
                mote = serial.Serial(serialport, baudrate)
            return mote
        except Exception as err:
            print "{0}".format(err)
            raw_input('Press Enter to close.')
            sys.exit(1)

#============================ read ============================================

    def startReceiving(self, mote):
        rawFrame = []

        while True:

            byte  = mote.read(1)
            rawFrame += [ord(byte)]

            if rawFrame[-3:]==[0xff]*3 and len(rawFrame)>=8:

                (rxpk_len,rxpk_num,rxpk_rssi,rxpk_lqi,rxpk_crc) = struct.unpack('>BBbBB', ''.join([chr(b) for b in rawFrame[-8:-3]]))

                print 'len={0:<3} num={1:<3} rssi={2:<4} lqi={3:<3} crc={4}'.format(
                    rxpk_len,
                    rxpk_num,
                    rxpk_rssi,
                    rxpk_lqi,
                    rxpk_crc
                )

                if rxpk_len>127:
                    print "ERROR: frame too long.\a"

                rawFrame = []


    def startPeriodicEpochTransmission(self):

        #self.s = sched.scheduler(time.time, time.sleep)
        #self.s.enter(10, 1, self.writeEpoch, ())
        Timer(1, self.writeEpoch, ()).start()

    def writeEpoch(self):

        time_read = calendar.timegm(time.gmtime())
        print time_read


        #self.s.enter(10, 1, self.writeEpoch, ())
        Timer(1, self.writeEpoch, ()).start()


#============================ main ============================================



#============================ main ============================================

if __name__=="__main__":
    conn = Connectivity()
    conn.startWorking()
