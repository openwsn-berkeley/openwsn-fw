#include <stdio.h>
#include <string.h>

#include "memory_map.h"
#include "scm3c_hw_interface.h"

#include "radio.h"
#include "scum_defs.h"

//=========================== defines =========================================

//=========================== variables =======================================

typedef struct {
    uint8_t     optical_cal_iteration;
    uint8_t     optical_cal_finished;
    
    uint32_t    num_32k_ticks_in_100ms;
    uint32_t    num_2MRC_ticks_in_100ms;
    uint32_t    num_IFclk_ticks_in_100ms;
    uint32_t    num_LC_ch11_ticks_in_100ms;
    uint32_t    num_HFclock_ticks_in_100ms;

    // reference to calibrate
    uint32_t    LC_target;
    uint32_t    LC_code;
} optical_vars_t;

optical_vars_t optical_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

void optical_init(void) {
    
    memset(&optical_vars, 0, sizeof(optical_vars_t));
    
    // Target radio LO freq = 2.4025G
    // Divide ratio is currently 480*2
    // Calibration counts for 100ms
    optical_vars.LC_target  = REFERENCE_LC_TARGET;
    optical_vars.LC_code    = DEFUALT_INIT_LC_CODE;
}

uint8_t optical_getCalibrationFinshed(void) {
    return optical_vars.optical_cal_finished;
}

void optical_enable(void){
    ISER = 0x0800;
}

//=========================== interrupt =======================================

// This interrupt goes off every time 32 new bits of data have been shifted into the optical register
// Do not recommend trying to do any CPU intensive actions while trying to receive optical data
// ex, printf will mess up the received data values
void optical_32_isr(){
    //printf("Optical 32-bit interrupt triggered\r\n");
    
    //unsigned int LSBs, MSBs, optical_shiftreg;
    //int t;
    
    // 32-bit register is analog_rdata[335:304]
    //LSBs = ANALOG_CFG_REG__19; //16 LSBs
    //MSBs = ANALOG_CFG_REG__20; //16 MSBs
    //optical_shiftreg = (MSBs << 16) + LSBs;    
    
    // Toggle GPIO 0
    //GPIO_REG__OUTPUT ^= 0x1;
    
}

// This interrupt goes off when the optical register holds the value {221, 176, 231, 47}
// This interrupt can also be used to synchronize to the start of an optical data transfer
// Need to make sure a new bit has been clocked in prior to returning from this ISR, or else it will immediately execute again
void optical_sfd_isr(){
    
    uint32_t rdata_lsb, rdata_msb; 
    uint32_t count_LC, count_32k, count_2M, count_HFclock, count_IF;
    
    uint32_t HF_CLOCK_fine;
    uint32_t HF_CLOCK_coarse;
    uint32_t RC2M_coarse;
    uint32_t RC2M_fine;
    uint32_t RC2M_superfine;
    uint32_t IF_clk_target;
    uint32_t IF_coarse;
    uint32_t IF_fine;
    
    HF_CLOCK_fine       = scm3c_hw_interface_get_HF_CLOCK_fine();
    HF_CLOCK_coarse     = scm3c_hw_interface_get_HF_CLOCK_coarse();
    RC2M_coarse         = scm3c_hw_interface_get_RC2M_coarse();
    RC2M_fine           = scm3c_hw_interface_get_RC2M_fine();
    RC2M_superfine      = scm3c_hw_interface_get_RC2M_superfine();
    IF_clk_target       = scm3c_hw_interface_get_IF_clk_target();
    IF_coarse           = scm3c_hw_interface_get_IF_coarse();
    IF_fine             = scm3c_hw_interface_get_IF_fine();
        
    // Disable all counters
    ANALOG_CFG_REG__0 = 0x007F;
    
    // Keep track of how many calibration iterations have been completed
    optical_vars.optical_cal_iteration++;
        
    // Read 32k counter
    rdata_lsb = *(unsigned int*)(APB_ANALOG_CFG_BASE + 0x000000);
    rdata_msb = *(unsigned int*)(APB_ANALOG_CFG_BASE + 0x040000);
    count_32k = rdata_lsb + (rdata_msb << 16);

    // Read HF_CLOCK counter
    rdata_lsb = *(unsigned int*)(APB_ANALOG_CFG_BASE + 0x100000);
    rdata_msb = *(unsigned int*)(APB_ANALOG_CFG_BASE + 0x140000);
    count_HFclock = rdata_lsb + (rdata_msb << 16);

    // Read 2M counter
    rdata_lsb = *(unsigned int*)(APB_ANALOG_CFG_BASE + 0x180000);
    rdata_msb = *(unsigned int*)(APB_ANALOG_CFG_BASE + 0x1C0000);
    count_2M = rdata_lsb + (rdata_msb << 16);
        
    // Read LC_div counter (via counter4)
    rdata_lsb = *(unsigned int*)(APB_ANALOG_CFG_BASE + 0x280000);
    rdata_msb = *(unsigned int*)(APB_ANALOG_CFG_BASE + 0x2C0000);
    count_LC = rdata_lsb + (rdata_msb << 16);
    
    // Read IF ADC_CLK counter
    rdata_lsb = *(unsigned int*)(APB_ANALOG_CFG_BASE + 0x300000);
    rdata_msb = *(unsigned int*)(APB_ANALOG_CFG_BASE + 0x340000);
    count_IF = rdata_lsb + (rdata_msb << 16);
    
    // Reset all counters
    ANALOG_CFG_REG__0 = 0x0000;        
    
    // Enable all counters
    ANALOG_CFG_REG__0 = 0x3FFF;    
        
    // Don't make updates on the first two executions of this ISR
    if(optical_vars.optical_cal_iteration > 2){
        
        // Do correction on HF CLOCK
        // Fine DAC step size is about 6000 counts
        if(count_HFclock < 1997000) {
            HF_CLOCK_fine--;
        }
        if(count_HFclock > 2003000) {
            HF_CLOCK_fine++;
        }
        
        set_sys_clk_secondary_freq(HF_CLOCK_coarse, HF_CLOCK_fine);
        scm3c_hw_interface_set_HF_CLOCK_coarse(HF_CLOCK_coarse);
        scm3c_hw_interface_set_HF_CLOCK_fine(HF_CLOCK_fine);
        
        // Do correction on LC
        if(count_LC > (optical_vars.LC_target + 0)) {
            optical_vars.LC_code -= 1;
        }
        if(count_LC < (optical_vars.LC_target - 0)) {
            optical_vars.LC_code += 1;
        }
        LC_monotonic(optical_vars.LC_code);
            
        // Do correction on 2M RC
        // Coarse step ~1100 counts, fine ~150 counts, superfine ~25
        // Too fast
        if(count_2M > (200600)) {
            RC2M_coarse += 1;
        } else {
            if(count_2M > (200080)) {
                RC2M_fine += 1;
            } else {
                if(count_2M > (200015)) {
                    RC2M_superfine += 1;
                }
            }
        } 
        
        // Too slow
        if(count_2M < (199400)) {
            RC2M_coarse -= 1;
        } else {
            if(count_2M < (199920)) {
                RC2M_fine -= 1;
            } else {
                if(count_2M < (199985)) {
                    RC2M_superfine -= 1;
                }
            }
        }
        
        set_2M_RC_frequency(31, 31, RC2M_coarse, RC2M_fine, RC2M_superfine);
        scm3c_hw_interface_set_RC2M_coarse(RC2M_coarse);
        scm3c_hw_interface_set_RC2M_fine(RC2M_fine);
        scm3c_hw_interface_set_RC2M_superfine(RC2M_superfine);

        // Do correction on IF RC clock
        // Fine DAC step size is ~2800 counts
        if(count_IF > (1600000+1400)) {
            IF_fine += 1;
        }
        if(count_IF < (1600000-1400)) {
            IF_fine -= 1;
        }
        
        set_IF_clock_frequency(IF_coarse, IF_fine, 0);
        scm3c_hw_interface_set_IF_coarse(IF_coarse);
        scm3c_hw_interface_set_IF_fine(IF_fine);
        
        analog_scan_chain_write();
        analog_scan_chain_load();    
    }
    
    // Debugging output
    printf("HF=%d-%d   2M=%d-%d,%d,%d   LC=%d-%d   IF=%d-%d\r\n",count_HFclock,HF_CLOCK_fine,count_2M,RC2M_coarse,RC2M_fine,RC2M_superfine,count_LC,optical_vars.LC_code,count_IF,IF_fine); 
     
    if(optical_vars.optical_cal_iteration == 25){
        // Disable this ISR
        ICER = 0x0800;
        optical_vars.optical_cal_iteration = 0;
        optical_vars.optical_cal_finished = 1;
        
        // Store the last count values
        optical_vars.num_32k_ticks_in_100ms = count_32k;
        optical_vars.num_2MRC_ticks_in_100ms = count_2M;
        optical_vars.num_IFclk_ticks_in_100ms = count_IF;
        optical_vars.num_LC_ch11_ticks_in_100ms = count_LC;
        optical_vars.num_HFclock_ticks_in_100ms = count_HFclock;
    
        // Debug prints
        //printf("LC_code=%d\r\n", optical_vars.LC_code);
        //printf("IF_fine=%d\r\n", IF_fine);
        
        printf("done\r\n");
                
        // This was an earlier attempt to build out a complete table of LC_code for TX/RX on each channel
        // It doesn't really work well yet so leave it commented
        //printf("Building channel table...");
        
        //radio_build_channel_table(LC_code);
        
        //printf("done\r\n");
        
        //radio_disable_all();
        
        // Halt all counters
        ANALOG_CFG_REG__0 = 0x0000;
    }
}
