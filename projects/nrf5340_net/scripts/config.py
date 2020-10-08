'''
 Configuration file for direction finding.

'''

C                           = 299792458 # speed of ligh 
ANT_D                       = 0.035     # meter
CENTER_FREQ                 = (2404*1e6)# channel 0
M_PI		                = 3.14159265358979323846

NUM_SAMPLES                 = 160
NUMBEROF8US                 = 3  # in unit of 8 us
TSWITCHSPACING              = 2  # 1:4us 2:2us 3: 1us
TSAMPLESPACINGREF           = 6  # 1:4us 2:2us 3: 1us 4:500ns 5:250ns 6:125ns
SAMPLETYPE                  = 1  # 0: IQ  1: magPhase
TSAMPLESPACING              = 6  # 1:4us 2:2us 3: 1us 4:500ns 5:250ns 6:125ns

num_sample_per_slot         = 8
num_sample_in_reference     = 64
switching_space             = 2
t_unit                      = 0.125 # micro-seconds

nrf5340_port                = 'COM9'
sample_file                 = 'samples.txt'

new_read_mark       = 255
num_new_read_mark   = 4

DEBUG_ON                    = 0

def debug_print(str, var1 = None, var2 = None, var3 = None):
    if DEBUG_ON:
        print "{0} var1={1} var2={2} var3={3}".format(str, var1, var2, var3)
