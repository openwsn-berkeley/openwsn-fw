import sys
import os
if __name__=='__main__':
    here = sys.path[0]
    sys.path.insert(0, os.path.join(here, 'firmware','openos','projects','common'))# contains openwsn module

try:

    import oos_openwsn

    def callback(notifId):
       print "callback {0} called".format(notifId)
       raw_input("press Enter for next")

    # create instance
    mote = oos_openwsn.OpenMote()
    print str(mote)
    
    # install some callbacks
    for i in range(81):
       temp_lambda = lambda notifId=i:callback(notifId)
       mote.set_callback(i,temp_lambda)
    
    # call other methods
    mote.supply_on()

except Exception as err:
    raw_input('ERROR: {0}'.format(err))