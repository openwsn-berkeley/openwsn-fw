import getopt
import sys
import serial

if __name__ == "__main__":

    conf = {
        'port': 'auto',
        'baud': 1000000,
    }
    
    try:
        opts, args = getopt.getopt(sys.argv[1:], "hp:b:", ['help'])
    except getopt.GetoptError as err:
        print(str(err))
        usage()
        sys.exit(2)

    for o, a in opts:
        if o == '-p':
            conf['port'] = a
        elif o == '-b':
            conf['baud'] = eval(a)

    try:
        args[0]
    except:
        raise Exception('No file path given.')

    # open serial port
    ser = serial.Serial(
        port=conf['port'],
        baudrate=conf['baud'],
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        bytesize=serial.EIGHTBITS
    )
    
    with open(args[0], "rb") as f:
        data = f.read()
    assert len(data)<65535, 'the image has to be smaller than 64KB!'

    # write secret characters
    ser.write("transfersram\n")
    print ser.readline()

    # Send the binary data over uart
    ser.write(data[0:65535])
    # Send all zeros to pad out to the full size of sram (64kB)
    ser.write("\0" * (65536-len(data)))
    print ser.readline()

    ser.write("boot3wb\n")
    print ser.readline()

    print "transfer complete"

    ser.close() 