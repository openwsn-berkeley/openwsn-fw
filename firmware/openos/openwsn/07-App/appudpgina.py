import socket
import binascii
import time
import os

num_measurements = str(chr(0x64))  # (100)d
myAddress        = ''    # means 'any suitable interface'
myPort           = 2158
hisAddress       = '2001:470:1f05:dff:1415:9209:22b:51'
hisPort          = 2190

print "Testing appudpgina..."

socket_handler = socket.socket(socket.AF_INET6,socket.SOCK_DGRAM)
socket_handler.settimeout(5)
socket_handler.bind((myAddress,myPort))
socket_handler.sendto(num_measurements,(hisAddress,hisPort))
print "\nrequest "+myAddress+"%"+str(myPort)+" -> "+hisAddress+"%"+str(hisPort)
print num_measurements+" ("+str(len(num_measurements))+" bytes)"

time_first = time.time()
replycounter = 0
while (1):
   try:
      reply,dist_addr = socket_handler.recvfrom(1024)
   except socket.timeout:
      print "\nno further replies, it seems\n"
      break
   else:
      time_last = time.time()
      os.system("CLS")
      print "\nreply "+str(replycounter)+": "+str(dist_addr[0])+"%"+str(dist_addr[1])+" -> "+myAddress+"%"+str(myPort)+" ("+str(len(reply))+" bytes)\n"
      replycounter += 1

      print "sensitive_accel X   : "+str(ord(reply[0]))
      print "                Y   : "+binascii.hexlify(reply[ 2: 4])
      print "                Z1  : "+binascii.hexlify(reply[ 4: 6])
      print "                Z3  : "+binascii.hexlify(reply[ 6: 8])+"\n"

      print "temperature         : "+binascii.hexlify(reply[ 8:10])+"\n"

      print "magnetometer X      : "+binascii.hexlify(reply[10:12])
      print "             Y      : "+binascii.hexlify(reply[12:14])
      print "             Z      : "+binascii.hexlify(reply[14:16])+"\n"

      print "large_range_accel X : "+binascii.hexlify(reply[16:18])
      print "                  Y : "+binascii.hexlify(reply[18:20])
      print "                  Z : "+binascii.hexlify(reply[20:22])+"\n"

      print "gyro_temperature    : "+binascii.hexlify(reply[22:24])
      print "gyro X              : "+binascii.hexlify(reply[24:26])
      print "     Y              : "+binascii.hexlify(reply[26:28])
      print "     Z              : "+binascii.hexlify(reply[28:30])

socket_handler.close()

try:
   print str(replycounter)+" replies in "+str(time_last-time_first)+"s (one every "+str((time_last-time_first)/(replycounter-1))+"s)"
except:
   pass

raw_input("\nPress return to close this window...")
