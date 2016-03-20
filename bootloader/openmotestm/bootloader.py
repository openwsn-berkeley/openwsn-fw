import sys
import serial
import time

class CmdException(Exception):
    pass

class CommandInterface(object):

    extended_erase = 0
    debugging = True

    def open(self, aport, abaudrate) :
        self.sp = serial.Serial(
            port=aport,
            baudrate=abaudrate,     # baudrate
            bytesize=8,             # number of databits
            parity=serial.PARITY_EVEN,
            stopbits=1,
            xonxoff=0,              # don't enable software flow control
            rtscts=0,               # don't enable RTS/CTS flow control
            timeout=5               # set a timeout value, None for waiting forever
        )
        
    def mdebug(self, message):
        if self.debugging:
            print >> sys.stderr , message
    
    def quiet(self):
        self.debugging = False

    def _wait_for_ask(self, info = ""):
        # wait for ask
        try:
            ask = ord(self.sp.read())
        except IOError:
            raise CmdException("Can't read port or timeout")
        else:
            if ask == 0x79:
                # ACK
                return 1
            else:
                if ask == 0x1F:
                    # NACK
                    raise CmdException("NACK "+info)
                else:
                    # Unknown responce
                    raise CmdException("Unknown response. "+info+": "+hex(ask))


    def reset(self):
        # set reset pin low first
        self.sp.setDTR(1)
        time.sleep(0.1)
        # set reset pin high to restart
        self.sp.setDTR(0)
        time.sleep(0.5)

    def initChip(self):
        # Set boot
        self.sp.setRTS(0)
        self.reset()

        self.sp.write("\x7F")       # Syncro
        return self._wait_for_ask("Syncro")

    def releaseChip(self):
        self.sp.setRTS(1)
        self.reset()

    def cmdGeneric(self, cmd):
        self.sp.write(chr(cmd))
        self.sp.write(chr(cmd ^ 0xFF)) # Control byte
        return self._wait_for_ask(hex(cmd))

    def cmdGet(self):
        if self.cmdGeneric(0x00):
            self.mdebug( "*** Get command");
            length = ord(self.sp.read())
            version = ord(self.sp.read())
            self.mdebug( "    Bootloader version: "+hex(version))
            dat = map(lambda c: hex(ord(c)), self.sp.read(length))
            if '0x44' in dat:
                self.extended_erase = 1
            self.mdebug( "    Available commands: "+", ".join(dat))
            self._wait_for_ask("0x00 end")
            return version
        else:
            raise CmdException("Get (0x00) failed")

    def cmdGetVersion(self):
        if self.cmdGeneric(0x01):
            self.mdebug( "*** GetVersion command")
            version = ord(self.sp.read())
            self.sp.read(2)
            self._wait_for_ask("0x01 end")
            self.mdebug( "    Bootloader version: "+hex(version))
            return version
        else:
            raise CmdException("GetVersion (0x01) failed")

    def cmdGetID(self):
        if self.cmdGeneric(0x02):
            self.mdebug( "*** GetID command")
            length = ord(self.sp.read())
            identity = self.sp.read(length+1)
            self._wait_for_ask("0x02 end")
            return reduce(lambda x, y: x*0x100+y, map(ord, identity))
        else:
            raise CmdException("GetID (0x02) failed")


    def _encode_addr(self, addr):
        byte3 = (addr >> 0) & 0xFF
        byte2 = (addr >> 8) & 0xFF
        byte1 = (addr >> 16) & 0xFF
        byte0 = (addr >> 24) & 0xFF
        crc = byte0 ^ byte1 ^ byte2 ^ byte3
        return (chr(byte0) + chr(byte1) + chr(byte2) + chr(byte3) + chr(crc))


    def cmdReadMemory(self, addr, lng):
        assert(lng <= 256)
        if self.cmdGeneric(0x11):
            # self.mdebug( "*** ReadMemory command")
            self.sp.write(self._encode_addr(addr))
            self._wait_for_ask("0x11 address failed")
            N = (lng - 1) & 0xFF
            crc = N ^ 0xFF
            self.sp.write(chr(N) + chr(crc))
            self._wait_for_ask("0x11 length failed")
            return map(lambda c: ord(c), self.sp.read(lng))
        else:
            raise CmdException("ReadMemory (0x11) failed")


    def cmdGo(self, addr):
        if self.cmdGeneric(0x21):
            self.mdebug( "*** Go command")
            self.sp.write(self._encode_addr(addr))
            self._wait_for_ask("0x21 go failed")
        else:
            raise CmdException("Go (0x21) failed")


    def cmdWriteMemory(self, addr, data):
        assert(len(data) <= 256)
        if self.cmdGeneric(0x31):
            # self.mdebug( "*** Write memory command")
            self.sp.write(self._encode_addr(addr))
            self._wait_for_ask("0x31 address failed")
            #map(lambda c: hex(ord(c)), data)
            lng = (len(data)-1) & 0xFF
            # self.mdebug( "    %s bytes to write" % [lng+1]);
            self.sp.write(chr(lng)) # len really
            crc = 0xFF
            for c in data:
                crc = crc ^ c
                self.sp.write(chr(c))
            self.sp.write(chr(crc))
            self._wait_for_ask("0x31 programming failed")
            # self.mdebug( "    Write memory done")
        else:
            raise CmdException("Write memory (0x31) failed")


    def cmdEraseMemory(self, sectors = None):
        if self.extended_erase:
            return self.cmdExtendedEraseMemory()

        if self.cmdGeneric(0x43):
            # self.mdebug( "*** Erase memory command")
            if sectors is None:
                # Global erase
                self.sp.write(chr(0xFF))
                self.sp.write(chr(0x00))
            else:
                # Sectors erase
                # see page 21-24 of AN3155 document at
                # http://www.st.com/st-web-ui/static/active/en/resource/technical/document/application_note/CD00264342.pdf
                self.sp.write(chr((len(sectors)-1) & 0xFF))
                # checksum: xor(N,N+1 bytes)
                crc = len(sectors)-1
                for c in sectors:
                    crc = crc ^ c
                    self.sp.write(chr(c))
                self.sp.write(chr(crc))
            self._wait_for_ask("0x43 erasing failed")
            self.mdebug( "Erase memory done.")
        else:
            raise CmdException("Erase memory (0x43) failed")

    def cmdExtendedEraseMemory(self):
        if self.cmdGeneric(0x44):
            self.mdebug( "*** Extended Erase memory command")
            # Global mass erase
            self.sp.write(chr(0xFF))
            self.sp.write(chr(0xFF))
            # Checksum
            self.sp.write(chr(0x00))
            tmp = self.sp.timeout
            self.sp.timeout = 30
            print "Extended erase (0x44), this can take ten seconds or more"
            self._wait_for_ask("0x44 erasing failed")
            self.sp.timeout = tmp
            self.mdebug( "    Extended Erase memory done")
        else:
            raise CmdException("Extended Erase memory (0x44) failed")

    def cmdWriteProtect(self, sectors):
        if self.cmdGeneric(0x63):
            self.mdebug( "*** Write protect command")
            self.sp.write(chr((len(sectors)-1) & 0xFF))
            crc = 0xFF
            for c in sectors:
                crc = crc ^ c
                self.sp.write(chr(c))
            self.sp.write(chr(crc))
            self._wait_for_ask("0x63 write protect failed")
            self.mdebug( "    Write protect done")
        else:
            raise CmdException("Write Protect memory (0x63) failed")

    def cmdWriteUnprotect(self):
        if self.cmdGeneric(0x73):
            self.mdebug( "*** Write Unprotect command")
            self._wait_for_ask("0x73 write unprotect failed")
            self._wait_for_ask("0x73 write unprotect 2 failed")
            self.mdebug( "    Write Unprotect done")
        else:
            raise CmdException("Write Unprotect (0x73) failed")

    def cmdReadoutProtect(self):
        if self.cmdGeneric(0x82):
            self.mdebug( "*** Readout protect command")
            self._wait_for_ask("0x82 readout protect failed")
            self._wait_for_ask("0x82 readout protect 2 failed")
            self.mdebug( "    Read protect done")
        else:
            raise CmdException("Readout protect (0x82) failed")

    def cmdReadoutUnprotect(self):
        if self.cmdGeneric(0x92):
            self.mdebug( "*** Readout Unprotect command")
            self._wait_for_ask("0x92 readout unprotect failed")
            self._wait_for_ask("0x92 readout unprotect 2 failed")
            self.mdebug( "    Read Unprotect done")
        else:
            raise CmdException("Readout unprotect (0x92) failed")


# Complex commands section

    def readMemory(self, addr, lng):
        data = []
        
        while lng > 256:
            sys.stdout.write("Read {1} bytes at 0x{0:x}\r".format(addr, 256))
            sys.stdout.flush()
            data = data + self.cmdReadMemory(addr, 256)
            addr = addr + 256
            lng = lng - 256
        sys.stdout.write("Read {1} bytes at 0x{0:x}\n".format(addr, 256))
        data = data + self.cmdReadMemory(addr, lng)
        return data

    def writeMemory(self, addr, data):
        lng = len(data)
        
        offs = 0
        while lng > 256:
            sys.stdout.write("Write {1} bytes at 0x{0:x}\r".format(addr, 256))
            sys.stdout.flush()
            self.cmdWriteMemory(addr, data[offs:offs+256])
            offs = offs + 256
            addr = addr + 256
            lng = lng - 256
        sys.stdout.write("Write {1} bytes at 0x{0:x}\n".format(addr, 256))
        self.cmdWriteMemory(addr, data[offs:offs+lng] + ([0xFF] * (256-lng)) )

    def __init__(self) :
        pass