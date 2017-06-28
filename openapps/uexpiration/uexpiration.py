import sys
import time
import socket
import datetime

myAddress  = '' #means 'all'
myPort     = 21568
hisPort    = 5

hisAddress = ''
numPkts = 0
delay = 0

if len(sys.argv) == 6:
    hisAddress = (sys.argv[1])
    pktInterval = (sys.argv[2])
    numPkts = int(sys.argv[3])
    delay = int(sys.argv[4])  
    d_flag =  int(sys.argv[5])  
  
    # Payload
    request    = str(pktInterval)+ ',' +str(numPkts)+','+str(delay)+','+str(d_flag)+','

    # log
    output         = []
    output        += ['Starting Packet Expiration Test App...']
    output        += ['Request deadline info [{0}]:{1}->[{2}]:{3}'.format(myAddress,myPort,hisAddress,hisPort)]
    output        += ['\nSender node IP             : {0} '.format(hisAddress)]
    output        += ['Pkt Interval               : {0} ms '.format(pktInterval)]
    output        += ['Number of pkts             : {0}'.format(numPkts)]
    output        += ['Max permissible pkt delay  : {0} ms'.format(delay)]
    output        += ['Drop flag                  : {0}'.format(d_flag)]
    output         = '\n'.join(output)
    print output
    print "\n"

    print "Starting Deadline Application..."
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
    except socket.timeout:
        # log
        print "\nno packet"    
    else:            
        # Wait for data packets
        var = 1
        while var <= 3 :              
            # wait for reply
            try:
                reply,dist_addr = socket_handler.recvfrom(1024)
            except socket.timeout: 
                var += 1
                print "\nno reply"        
            else:
                payload = map(ord,reply)
                seq_num =	payload[1] << 8 | payload[0]
                now = datetime.datetime.now()
                curr_time = now.strftime("%H:%M:%S")
                output     = []
                output    += ['{0}'.format(curr_time)]
                output    += ['Received data packet [{0}]:{1}->[{2}]:{3}'.format(dist_addr[0],dist_addr[1],myAddress,myPort)]                
                output    += ['Seq no: {0} '.format(seq_num)]
                output     = '\t'.join(output)
                print output
            
    # close socket
    socket_handler.close()
else:
    # log
    output         = []
    output        += ['Argument field missing ...']
    output        += ['Command Usage : python uexpiration.py <dest_ip> <pkt_interval> <no_of_pkts> <delay in ms> <drop_flag>']
    output        += ['<dest_ip> format : bbbb::xxxx:xxxx:xxxx:xxxx']
    output         = '\n'.join(output)
    print output 



raw_input("\nPress return to close this window...")
