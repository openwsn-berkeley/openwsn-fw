import socket
import time
from random import randint

import colorama as c

c.init()


def pretty_print(lst):
    lst = list(bytearray(lst))
    pld_str = ''

    pld_str = ''.join([pld_str, '[ '])
    if len(lst) <= 10:
        pld_str = ''.join([pld_str, "".join('0x{:02x} '.format(x) for x in lst)])
    else:
        pld_str = ''.join([pld_str, ''.join('0x{:02x} '.format(lst[i]) for i in range(10))])
        pld_str = ''.join([pld_str, '... '])
    pld_str = ''.join([pld_str, ']'])
    return pld_str


print "\n  UDP Echo Application"
print "------------------------\n"

num_tries = raw_input("> Number of echoes [10]? ")
pld_size = raw_input("> Payload size [50]? ")
address = raw_input("> Destination address [bbbb::1415:92cc:0:2]? ")
timeout = raw_input("> Set timeout value [10]? ")

print '\n Starting ...\n'

if num_tries == '':
    num_tries = 10
else:
    num_tries = int(num_tries)

if pld_size == '':
    pld_size = 50
else:
    pld_size = int(pld_size)

if address == '':
    hisAddress = 'bbbb:0:0:0:1415:92cc:0:2'
else:
    hisAddress = address

if timeout == '':
    timeout = 10
else:
    timeout = int(timeout)

myAddress = ''  # means 'all'
myPort = randint(1024, 65535)
hisPort = 7
delays = []
succ = 0
fail = 0

for i in range(num_tries):

    # generate a new random payload for each echo request
    request = "".join([chr(randint(0, 255)) for j in range(pld_size)])

    # log
    output = []
    output += [c.Back.GREEN + c.Fore.BLACK + 'ECHO REQUEST {0}'.format(i) + c.Style.RESET_ALL]
    output += ['  - Address    [{0}]:{1} --> [{2}]:{3}'.format(myAddress, myPort, hisAddress, hisPort)]
    output += ['  - Payload    {0} ({1} bytes)'.format(pretty_print(request), len(request))]
    output = '\n'.join(output)
    print output

    # open socket
    socket_handler = socket.socket(socket.AF_INET6, socket.SOCK_DGRAM)
    socket_handler.settimeout(timeout)
    socket_handler.bind((myAddress, myPort))

    # send request
    socket_handler.sendto(request, (hisAddress, hisPort))
    startTime = time.time()

    # wait for reply
    try:
        reply, dist_addr = socket_handler.recvfrom(2048)
    except socket.timeout:
        # account
        fail += 1

        # log
        print '\n'
        print c.Back.YELLOW + c.Fore.BLACK + "TIMEOUT" + c.Style.RESET_ALL
        print "\n=========================================\n"

    else:
        if reply == request:
            succ += 1
            delay = time.time() - startTime
            delays += [delay]

            # log
            output = ['\n']
            output += [c.Back.GREEN + c.Fore.BLACK + 'ECHO RESPONSE {0}'.format(i) + c.Style.RESET_ALL]
            output += ['  - Address    [{0}]:{1}->[{2}]:{3}'.format(dist_addr[0], dist_addr[1], myAddress, myPort)]
            output += ['  - Payload    {0} ({1} bytes)'.format(pretty_print(reply), len(reply))]
            output += ['  - Delay      {0:.03f}s'.format(delay)]
            output = '\n'.join(output)

        else:
            fail += 1

            output = ['\n']
            output += [c.Back.RED + c.Fore.BLACK + 'ECHO FAILED {0}'.format(i) + c.Style.RESET_ALL]
            output += ['  - Response does not match request..']
            output += ['  - Payload    {0} ({1} bytes)'.format(pretty_print(reply), len(reply))]
            output = '\n'.join(output)

        print output

        print "\n=========================================\n"

    # close socket
    socket_handler.close()

output = []
output += ['\nStatistics:']
output += ['  - success            {0}'.format(succ)]
output += ['  - fail               {0}'.format(fail)]

if len(delays) > 0:
    output += ['  - min/max/avg delay  {0:.03f}/{1:.03f}/{2:.03f}'.format(
        min(delays),
        max(delays),
        float(sum(delays)) / float(len(delays)))]

output = '\n'.join(output)
print output

raw_input("\nPress return to close this window...")
