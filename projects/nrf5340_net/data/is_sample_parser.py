import struct
import numpy as np
import matplotlib.pyplot as plt

NUM_SAMPLES  = 160

nrf5340_port = {
    'red'  : 'COM9',
    'black': 'COM11',
}

key = 'black'

NUMBEROF8US         = 3  # in unit of 8 us
TSWITCHSPACING      = 2  # 1:4us 2:2us 3: 1us
TSAMPLESPACINGREF   = 6  # 1:4us 2:2us 3: 1us 4:500ns 5:250ns 6:125ns
SAMPLETYPE          = 1  # 0: IQ  1: magPhase
TSAMPLESPACING      = 6  # 1:4us 2:2us 3: 1us 4:500ns 5:250ns 6:125ns

num_sample_per_slot     = 8
num_sample_in_reference = 64
switching_space         = 2
t_unit                  = 0.125

reference_time  = [ 4 + 0.125*i for i in range(num_sample_in_reference)]

switching_time  =[
            [
                12 + switching_space*switch_index + 1 + t_unit*i for i in range(num_sample_per_slot)
            ] for switch_index in range((NUMBEROF8US*8-12)/switching_space)
        ]
  
time = reference_time
for switch in switching_time:
    time += switch

time = [4 + 0.125*i for i in range(8*8)] + [12 + 0.125*i for i in range(12*8)]

time_data = time

sample_file = 'samples_{0}.txt'.format(key)
# sample_file = 'samples_black_scum.txt'

with open(sample_file, 'r') as f:
    fig, ax = plt.subplots()
    num_pkt = 0
    for line in f:
        one_entry_samples = eval(line)
        print "one_entry_samples with length = {0} for pkt {1}".format(len(one_entry_samples['samples']), num_pkt)
        num_pkt += 1
        magPhase  = {'mag_data': [], 'phase_data': []}
        for sample in one_entry_samples['samples']:
            magPhase['mag_data'].append(sample['magnitude'])
            magPhase['phase_data'].append(sample['phase'])
        ax.plot(time_data, magPhase['mag_data'], '^-', label='mag_data')
        ax.plot(time_data, magPhase['phase_data'], '*-', label='phase_data')
        ax.set_ylim(-400, 400)
        ax.set_xlim(0,    25)
        ax.set_xlabel('time (us)')
        ax.grid(True)
        ax.legend()
        fig.savefig('sampe_pkt_{0}'.format(num_pkt))
        ax.clear()
        