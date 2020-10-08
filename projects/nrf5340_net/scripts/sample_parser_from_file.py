import struct
import numpy                as np
import matplotlib.pyplot    as plt

# direction finding related
from config     import *
from calc_angle import *

def generate_time_line():
    
    time_data = [4 + t_unit*i for i in range(num_sample_in_reference)] + \
                [12+switching_space/2 + t_unit*i for i in range((24-12) * num_sample_per_slot)]
    return time_data
    
def get_angle_to_pkt(pkt_id = 0, on_board_calculation=False, contine_mode=False):

    with open(sample_file, 'r') as f:
        num_pkt = 0
        for line in f:
            one_entry_samples = eval(line)
            num_pkt += 1
            
            if contine_mode == False:
                if num_pkt != pkt_id:
                    continue
                
            debug_print( "=========================================================================")
            debug_print("one_entry_samples with length = {0} for pkt {1} (angle_calc_on_board={2})".format(len(one_entry_samples['samples']), num_pkt, one_entry_samples['angle']))
            
            magPhase  = {'mag_data': [], 'phase_data': []}
            for sample in one_entry_samples['samples']:
                magPhase['mag_data'].append(sample['magnitude'])
                magPhase['phase_data'].append(sample['phase'])
                
            time_data = generate_time_line()
            
            # ax.plot(time_data, magPhase['mag_data'], '^-', label='mag_data')
            # ax.plot(time_data, magPhase['phase_data'], '*-', label='phase_data')
            
            # determine angles
            data, angle = aoa_angle_calculation(magPhase['phase_data'], num_pkt, one_entry_samples['array'])
            
            if DEBUG_ON:
                
                debug_fig, ax = plt.subplots(figsize=(10,8))
            
                if data:
                    ax.plot(time_data, data, '--', label='reference phase data')
                    debug_print( "angle calculated off-board = {0}".format(angle))
            
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
                debug_fig.savefig('figs\sampe_pkt_{0}_{1}-{2}-{3}'.format(
                        num_pkt,
                        ((one_entry_samples['setting']>>10) & 0x1f), 
                        ((one_entry_samples['setting']>>5)  & 0x1f),
                        ((one_entry_samples['setting']>>0)  & 0x1f)
                    )
                )
                ax.clear()
            
            if contine_mode == False:
                if on_board_calculation == False:
                    return angle
                else:
                    if one_entry_samples['angle'] == 254:
                        return None
                    else:
                        return one_entry_samples['angle']
            
            
if __name__ == '__main__':
    get_angle_to_pkt(pkt_id=3, contine_mode=False)