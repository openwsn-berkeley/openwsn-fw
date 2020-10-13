import matplotlib.pyplot as plt
import math
from config import *

def aoa_angle_calculation(phase_data, num_pkt, array):
    
    # find first two adjencent time point when phase wrapped
    
    debug_print("phase_data :",var1=phase_data)
    
    # 64 
    index_1 = 0
    index_2 = 0
    for index in range(num_sample_in_reference-1):
        # phase varies from -201 to 200 (-180 to 180 degree)
        if phase_data[index] - phase_data[index+1] > 300:
            if  index_1 == 0:
                index_1 = index+1
            else:
                index_2 = index+1
                break
    
    if index_1 != 0 and index_2 != 0 and index_2 - index_1 > 15 and index_2 - index_1 < 49:
        
        IF = 1 / ((index_2 - index_1) * t_unit * 1e-6)
        freq = CENTER_FREQ + IF
        
        wave_length = (1.0/freq) * C  # in meters
        reference_phase_data = phase_data[index_1:index_2]
        step = (phase_data[index_2-1] - phase_data[index_1])/(index_2-index_1-1)
        
        debug_print("step={0}".format(step))
    else:
        return None, None
    
    # ==== generate phase data for ANT used reference at each sampling point
    
    phase_data_ant2 = phase_data[:index_2]
    
    # ---- ref: section 3.2 First IQ sample in nwp_036.pdf (Direction Finding nWP-036)
    
    while len(phase_data_ant2) < NUM_SAMPLES + num_sample_per_slot: 
        new_phase = phase_data_ant2[-1] + step
        if new_phase > 201:
            new_phase -= 403
        phase_data_ant2.append(new_phase)
    phase_data_ant2 = phase_data_ant2[:64] + phase_data_ant2[64+8:160+8]
    
    # ---- calculate phase diff bewteen ANT2 and ANTx
    
    if DEBUG_ON:
        figure, temp = plt.subplots()
    
    def calc_phase_diff(shift=0, reversed=False):
        
        i = 8 + shift
        phase_ant_x_data = phase_data[8*i:8*(i+1)]      + phase_data[8*(i+4):8*(i+5)]           + phase_data[8*(i+8):8*(i+9)]
        phase_ant_2_data = phase_data_ant2[8*i:8*(i+1)] + phase_data_ant2[8*(i+4):8*(i+5)]    + phase_data_ant2[8*(i+8):8*(i+9)]
        
        phase_diff_ant_2_x = []
        for index in range(len(phase_ant_x_data)):
        
            diff = phase_ant_2_data[index]-phase_ant_x_data[index]
            
            if reversed:
                diff = 0 - diff
                
            # if diff <= -201:
                # diff += 402
            # elif diff >= 201:
                # diff -= 402
            if diff > (0-VALID_PHASE_DIFF) and diff < VALID_PHASE_DIFF:
                phase_diff_ant_2_x.append(diff)
            
        if DEBUG_ON:
            if reversed:
                x = 3
            else:
                x = 1
            temp.plot(phase_diff_ant_2_x, 'o-', label='phase_diff_ant_2_{0}'.format(x))
            
        if len(phase_diff_ant_2_x) > 0:
            debug_print("phase_diff_ant2_x and avg_value", phase_diff_ant_2_x, sum(phase_diff_ant_2_x)/len(phase_diff_ant_2_x))
            return (sum(phase_diff_ant_2_x)/len(phase_diff_ant_2_x)) / 402.0
        else:
            return None
            
    avg_phase_diff_ant_2_1 = calc_phase_diff(0, False)
    avg_phase_diff_ant_3_2 = calc_phase_diff(2, True)
    
    if DEBUG_ON:
        debug_print("phase diff ant2_1 and ant3_2", avg_phase_diff_ant_2_1, avg_phase_diff_ant_3_2)
        temp.legend()
        figure.savefig('figs\sampe_pkt_phase_diff_{0}'.format(num_pkt))
        
    if avg_phase_diff_ant_2_1 == None or avg_phase_diff_ant_3_2 == None:
        return phase_data_ant2, None
    
    # ==== calculate the angles according to phase diff
    
    theta_1 = None
    theta_2 = None
    
    def calculate_angle(avg_phase_diff):
        
        arccos_x = (avg_phase_diff * wave_length) / ANT_D
        if arccos_x <= 1 and arccos_x >= -1:
            theta = math.acos(arccos_x)
        else:
            return None
            
        return theta
            
    theta_1 = calculate_angle(avg_phase_diff_ant_2_1)
    theta_2 = calculate_angle(avg_phase_diff_ant_3_2)

    # ==== provide one angle from 0 (right) to 180 (left)
    
    if theta_1 != None and theta_2 != None:
        if math.tan(theta_1)+math.tan(theta_2) != 0:
            angle = 180 * (
                    math.atan(
                        2 * math.tan(theta_1)*math.tan(theta_2)/(math.tan(theta_1)+math.tan(theta_2))
                    ) / M_PI
            )
        else:
            angle = None
        if angle < 0:
            angle = 180 + angle
    elif theta_1 == None and theta_2 != None:
        angle = 180 * (theta_2) / M_PI
    elif theta_1 != None and theta_2 == None:
        angle = 180 * (theta_1) / M_PI
    else:
        debug_print("illegal data, theta1 = var1 theta2=var2", theta_1, theta_2)
        return None, None
    
    debug_print("off-board angle  = {0}".format(angle), theta_1, theta_2)
    return phase_data_ant2, angle
    
