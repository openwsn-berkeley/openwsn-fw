import sys
import os
if __name__=='__main__':
    here = sys.path[0]
    #sys.path.insert(0, os.path.join(here, 'build', 'lib.win32-2.7'))# contains openwsn module
    sys.path.insert(0, os.path.join(here, 'build', 'python_gcc', 'bsp'))# contains openwsn module

try:

    import openwsn

    def callback():
       print "callback called"

    # create instance
    mote = openwsn.OpenMote()
    print str(mote)
    
    # install some callbacks
    for i in range(20):
       mote.set_callback(i,callback)
    
    # call other methods
    print mote.bsp_timer_isr()
    #print mote.radio_isr_startFrame()
    #print mote.radio_isr_endFrame()
    print mote.radiotimer_isr_compare()
    print mote.radiotimer_isr_overflow()
    
    raw_input('OK')

except Exception as err:
    raw_input('ERROR: {0}'.format(err))