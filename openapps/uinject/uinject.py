import socket
import struct

import time

import matplotlib.pyplot as plt
import numpy as np

# ================= definitions ===============================================

NUM_CELLS_ELAPSED       = 64
NUM_CELLS_USAGE_HIGH    = 48
NUM_CELLS_USAGE_LOW     = 16

SLOTDURATION            = 0.02 # second

EXP_DURATION            = 3 # in hour
mote_counter_asn_cellusage = {}

# get data from file?

fromFile = raw_input("get data from file? (yes, no):")

if fromFile == 'yes':

    with open('mote_counter_asn_cellusage.log','r') as f:
        line_list = f.readlines()
        mote_counter_asn_cellusage = eval(line_list[-1])
else:
    
    # get the network start time
    raw_input("Press Enter at the same time when network starts...")
    network_start_time = time.time()
    print "Network starts at {0}\n".format(time.ctime(network_start_time))
    
    with open('network_start_time.txt','a') as f:
        f.write(str(network_start_time))

    # open socket
    socket_handler = socket.socket(socket.AF_INET6,socket.SOCK_DGRAM)
    socket_handler.bind(('',61617))

    previous_time = time.time()

    while True:
        
        # wait for a request
        request,dist_addr = socket_handler.recvfrom(1024)
        
        hisAddress     = dist_addr[0]
        hisPort        = dist_addr[1]
        
        numCellsUsed = ord(request[-15])
        asnBytes     = struct.unpack('<HHB',request[-14:-9])
        counterBytes = struct.unpack('<h',request[-9:-7])
        
        asn         = asnBytes[2] * 65536 * 65536 + asnBytes[1] * 65536 + asnBytes[0]
        counter     = counterBytes[0]
        
        # record data to dictionary
        mote = hisAddress.split(':')[-1]
        
        if mote not in mote_counter_asn_cellusage.keys():
            mote_counter_asn_cellusage[mote] = {}
            mote_counter_asn_cellusage[mote]['counter']      = []
            mote_counter_asn_cellusage[mote]['arriveTime']   = []
            mote_counter_asn_cellusage[mote]['asnBytes']     = []
            mote_counter_asn_cellusage[mote]['latency']      = []
            mote_counter_asn_cellusage[mote]['numCellsUsed'] = []
        
        mote_counter_asn_cellusage[mote]['counter'].append(counter)
        mote_counter_asn_cellusage[mote]['arriveTime'].append(time.time())
        mote_counter_asn_cellusage[mote]['asnBytes'].append(asnBytes)
        mote_counter_asn_cellusage[mote]['latency'].append(time.time()-asn*SLOTDURATION-network_start_time)
        mote_counter_asn_cellusage[mote]['numCellsUsed'].append(numCellsUsed)
        
        print 'received "{0}" at asn {1} from [{2}]:{3}, numCellsUsed={4}'.format(counter, asnBytes, hisAddress,hisPort,numCellsUsed)
        
        # record data every 10 minutes
        if time.time()-previous_time>600:
            previous_time = time.time()
            with open('mote_counter_asn_cellusage.log','a') as f:
                f.write(str(mote_counter_asn_cellusage)+'\n')
                
        # stop listening udp packet when running for EXP_DURATION hours
        if (time.time()-network_start_time)>(3600*EXP_DURATION):
            break
            
    # ==== record to file

    with open('mote_counter_asn_cellusage.log','a') as f:
        f.write(str(mote_counter_asn_cellusage)+'\n')
        
# ==== generate list to plot 
'''
target list: 
    num_node_list, 
    node_list_label, 
    node_e2e_reliability, 
    node_e2e_avg_latency, 
    node_avg_cell_usage, 
    node_avg_cell_usage_HIGH, 
    node_avg_cell_usage_LOW
'''

num_node_list            = []
node_list_label          = []
node_e2e_reliability     = []
node_e2e_avg_latency     = []
node_avg_cell_usage      = []
node_avg_cell_usage_HIGH = []
node_avg_cell_usage_LOW  = []

num_node = 0
for mote,data in mote_counter_asn_cellusage.items():
    
    num_node += 1
    num_node_list.append(num_node)
    
    node_list_label.append(mote)
    
    note_pdr = float(len(set(data['counter'])))/float(data['counter'][-1]-data['counter'][0]+1)
    node_e2e_reliability.append(note_pdr)
    
    note_latency = sum(data['latency'])/len(data['latency'])
    node_e2e_avg_latency.append(note_latency)
    
    note_cell_usage  = float(sum(data['numCellsUsed'])/len(data['numCellsUsed']))
    note_cell_usage /= float(NUM_CELLS_ELAPSED)
    node_avg_cell_usage.append(note_cell_usage)
    
    node_avg_cell_usage_HIGH.append(NUM_CELLS_USAGE_HIGH/float(NUM_CELLS_ELAPSED))
    node_avg_cell_usage_LOW.append(NUM_CELLS_USAGE_LOW/float(NUM_CELLS_ELAPSED))
    
# ==== generate figures

# pdr
fig, ax = plt.subplots()
ax.bar(num_node_list,node_e2e_reliability)
# ax.set_xlabel('nodes')
ax.set_xticks(num_node_list)
ax.set_xticklabels(node_list_label, rotation=90)
ax.set_ylabel('end-to-end reliability')
plt.savefig('e2e_reliability.png')
plt.clf()

# latency
fig, ax = plt.subplots()
ax.bar(num_node_list,node_e2e_avg_latency)
# ax.set_xlabel('nodes')
ax.set_xticks(num_node_list)
ax.set_xticklabels(node_list_label, rotation=90)
ax.set_ylabel('end-to-end latency')
plt.savefig('e2e_latency.png')
plt.clf()

# cell Usage
fig, ax = plt.subplots()

ax.plot(num_node_list,node_avg_cell_usage_HIGH,'r-')
plt.text(len(num_node_list)/2, 0.77, 'LIM_NUMCELLSUSED_HIGH', color='red')
ax.plot(num_node_list,node_avg_cell_usage_LOW,'r-')
plt.text(len(num_node_list)/2, 0.20, 'LIM_NUMCELLSUSED_LOW', color='red')
ax.plot(num_node_list,node_avg_cell_usage,'b-^')
# ax.set_xlabel('nodes')
ax.set_xticks(num_node_list)
ax.set_xticklabels(node_list_label, rotation=90)
ax.set_ylabel('node cell usage')
ax.set_ylim([0,1])
plt.savefig('cell_usage.png')
plt.clf()
