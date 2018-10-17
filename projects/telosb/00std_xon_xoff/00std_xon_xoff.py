import serial
import threading
import time

#============================ defines =========================================

SERIALPORT = 'COM4'
BURSTSIZE  = 200

#============================ classes =========================================

class SerialRxThread(threading.Thread):
    
    XOFF = 0x13
    XON  = 0x11
    
    def __init__(self,serialport=None):
        
        # store params
        self.serialport           = serialport
        
        # local variables
        self.lastRxByte           = None
        self.serial = serial.Serial(
            port                  = self.serialport,
            baudrate              = 115200,
            xonxoff               = True,
        )
        
        # start thread
        threading.Thread.__init__(self)
        self.name                 = 'SerialRxThread'
        self.start()
    
    def run(self):
        try:
            while True:
                rxByte = ord(self.serial.read(1))
                
                #print '0x{0:02x}'.format(rxByte)
                if self.lastRxByte!=None:
                    expectedRxByte = self._expectedRxByte(self.lastRxByte)
                    if rxByte!=expectedRxByte:
                        print 'got 0x{0:02x}, expected 0x{1:02x}'.format(rxByte,expectedRxByte)
                        #raw_input('')
                self.lastRxByte = rxByte
        
        except Exception as err:
            print '{0}: {1}'.format(
                self.name,
                err,
            )
    
    def _expectedRxByte(self,lastRxByte):
        expectedRxByte = (lastRxByte+1) & 0xff
        if expectedRxByte in [self.XON,self.XOFF]:
            expectedRxByte += 1
        if expectedRxByte>BURSTSIZE:
            expectedRxByte = 0
        return expectedRxByte

class SerialTxThread(threading.Thread):
    
    # 9600 chars/sec @50% DC -> 4800 chars per second -> 480 chars per 100ms
    NUMBURSTBYTES      = 500
    INTERBURSTDURATION = 0.020
    
    def __init__(self,serial):
        
        # store params
        self.serial          = serial
        
        # local variables
        self.txByte          = 0xff
        
        # start thread
        threading.Thread.__init__(self)
        self.name                 = 'SerialTxThread'
        self.start()
    
    def run(self):
        try:
            while True:
                time.sleep(self.INTERBURSTDURATION)
                
                for _ in range(self.NUMBURSTBYTES):
                    self.txByte = (self.txByte+1)&0xff
                    self.serial.write(chr(self.txByte))
        
        except Exception as err:
            print '{0}: {1}'.format(
                self.name,
                err,
            )
    
#============================ main ============================================

def main():
    serialRxThread = SerialRxThread(SERIALPORT)
    serialTxThread = SerialTxThread(serialRxThread.serial)

if __name__=="__main__":
    main()
