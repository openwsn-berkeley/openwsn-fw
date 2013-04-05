import socket

request    = "poipoipoipoi"
myAddress  = '' #means 'all'
myPort     = 21568
hisAddress = '2001:470:48b8:cfde:1415:9200:12:e63b'
hisPort    = 7

print "Testing udpEcho..."

socket_handler = socket.socket(socket.AF_INET6,socket.SOCK_DGRAM)
socket_handler.settimeout(5)
socket_handler.bind((myAddress,myPort))
socket_handler.sendto(request,(hisAddress,hisPort))
print "\nrequest "+myAddress+"%"+str(myPort)+" -> "+hisAddress+"%"+str(hisPort)
print request+" ("+str(len(request))+" bytes)"
try:
   reply,dist_addr = socket_handler.recvfrom(1024)
except socket.timeout:
   print "\nno reply"
else:
   print "\nreply "+str(dist_addr[0])+"%"+str(dist_addr[1])+" -> "+myAddress+"%"+str(myPort)
   print reply+" ("+str(len(reply))+" bytes)"
socket_handler.close()

raw_input("\nPress return to close this window...")
