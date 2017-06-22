import sys
import time
import socket

request    = "poipoipoipoi"
myAddress  = '' #means 'all'
myPort     = 21569
hisAddress = ''
hisPort    = 3
monitor_interval  = 0
slotDuration      = 15 # miliseconds

if len(sys.argv) == 2:
    # log
    output         = []
    output        += ['Argument missing !!!']
    output        += ['Command Usage : python umonitor.py <node_ip> <probe_interval in sec>']
    output         = '\n'.join(output)
    print output 
else:   
    hisAddress = (sys.argv[1]) 
    monitor_interval = (sys.argv[2]) 
    
    print "Starting monitoring delay application..."
      
    # log
    output         = []
    output        += ['[{0}]:{1}->[{2}]:{3}'.format(myAddress,myPort,hisAddress,hisPort)]
    output        += ['\nMonitoring node IP        : {0} '.format(hisAddress)]
    output        += ['Probe interval            : {0} sec '.format(monitor_interval)]
    output        += ['Slot duration             : {0} miliseconds '.format(slotDuration)]
    output         = '\n'.join(output)
    print output
    print "\n"     

    while 1 :
  
        # open socket
        socket_handler = socket.socket(socket.AF_INET6,socket.SOCK_DGRAM)
        socket_handler.settimeout(5)
        socket_handler.bind((myAddress,myPort))
        
        # send request
        socket_handler.sendto(request,(hisAddress,hisPort))
        startTime      = time.time()
        
        # wait for reply
        try:
            reply,dist_addr = socket_handler.recvfrom(1024)
            time.sleep(float(monitor_interval))
        except socket.timeout:        
            # log
            print "\nData not received"
            
        else:         
            n = map(ord,reply)
            time_elapsed =	n[1] << 8 | n[0]
            time_elapsed_ms = time_elapsed * slotDuration            
            time_left = n[3] << 8 | n[2]
            time_left_ms = time_left * slotDuration
                        
            
            if(time_elapsed != 0):
                output         = []
                output        += ['Delay Experienced : {:<3} ({:<5} ms)\tTime Left : {:<3} ({:<5} ms)'.format(time_elapsed,time_elapsed_ms,time_left,time_left_ms)]
                output         = '\t'.join(output)
                print output       
        
    # close socket
    socket_handler.close()

    output     = []
    output    += ['\nDone...']
    output     = '\n'.join(output)
    print output

    raw_input("\nPress return to close this window...")
