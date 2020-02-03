import serial
import random
import argparse
import time

def program_cortex(teensy_port="COM15", scum_port="COM18", binary_image="./code.bin",
        boot_mode='optical', skip_reset=False, insert_CRC=False,
        pad_random_payload=False):
    """
    Inputs:
        teensy_port: String. Name of the COM port that the Teensy
            is connected to.
        scum_port: String. Name of the COM port that the UART 
            is connected to. If None, does not attempt to connect
            to SCM via UART.
        binary_image: String. Path to the binary file to 
            feed to Teensy to program SCM. This binary file shold be
            compiled using whatever software is meant to end up 
            on the Cortex. This group tends to compile it using Keil
            projects.
        boot_mode: String. 'optical' or '3wb'. The former will assume an
            optical bootload, whereas the latter will use the 3-wire
            bus.
        skip_reset: Boolean. True: Skip hard reset before optical 
            programming. False: Perform hard reset before optical programming.
        insert_CRC: Boolean. True = insert CRC for payload integrity 
            checking. False = do not insert CRC. Note that SCM C code 
            must also be set up for CRC check for this to work.
        pad_random_payload: Boolean. True = pad unused payload space with 
            random data and check it with CRC. False = pad with zeros, do 
            not check integrity of padding. This is useful to check for 
            programming errors over full 64kB payload.
    Outputs:
        No return value. Feeds the input from binary_image to the Teensy to program SCM
        and programs SCM. 
    Raises:
        ValueError if the boot_mode isn't 'optical' or '3wb'.
    Notes:
        When setting optical parameters, all values can be toggled to improve
        success when programming. In particular, too small a third value can
        cause the optical programmer to lose/eat the short pulses.
    """
    # Open COM port to Teensy
    teensy_ser = serial.Serial(
        port=teensy_port,
        baudrate=19200,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        bytesize=serial.EIGHTBITS)

    # Open binary file from Keil
    with open(binary_image, 'rb') as f:
        bindata = bytearray(f.read())

    # Need to know how long the binary payload is for computing CRC
    code_length = len(bindata) - 1
    pad_length = 65536 - code_length - 1

    # Optional: pad out payload with random data if desired
    # Otherwise pad out with zeros - uC must receive full 64kB
    if(pad_random_payload):
        for i in range(pad_length):
            bindata.append(random.randint(0,255))
        code_length = len(bindata) - 1 - 8
    else:
        for i in range(pad_length):
            bindata.append(0)

    if insert_CRC:
        # Insert code length at address 0x0000FFF8 for CRC calculation
        # Teensy will use this length value for calculating CRC
        bindata[65528] = code_length % 256 
        bindata[65529] = code_length // 256
        bindata[65530] = 0
        bindata[65531] = 0
    
    # Transfer payload to Teensy
    teensy_ser.write(b'transfersram\n')
    print(teensy_ser.readline())
    # Send the binary data over uart
    teensy_ser.write(bindata)

    if insert_CRC:
        # Have Teensy calculate 32-bit CRC over the code length 
        # It will store the 32-bit result at address 0x0000FFFC
        teensy_ser.write(b'insertcrc\n')

    if boot_mode == 'optical':
        # Configure parameters for optical TX
        teensy_ser.write(b'configopt\n')
        teensy_ser.write(b'80\n')
        teensy_ser.write(b'80\n')
        teensy_ser.write(b'3\n')
        teensy_ser.write(b'80\n')
        
        # Encode the payload into 4B5B for optical transmission
        teensy_ser.write(b'encode4b5b\n')

        if not skip_reset:
            # Do a hard reset and then optically boot
            teensy_ser.write(b'bootopt4b5b\n')
        else:
            # Skip the hard reset before booting
            teensy_ser.write(b'bootopt4b5bnorst\n')

        # Display confirmation message from Teensy
        print(teensy_ser.readline())
        # in case the large image's CRC-check takes long, delay 2 seconds to start optical calibration
        time.sleep(2)
        teensy_ser.write(b'opti_cal\n');
    elif boot_mode == '3wb':
        # Execute 3-wire bus bootloader on Teensy
        teensy_ser.write(b'boot3wb\n')

        # Display confirmation message from Teensy
        print(teensy_ser.readline())
        print(teensy_ser.readline())
        teensy_ser.write(b'3wb_cal\n')
    else:
        raise ValueError("Boot mode '{}' invalid.".format(boot_mode))

    teensy_ser.close()

    # Open UART connection to SCM
    if scum_port != None:
        uart_ser = serial.Serial(
            port=scum_port,
            baudrate=19200,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            bytesize=serial.EIGHTBITS,
            timeout=.5)

        # After programming, several lines are sent from SCM over UART
        for _ in range(5):
            print(uart_ser.readline())

        uart_ser.close()

    return

if __name__ == "__main__":
    
    parser = argparse.ArgumentParser(description='bootlaod script arguments')
    
    parser.add_argument('-tp', '--teensy_port',
        dest='teensy_port', 
        default='COM12',
        action='store', 
        help='Name of the COM port that the Teensy\
            is connected to.'
    )
    
    parser.add_argument('-sp', '--scum_port',
        dest='scum_port',
        action='store',
        default=None,
        help='Name of the COM port that the UART \
            is connected to. If None, does not attempt to connect\
            to SCM via UART.'
    )
    
    parser.add_argument('-i','--image',
        dest='binary_image',
        default='pingpong_test.bin',
        help='Path to the binary file to \
            feed to Teensy to program SCM. This binary file shold be\
            compiled using whatever software is meant to end up \
            on the Cortex. This group tends to compile it using Keil\
            projects.'
    )
    
    parser.add_argument('-bm','--boot_mode',
        dest='boot_mode',
        default='optical',
        help="'optical' or '3wb'. \
            The former will assume an optical bootload,\
            whereas the latter will use the 3-wire \
            bus."
    )
    
    parser.add_argument('-sr','--skip_reset',
        dest='skip_reset',
        default=False,
        help='Boolean. True: Skip hard reset before optical \
            programming. False: Perform hard reset before \
            optical programming.'
    )
    
    parser.add_argument('-c','--insert_CRC',
        dest='insert_CRC',
        default=True,
        help='True = insert CRC for payload integrity \
            checking. False = do not insert CRC. Note that SCM C code \
            must also be set up for CRC check for this to work.'
    )
    
    parser.add_argument('-pl','--pad_random_payload',
        dest='pad_random_payload',
        default=False,
        help='True = True = pad unused payload space with \
            random data and check it with CRC. False = pad with zeros, do \
            not check integrity of padding. This is useful to check for \
            programming errors over full 64kB payload.'
    )
    
    argspace = vars(parser.parse_args())
    program_cortex(**argspace)
