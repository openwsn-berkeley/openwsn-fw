import time
import socket

NUM_TRIES  = 10

request    = "poipoipoipoi"
myAddress  = '' #means 'all'
myPort     = 21568
hisAddress = 'bbbb::1415:92cc:0:2'
hisPort    = 7
delays     = []
succ       = 0
fail       = 0
print "Testing udpEcho..."

for i in range(NUM_TRIES):
    
    # log
    output         = []
    output        += ['echo {0}'.format(i)]
    output        += ['request [{0}]:{1}->[{2}]:{3}'.format(myAddress,myPort,hisAddress,hisPort)]
    output        += ['{0} ({1} bytes)'.format(request,len(request))]
    output         = '\n'.join(output)
    print output
    
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
        # account
        fail      += 1
        
        # log
        print "\nno reply"
        
    else:
        # account
        succ      += 1
        delay      = time.time()-startTime
        delays    += [delay]
        
        # log
        output     = []
        output    += ['\necho {0}'.format(i)]
        output    += ['request [{0}]:{1}->[{2}]:{3}'.format(dist_addr[0],dist_addr[1],myAddress,myPort)]
        output    += ['{0} ({1} bytes)'.format(reply,len(reply))]
        output    += ['delay: {0:.03f}s'.format(delay)]
        output     = '\n'.join(output)
        print output
    
    # close socket
    socket_handler.close()

output     = []
output    += ['\nstatistics:']
output    += ['- success            {0}'.format(succ)]
output    += ['- fail               {0}'.format(fail)]
output    += ['- min/max/avg delay  {0:.03f}/{1:.03f}/{2:.03f}'.format(
        min(delays),
        max(delays),
        float(sum(delays))/float(len(delays)),
    )
]
output     = '\n'.join(output)
print output

raw_input("\nPress return to close this window...")
