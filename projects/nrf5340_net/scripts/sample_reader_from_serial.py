import serial
import struct
from config import *
from calc_angle import *

latest_angle = [None, None]

def start_read():

    sample = {
        'magnitude' : 0,
        'phase'     : 0,
    }
    samples = []

    input_data = []
    count = num_new_read_mark
    new_sample_read = False
    
    s = serial.Serial(nrf5340_port, baudrate=115200)

    data = {'samples':[]}

    while True:

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
            if len(input_data) == NUM_SAMPLES*4+10:
                for i in range(NUM_SAMPLES):
                    magnitude, phase = struct.unpack('>Hh', ''.join(input_data[4*i: 4*(i+1)]))
                    sample['magnitude'] = magnitude
                    sample['phase']     = phase
                    data['samples'].append(sample.copy())
                data['rssi']        = struct.unpack('>b', ''.join(input_data[NUM_SAMPLES*4]))
                data['setting']     = ((ord(input_data[NUM_SAMPLES*4+1])) << 10 ) | ((ord(input_data[NUM_SAMPLES*4+2])) << 5) | (ord(input_data[NUM_SAMPLES*4+3]))
                data['array']       = ord(input_data[NUM_SAMPLES*4+4])
                data['angle']       = ord(input_data[NUM_SAMPLES*4+5])
                
                phase_data  = []
                for sample in data['samples']:
                    phase_data.append(sample['phase'])
                _, angle = aoa_angle_calculation(phase_data, 0, data['array'])
                
                if data['angle'] != 254:
                    latest_angle[int(data['array'])-1] = data['angle']
                
                with open(sample_file, 'a') as f:
                    f.write(str(data)+'\n')
                    data['samples'] = []
            else:
                print len(input_data)
                
            input_data = []