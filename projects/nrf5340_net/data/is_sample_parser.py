import struct
import numpy as np
import matplotlib.pyplot as plt

import math

NUM_SAMPLES  = 160
C            = 299792458 # speed of ligh 
ANT_D        = 0.035     # meter
CENTER_FREQ  = (2404*1e6)# channel 0

nrf5340_port = {
    'red'  : 'COM9',
    'black': 'COM11',
}

key = 'red'

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

switching_time  = [
            [
                12 + switching_space*switch_index + 1 + t_unit*i for i in range(num_sample_per_slot)
            ] for switch_index in range((NUMBEROF8US*8-12)/switching_space)
        ]
  
time = reference_time
for switch in switching_time:
    time += switch

time = [4 + 0.125*i for i in range(8*8)] + [12+1 + 0.125*i for i in range(12*8)]

time_data = time

sample_file = 'samples_{0}.txt'.format(key)
# sample_file = 'samples_black_scum.txt'

def aoa_angle_calculation(phase_data, num_pkt, array):
    
    # find a complete sweep from -201 to 201
    
    # 64 
    index_1 = 0
    index_2 = 0
    for index in range(64):
        if phase_data[index] - phase_data[index+1] > 300:
            if  index_1 == 0:
                index_1 = index+1
            else:
                index_2 = index+1
                break   
    
    if index_1 != 0 and index_2 != 0 and index_2 - index_1 > 20 and index_2 - index_1 < 44:
        
        IF = 1 / ((index_2 - index_1) * t_unit * 1e-6)
        freq = CENTER_FREQ + IF
        
        wave_length = (1.0/freq) * C  # in meters
        reference_phase_data = phase_data[index_1:index_2]
    else:
        return None
    
    # generate phase data for ANT used reference at each sampling point
    
    target_phase_data = phase_data[:index_2]
    
    # total number of samples is 160, 8 is the samples missed during first antenna switching slot
    while len(target_phase_data) < 160 + 8: 
        target_phase_data += reference_phase_data
    
    target_phase_data = target_phase_data[:64] + target_phase_data[64+8:160+8]
    
    # compare to the phase data between generated and sampled
    
    figure, temp = plt.subplots()
    
    i = 8
    
    data_1 = phase_data[8*i:8*(i+1)]        + phase_data[8*(i+4):8*(i+5)]           + phase_data[8*(i+8):8*(i+9)]
    data_2 = target_phase_data[8*i:8*(i+1)] + target_phase_data[8*(i+4):8*(i+5)]    + target_phase_data[8*(i+8):8*(i+9)]
    
    phase_diff_ant_1_1 = []
    for index in range(len(data_1)):
        diff = data_2[index]-data_1[index]
        if diff <= -201:
            diff += 402
        if diff >= 201:
            diff -= 402
        phase_diff_ant_1_1.append(diff) 
        
    temp.plot(phase_diff_ant_1_1, 'o-', label='ant1.2 and ant1.1')
    
    i += 2
    
    data_1 = phase_data[8*i:8*(i+1)]        + phase_data[8*(i+4):8*(i+5)]           + phase_data[8*(i+8):8*(i+9)]
    data_2 = target_phase_data[8*i:8*(i+1)] + target_phase_data[8*(i+4):8*(i+5)]    + target_phase_data[8*(i+8):8*(i+9)]
    
    phase_diff_ant_1_3 = []
    for index in range(len(data_1)):
        diff = data_1[index] - data_2[index]
        if diff <= -201:
            diff += 402
        if diff >= 201:
            diff -= 402
        phase_diff_ant_1_3.append(diff) 
        
    temp.plot(phase_diff_ant_1_3, 'o-', label='ant1.2 and ant1.3')
    temp.legend()
    figure.savefig('sampe_pkt_phase_diff_{0}'.format(num_pkt))
    
    # calculate the phase degree
    
    theta_1 = None
    theta_2 = None
    
    # phase_diff_ant_1_1 = phase_diff_ant_1_1[:8]
    # phase_diff_ant_1_3 = phase_diff_ant_1_3[:8]
    
    print "arccos(x) = {0} phase_diff = {1} wave length = {2} ANT_diff = {3}".format(
        (((sum(phase_diff_ant_1_1)/len(phase_diff_ant_1_1))/402.0) * wave_length) / ANT_D, 
        (sum(phase_diff_ant_1_1)/len(phase_diff_ant_1_1))/402.0, 
        wave_length, 
        ANT_D
    )
    
    arccos_x = (((sum(phase_diff_ant_1_1)/len(phase_diff_ant_1_1))/402.0) * wave_length) / ANT_D
    if arccos_x <= 1 and arccos_x >= -1:
        theta_1 = math.acos(arccos_x)
    else:
        print("invalided arcos_x {0}".format(arccos_x))
        return None
    

    print "arccos(x) = {0} phase_diff = {1} wave length = {2} ANT_diff = {3}".format(
        (((sum(phase_diff_ant_1_3)/len(phase_diff_ant_1_3))/402.0) * wave_length) / ANT_D, 
        (sum(phase_diff_ant_1_3)/len(phase_diff_ant_1_3))/402.0, 
        wave_length, 
        ANT_D
    )
    arccos_x = (((sum(phase_diff_ant_1_3)/len(phase_diff_ant_1_3))/402.0) * wave_length) / ANT_D
    
    if arccos_x <= 1 and arccos_x >= -1:
        theta_2 = math.acos(arccos_x)
    else:
        print("invalided arcos_x {0}".format(arccos_x))
        return None
    
    print "array {0} avg angle {1} (angle_1 {2}, angle_2 {3})".format(
        array,
        (math.degrees(float(theta_1)) + math.degrees(float(theta_2)))/2,
        math.degrees(theta_1), 
        math.degrees(theta_2)
    )
    
    # a better angle calculation
    arctan_x = 2 * math.tan(theta_1)*math.tan(theta_2)/(math.tan(theta_1)+math.tan(theta_2))
    
    print "array {0} calculated angle {1} (angle_1 {2}, angle_2 {3})".format(
        array,
        math.degrees(math.atan(arctan_x)),
        math.degrees(theta_1), 
        math.degrees(theta_2)
    )
    
    
    return target_phase_data
    

with open(sample_file, 'r') as f:
    fig, ax = plt.subplots(figsize=(10,8))
    num_pkt = 0
    for line in f:
        one_entry_samples = eval(line)
        num_pkt += 1
        print "one_entry_samples with length = {0} for pkt {1}".format(len(one_entry_samples['samples']), num_pkt)
        magPhase  = {'mag_data': [], 'phase_data': []}
        for sample in one_entry_samples['samples']:
            magPhase['mag_data'].append(sample['magnitude'])
            magPhase['phase_data'].append(sample['phase'])
        # ax.plot(time_data, magPhase['mag_data'], '^-', label='mag_data')
        # ax.plot(time_data, magPhase['phase_data'], '*-', label='phase_data')
        
        # determine angles
        result = aoa_angle_calculation(magPhase['phase_data'], num_pkt, one_entry_samples['array'])
        
        if result:
            ax.plot(time_data, result, '--', label='reference phase data')
        
        # only plots samples in sample slots
        i = 8
        ax.plot(time_data[:8*i], magPhase['mag_data'][:8*i], 'k^', label='mag_data_ant1.2')
        ax.plot(time_data[:8*i], magPhase['phase_data'][:8*i], '*-', label='phase_data_ant1.2')

        ax.plot(
            time_data[8*i:8*(i+1)]              + time_data[8*(i+4):8*(i+5)]                + time_data[8*(i+8):8*(i+9)], 
            magPhase['mag_data'][8*i:8*(i+1)]   + magPhase['mag_data'][8*(i+4):8*(i+5)]     + magPhase['mag_data'][8*(i+8):8*(i+9)], 
            'k^',
            label='mag_data_antt1.1'
        )
        ax.plot(
            time_data[8*i:8*(i+1)]              + time_data[8*(i+4):8*(i+5)]                + time_data[8*(i+8):8*(i+9)], 
            magPhase['phase_data'][8*i:8*(i+1)] + magPhase['phase_data'][8*(i+4):8*(i+5)]   + magPhase['phase_data'][8*(i+8):8*(i+9)], 
            '*',
            label='phase_data_antt1.1'
        )
        i += 2
        
        ax.plot(
            time_data[8*i:8*(i+1)]              + time_data[8*(i+4):8*(i+5)]                + time_data[8*(i+8):8*(i+9)], 
            magPhase['mag_data'][8*i:8*(i+1)]   + magPhase['mag_data'][8*(i+4):8*(i+5)]     + magPhase['mag_data'][8*(i+8):8*(i+9)], 
            'k^',
            label='mag_data_antt1.3'
        )
        ax.plot(
            time_data[8*i:8*(i+1)]              + time_data[8*(i+4):8*(i+5)]                + time_data[8*(i+8):8*(i+9)], 
            magPhase['phase_data'][8*i:8*(i+1)] + magPhase['phase_data'][8*(i+4):8*(i+5)]   + magPhase['phase_data'][8*(i+8):8*(i+9)], 
            '*',
            label='phase_data_antt1.3'
        )
        # i += 2
        # ax.plot(time_data[8*i:8*(i+1)], magPhase['mag_data'][8*i:8*(i+1)], '^-', label='phase_data_antt1.3')
        # ax.plot(time_data[8*i:8*(i+1)], magPhase['phase_data'][8*i:8*(i+1)], '*-', label='phase_data_antt1.3')
        # i += 2
        # ax.plot(time_data[8*i:8*(i+1)], magPhase['mag_data'][8*i:8*(i+1)], '^-', label='phase_data_antt1.2')
        # ax.plot(time_data[8*i:8*(i+1)], magPhase['phase_data'][8*i:8*(i+1)], '*-', label='phase_data_antt1.2')
        # i += 2
        # ax.plot(time_data[8*i:8*(i+1)], magPhase['mag_data'][8*i:8*(i+1)], '^-', label='phase_data_antt1.3')
        # ax.plot(time_data[8*i:8*(i+1)], magPhase['phase_data'][8*i:8*(i+1)], '*-', label='phase_data_antt1.3')
        # i += 2
        # ax.plot(time_data[8*i:8*(i+1)], magPhase['mag_data'][8*i:8*(i+1)], '^-', label='phase_data_antt1.2')
        # ax.plot(time_data[8*i:8*(i+1)], magPhase['phase_data'][8*i:8*(i+1)], '*-', label='phase_data_antt1.2')
        # i += 2
        # ax.plot(time_data[8*i:8*(i+1)], magPhase['mag_data'][8*i:8*(i+1)], '^-', label='phase_data_antt1.2')
        # ax.plot(time_data[8*i:8*(i+1)], magPhase['phase_data'][8*i:8*(i+1)], '*-', label='phase_data_antt1.3')
        
        ax.set_ylim(-500, 500)
        ax.set_xlim(0,    30)
        ax.set_xlabel('time (us)')
        ax.grid(True)
        ax.legend(loc=2)
        fig.savefig('sampe_pkt_{0}_{1}-{2}-{3}'.format(
                num_pkt,
                ((one_entry_samples['setting']>>10) & 0x1f), 
                ((one_entry_samples['setting']>>5)  & 0x1f),
                ((one_entry_samples['setting']>>0)  & 0x1f)
            )
        )
        ax.clear()
        