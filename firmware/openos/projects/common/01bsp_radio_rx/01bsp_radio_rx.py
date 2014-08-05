import sys
import serial
import struct
import socket

banner  = []
banner += [""]
banner += [" ___                 _ _ _  ___  _ _ "]
banner += ["| . | ___  ___ ._ _ | | | |/ __>| \ |"]
banner += ["| | || . \/ ._>| ' || | | |\__ \|   |"]
banner += ["`___'|  _/\___.|_|_||__/_/ <___/|_\_|"]
banner += ["     |_|                  openwsn.org"]
banner += [""]
banner  = '\n'.join(banner)
print banner

DEFAULT_IOTLAB     = True
DEFAULT_MOTENAME   = 'wsn430-35'
DEFAULT_SERIALPORT = 'COM10'

#============================ configuration ===================================

iotlab        = None
motename      = None
serialport    = None

# iotlab

t = raw_input('running IoT-lAB? (Y|N): '.format(DEFAULT_IOTLAB))
if   not t.strip():
    iotlab         = DEFAULT_IOTLAB
elif t.strip() in ['1','yes','y','Y']:
    iotlab         = True
else:
    iotlab         = False

# motename

if iotlab:
    t = raw_input('motename? (e.g. {0}): '.format(DEFAULT_MOTENAME))
    if   not t.strip():
        motename   = DEFAULT_MOTENAME
    else:
        motename   = t.strip()

# serialport

if not iotlab:
    t = raw_input('name of serial port (e.g. {0}): '.format(DEFAULT_SERIALPORT))
    if   not t.strip():
        serialport = DEFAULT_SERIALPORT
    else:
        serialport = t.strip()

#============================ connect =========================================

if iotlab:
    assert motename
    try:
        mote = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
        mote.connect((motename,20000))
    except Exception as err:
        print 'could not connect to {0}, reason: {1}'.format(motename,err)
        raw_input('Press Enter to close.')
        sys.exit(1)
else:
    assert serialport
    try:
        mote = serial.Serial(serialport,115200)
    except Exception as err:
        print 'could not open {0}, reason: {1}'.format(serialport,err)
        raw_input('Press Enter to close.')
        sys.exit(1)

#============================ read ============================================

rawFrame = []

while True:
    
    if iotlab:
        bytes = mote.recv(1024)
        rawFrame += [ord(b) for b in bytes]
    else:
        byte  = mote.read(1)
        rawFrame += [ord(byte)]
    
    if rawFrame[-3:]==[0xff]*3 and len(rawFrame)>=8:
        
        (rxpk_len,rxpk_num,rxpk_rssi,rxpk_lqi,rxpk_crc) = \
            struct.unpack('>BBbBB', ''.join([chr(b) for b in rawFrame[-8:-3]]))
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
