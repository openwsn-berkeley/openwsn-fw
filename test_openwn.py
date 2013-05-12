import sys
import os
if __name__=='__main__':
    here = sys.path[0]
    sys.path.insert(0, os.path.join(here, 'build', 'lib.win32-2.7'))# contains openwsn module

import openwsn

mote = openwsn.OpenMote()
print str(mote)
print mote.bsp_timer_isr()

raw_input('poi')