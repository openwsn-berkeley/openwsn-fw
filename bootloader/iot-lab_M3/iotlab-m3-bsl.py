#!/usr/bin/python
import sys
import getopt
import os.path
from subprocess import call

currentDir = os.path.dirname(os.path.realpath(__file__))

OOCD_ITF=os.path.join(currentDir, 'iotlab-m3.cfg')

if os.name=='nt':
   extension = '.exe'
else:
   extension = ''

def usage():
    print("""Usage: %s [-i binary file] [-p port] 
Examples:
    ./%s example/main -p /dev/ttyUSB0 
    """ % (sys.argv[0], sys.argv[0]))

# from http://stackoverflow.com/questions/377017/test-if-executable-exists-in-python
def which(program):
    def is_exe(fpath):
        return os.path.isfile(fpath) and os.access(fpath, os.X_OK)

    fpath, fname = os.path.split(program)
    if fpath:
        if is_exe(program):
            return program
    else:
        for path in os.environ["PATH"].split(os.pathsep):
            path = path.strip('"')
            exe_file = os.path.join(path, program)
            if is_exe(exe_file):
                return exe_file

    raise RuntimeError('Command not found.')

try:
   OPENOCD = which('openocd' + extension)
except RuntimeError as e:
    print('ERROR: {} requires OpenOCD with ftdi interface.'.format(sys.argv[0]))
    print('')
    sys.exit(1)

try:
   opts, args = getopt.getopt(sys.argv[1:], "hi:p:", ["help", "input=","port="])
except getopt.GetoptError as err:
   # print help information and exit:
   print str(err) # will print something like "option -a not recognized"
   usage()
   sys.exit(2)
binary = None
port = '0'
for o, a in opts:
   if o in ("-h", "--help"):
      usage()
      sys.exit()
   elif o in ("-i", "--input"):
      binary = a
   elif o in ("-p", "--port"):
      port = a
   else:
      assert False, "unhandled option" 

try: 
   os.path.isfile(binary)
except:
   print('ERROR: Binary file not found/specified.')
   usage()
   sys.exit(2)

if os.name=='nt':
   binary = '{' + binary + '}'

call([OPENOCD,
   '-f', os.path.join(currentDir, OOCD_ITF),
   '-c', 'tcl_port 0',
   '-c', 'telnet_port 0',
   '-c', 'gdb_port 0',
   '-c', 'init',
   '-c', 'targets',
   '-c', 'reset halt', 
   '-c', 'flash write_image erase ' + binary,
   '-c', 'reset halt',
   '-c', 'verify_image ' + binary,
   '-c', 'reset run',
   '-c', 'shutdown'])
