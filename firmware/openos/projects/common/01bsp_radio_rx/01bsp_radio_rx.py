import sys
import serial
import struct

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

DEFAULT_SERIALPORT = 'COM10'

serialport = raw_input('name of serial port (e.g. {0}): '.format(DEFAULT_SERIALPORT))
if not serialport.strip():
    serialport = DEFAULT_SERIALPORT

try:
    s = serial.Serial(serialport,115200)
except Exception as err:
    print 'could not open {0}, reason: {1}'.format(serialport,err)
    raw_input('Press Enter to close.')
    sys.exit(1)

rawFrame = []

while True:
    
    b = ord(s.read(1))
    
    rawFrame += [b]
    
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
