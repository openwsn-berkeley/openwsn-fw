import serial
import struct

NUM_SAMPLES  = 14

nrf5340_port = {
    'red'  : 'COM9',
    'black': 'COM11',
}

sample = {
    'magnitude' : 0,
    'phase'     : 0,
}
samples = []

key = 'black'

new_read_mark       = 255
num_new_read_mark   = 4

input_data = []
count = num_new_read_mark
new_sample_read = False
    
s = serial.Serial(nrf5340_port[key], baudrate=115200)

while 1:

    c = s.read(1)
    input_data.append(c)
    
    if ord(c) == 255:
        count -= 1
    else:
        count = num_new_read_mark
    
    if count == 0:
        new_sample_read = True
        count = num_new_read_mark
    
    if new_sample_read:
        new_sample_read = False
        if len(input_data) == NUM_SAMPLES*4+4:
            for i in range(NUM_SAMPLES):
                magnitude, phase = struct.unpack('>Hh', ''.join(input_data[4*i: 4*(i+1)]))
                sample['magnitude'] = magnitude
                sample['phase']     = phase
                samples.append(sample)
            with open('samples_{0}.txt'.format(key), 'a') as f:
                f.write(str(samples)+'\n')
        else:
            print len(input_data)
            
        input_data = []