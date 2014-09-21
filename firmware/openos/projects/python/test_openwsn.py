import sys
import os
if __name__=='__main__':
    here = sys.path[0]
    sys.path.insert(0, os.path.join(here, '..','common'))# contains openwsn module

import re

#============================ get notification IDs ============================

f = open(os.path.join('..','..','bsp','boards','python','openwsnmodule_obj.h'))
lines = f.readlines()
f.close()

notifString = []

for line in lines:
    m = re.search('MOTE_NOTIF_(\w+)',line)
    if m:
        if m.group(1) not in notifString:
            notifString += [m.group(1)]

def notifId(s):
    assert s in notifString
    return notifString.index(s)

import oos_openwsn

def default_callback(id):
   print "P: {0}() (callback {1})".format(notifString[id],id)
   raw_input("press Enter for next")

def eui64_get():
   print "P: eui64_get()"
   return range(8)

def bsp_timer_scheduleIn(delay):
   print "P: bsp_timer_scheduleIn({0})".format(delay)
   
# create instance
mote = oos_openwsn.OpenMote()
print str(mote)

# install default callback
for i in range(len(notifString)-1):
    temp_lambda = lambda id=i:default_callback(id)
    mote.set_callback(i,temp_lambda)

# overwrite some callbacks
mote.set_callback(notifId('eui64_get'),           eui64_get)
mote.set_callback(notifId('bsp_timer_scheduleIn'),bsp_timer_scheduleIn)

# start the mote
mote.supply_on()

#print mote.getState()
