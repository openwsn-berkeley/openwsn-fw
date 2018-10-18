#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "memory_Map.h"

unsigned int ASC[38];
char send_packet[127];

unsigned int current_lfsr = 0x12345678;

/* START 2 MHz oscillator related variables */
bool read_counters_now = false;

// coarse1, coarse2, coarse3, fine, superfine dac settings
unsigned int dac_2M_settings[5] = {31, 31, 29, 2, 2};

// buffer of the last ten valid samples
int buffer_samples[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// blip defined as blip_threshold_ticks ticks below the rolling average 
int blip_threshold_ticks = 100;

//threshold above which we adjust the 2 MHz dac
int calibration_threshold_max = 200100;
//threshold below which we adjust the 2 MHz dac
int calibration_threshold_min = 199900;

// resolution of DAC components in ticks/Hz
double fine_resolution = 165;
double superfine_resolution = 55;

// These booleans control modifying of each of the 2 MHz DAC components.
bool modify_coarse1 = false;
bool modify_coarse2 = false;
bool modify_coarse3 = true;
bool modify_fine = true;
bool modify_superfine = true;

//This boolean is true if we have received < 10 packets after changing the DAC settings.
bool first_ten = true;

// 10-point rolling average of the 2 MHz counter
double rolling_average_2M = 200000;

// number of packets received with the current DAC setting
int num_iterations_with_dac_setting = 0;

// general-purpose index for iterating through arrays
int temp_index = 0;

/* END 2 MHz oscillator related variables */


// Reverse endianness of lower 16 bits
unsigned int flip_lsb8(unsigned int in){
    int out = 0;
    
    out |= (0x01 & in) << 7;
    out |= (0x02 & in) << 5;
    out |= (0x04 & in) << 3;
    out |= (0x08 & in) << 1;
    
    out |= (0x10 & in) >> 1;
    out |= (0x20 & in) >> 3;
    out |= (0x40 & in) >> 5;
    out |= (0x80 & in) >> 7;
    
    return out;
}

void analog_scan_chain_write(unsigned int* scan_bits) {
    
    int i = 0;
    int j = 0;
    unsigned int asc_reg;
    
    // analog_cfg<357> is resetb for chip shift register, so leave that high
    
    for (i=37; i>=0; i--) {
        
        //printf("\n%d,%lX\n",i,scan_bits[i]);
        
        for (j=0; j<32; j++) {

        // Set scan_in (should be inverted)
        if((scan_bits[i] & (0x00000001 << j)) == 0)
            asc_reg = 0x21;    
        else
            asc_reg = 0x20;

        // Write asc_reg to analog_cfg
        ANALOG_CFG_REG__22 = asc_reg;

        // Lower phi1
        asc_reg &= ~(0x2);
        ANALOG_CFG_REG__22 = asc_reg;

        // Toggle phi2
        asc_reg |= 0x4;
        ANALOG_CFG_REG__22 = asc_reg;
        asc_reg &= ~(0x4);
        ANALOG_CFG_REG__22 = asc_reg;

        // Raise phi1
        asc_reg |= 0x2;
        ANALOG_CFG_REG__22 = asc_reg;
        
        }    
    }
}

void analog_scan_chain_load(void) {
    
    // Assert load signal (and cfg<357>)
    ANALOG_CFG_REG__22 = 0x0028;

    // Lower load signal
    ANALOG_CFG_REG__22 = 0x0020;

}


/* Prints the value of the 2MHz dac.*/
void print_2MHz_DAC(void) {
    int ind;
    // print the DAC settings
    //printf("2MHz DAC: ");
    for (ind = 0; ind < 5; ind++) {
        printf("%d, ", dac_2M_settings[ind]);
    }
    printf("\n");
}


/* Returns the approximate 2M counter value expected if changing the fine/superfine dac values to the new values.
int approximate_2M_counter(int new_fine_code, int new_superfine_code) {
    int fine_below;
    int superfine_below;
    
    fine_below = (new_fine_code - 2) * fine_resolution;
    superfine_below = (new_superfine_code - 2) * superfine_resolution;
    
    return (200000 - fine_below - superfine_below);
}
*/

/* sets the 2 MHz RC DAC frequency. 
-updates the local dac settings array
-flips endianness and sets the appropriate bits in the scanchain array
-writes it to the scanchain
-loads the scanchain
*/
void set_2M_RC_frequency(int coarse1, int coarse2, int coarse3, int fine, int superfine) {
    
    unsigned int newval;
    unsigned int newcoarse1, newcoarse2, newcoarse3, newfine, newsuperfine;
    unsigned int temp;
    
    /* update our local dac array */
    dac_2M_settings[0] = coarse1;
    dac_2M_settings[1] = coarse2;
    dac_2M_settings[2] = coarse3;
    dac_2M_settings[3] = fine;
    dac_2M_settings[4] = superfine;
    
    // make sure each argument is between 0-31, inclusive
    
    // ASC[34] covers 1088:1119
    newval = ASC[34] & 0x8000001F;
    
    // flip endianness of each
    newcoarse1 = (flip_lsb8(coarse1) >> 3) & 0x1F;
    newcoarse2 = (flip_lsb8(coarse2) >> 3) & 0x1F;
    newcoarse3 = (flip_lsb8(coarse3) >> 3) & 0x1F;
    newfine = (flip_lsb8(fine) >> 3) & 0x1F;
    newsuperfine = (flip_lsb8(superfine) >> 3) & 0x1F;

    newval |= newcoarse1 << 26;
    newval |= newcoarse2 << 21;
    newval |= newcoarse3 << 16;
    newval |= newfine << 11;
    newval |= newsuperfine << 6;
    
    // Enable bit
    newval |= 0x1 << 5;
    
    ASC[34] = newval;
    
    num_iterations_with_dac_setting = 0;
    
    /* flush the buffer so every subsequent read is not considered a blip.*/
    for (temp_index = 0; temp_index < 9; temp_index++) {
        buffer_samples[temp_index] = 0;
    }
    /* set the first_ten boolean back to true so we don't check for blips in the first ten packets.*/
    first_ten = true;
    
    //write to analog scanchain and load
    analog_scan_chain_write(&ASC[0]);
    analog_scan_chain_load();
    
    //print_2MHz_DAC();
}


/* Initializes the 2MHz DAC with values set in the dac_2M_settings array. */
void initialize_2M_DAC(void) {
    set_2M_RC_frequency(dac_2M_settings[0], dac_2M_settings[1], dac_2M_settings[2], dac_2M_settings[3], dac_2M_settings[4]);
    // printf("Initialized 2MHz DAC\n");
    // print_2MHz_DAC();
}


/* returns 1 if the counter was not a blip, on comparison to the rolling average.
Otherwise, returns 0.
*/
int valid_2M_read(int counter) {
    
    int lower_threshold;
    int upper_threshold;
    
    if (first_ten) {
        //lower_threshold = 200000 - blip_threshold_ticks;
        //upper_threshold = 200000 + blip_threshold_ticks;
        return 1;
    } else {
        // +/- blip_threshold_ticks around the rolling average.
        lower_threshold = rolling_average_2M - blip_threshold_ticks;
        upper_threshold = rolling_average_2M + blip_threshold_ticks;
    }
    //printf("L: %d, U: %d\n", lower_threshold, upper_threshold);
    
    if ((counter < lower_threshold) || (counter > upper_threshold)) {
        printf("Blip, %d\n", counter);
        return 0;
    }
    
    return 1;
}

/* Returns 1 if rolling_average_2M is within the calibration window (cal not needed).
Otherwise, returns 0.*/
int is_2M_within_cal_window(void) {
    if ((rolling_average_2M < calibration_threshold_min) || 
        (rolling_average_2M > calibration_threshold_max)) {
            return 0;
        }
    return 1;
}

/* Takes in COUNTER, which is the number of ticks elapsed since resetting the 2MHz counter.
Adjusts the 2MHz DAC one bit at a time to bring COUNTER closer to 200000
which is the number of ticks of the clock in 100ms.

Returns 0 if no calibration occurs, 1 if the DAC was changed.
*/
int cal_2M_RC(void) {
    int ind;
    int counter_dif;
    //printf("Calibrating 2M oscillator.\n");
    
    /* increment the DAC setting to lower the frequency.*/
    
    // change this condition to match calibration threshold variables defined at the top.
    if (rolling_average_2M > calibration_threshold_max) {
        
        if ((dac_2M_settings[4] < 31) && (modify_superfine)) {
            // superfine
            dac_2M_settings[4] += 1;
        } else if ((dac_2M_settings[3] < 31) && (modify_fine)) {
            // fine
            dac_2M_settings[3] += 1;
        } else if ((dac_2M_settings[2] < 31) && (modify_coarse3)) {
            // coarse 3
            dac_2M_settings[2] += 1;
            // reset fine and superfine so we can modify them again.
            dac_2M_settings[3] = 15;
            dac_2M_settings[4] = 15;
        } else if ((dac_2M_settings[1] < 31) && (modify_coarse2)) {
            // coarse 2
            dac_2M_settings[1] += 1;
        } else if ((dac_2M_settings[0] < 31) && (modify_coarse1)) {
            // coarse 1
            dac_2M_settings[0] += 1;
        } else {
        }

        /* coarse1 -> coarse2 -> coarse3 -> fine -> superfine
        if ((dac_2M_settings[0] < 31) && (modify_coarse1)) {
            // coarse 1
            dac_2M_settings[0] += 1;
        } else if ((dac_2M_settings[1] < 31) && (modify_coarse2)) {
            // coarse 2
            dac_2M_settings[1] += 1;
        } else if ((dac_2M_settings[2] < 31) && (modify_coarse3)) {
            // coarse 3
            dac_2M_settings[2] += 1;
        } else if ((dac_2M_settings[3] < 31) && (modify_fine)) {
            // fine
            dac_2M_settings[3] += 1;
        } else if ((dac_2M_settings[4] < 31) && (modify_superfine)) {
            // superfine
            dac_2M_settings[4] += 1;
        } else {
            // DAC settings are maxed out.
        } */
    }
    
    /* decrement the DAC setting to increase the frequency.*/
    else {
        
        if ((dac_2M_settings[4] > 0) && (modify_superfine)) {
            // superfine
            dac_2M_settings[4] -= 1;
        } else if ((dac_2M_settings[3] > 0) && (modify_fine)) {
            // fine
            dac_2M_settings[3] -= 1;
        } else if ((dac_2M_settings[2] > 0) && (modify_coarse3)) {
            // coarse 3 
            dac_2M_settings[2] -= 1;
            // adjust fine and superfine so we can adjust them again
            dac_2M_settings[3] = 15;
            dac_2M_settings[4] = 15;
        } else if ((dac_2M_settings[1] > 0) && (modify_coarse2)) {
            // coarse 2
            dac_2M_settings[1] -= 1;
        } else if ((dac_2M_settings[0] > 0) && (modify_coarse1)) {
            // start with coarse 1 
            dac_2M_settings[0] -= 1;
        } else {
        }
        
        /* coarse1 -> coarse2 -> coarse3 -> fine -> superfine
        if ((dac_2M_settings[0] > 0) && (modify_coarse1)) {
            // start with coarse 1
            dac_2M_settings[0] -= 1;
        } else if ((dac_2M_settings[1] > 0) && (modify_coarse2)) {
            // coarse 2
            dac_2M_settings[1] -= 1;
        } else if ((dac_2M_settings[2] > 0) && (modify_coarse3)) {
            // coarse 3 
            dac_2M_settings[2] -= 1;
        } else if ((dac_2M_settings[3] > 0) && (modify_fine)) {
            // fine 
            dac_2M_settings[3] -= 1;
        } else if ((dac_2M_settings[4] > 0) && (modify_superfine)) {
            // superfine 
            dac_2M_settings[4] -= 1;
        } else {
            // DAC settings are maxed out. 
        } */
    }
    
    set_2M_RC_frequency(dac_2M_settings[0], dac_2M_settings[1], dac_2M_settings[2], dac_2M_settings[3], dac_2M_settings[4]);
    //printf("Cal\n");
    // print the DAC settings
    print_2MHz_DAC();
    return 1;
}


void LC_FREQCHANGE(char coarse, char mid, char fine){
    
    // mask to ensure that the coarse, mid, and fine are actually 5-bit
    char coarse_m = coarse & 0x1F;
    char mid_m = mid & 0x1F;
    char fine_m = fine & 0x1F;
    char coarse_m2 = 0x0;
    char mid_m2 = 0x0;
    char fine_m2 = 0x0;
    
    // shift bits in
    unsigned int fcode = 0x00000000;
    unsigned int fcode2 = 0x00000000;
    
    // Flip endianness
    coarse_m2 |= (0x01 & coarse_m) << 4;
    coarse_m2 |= (0x02 & coarse_m) << 2;
    coarse_m2 |= (0x04 & coarse_m);
    coarse_m2 |= (0x08 & coarse_m) >> 2;
    coarse_m2 |= (0x10 & coarse_m) >> 4;
    
    mid_m2 |= (0x01 & mid_m) << 4;
    mid_m2 |= (0x02 & mid_m) << 2;
    mid_m2 |= (0x04 & mid_m);
    mid_m2 |= (0x08 & mid_m) >> 2;
    mid_m2 |= (0x10 & mid_m) >> 4;
    
    fine_m2 |= (0x01 & fine_m) << 4;
    fine_m2 |= (0x02 & fine_m) << 2;
    fine_m2 |= (0x04 & fine_m);
    fine_m2 |= (0x08 & fine_m) >> 2;
    fine_m2 |= (0x10 & fine_m) >> 4;
    
    
    fcode |= coarse_m2;
    fcode |= mid_m2 << 6;
    fcode |= (fine_m2 & 0xF) << 12;
    
    fcode2 |= fine_m2 >> 4;
    
    // set the memory and prevent any overwriting of other analog config
    ACFG_LO__ADDR = fcode;
    ACFG_LO__ADDR_2 = fcode2;
    
    // acknowledge that this was run
    //printf("LC frequency set %X\n",fcode);
    //printf("LC frequency set %X\n",fcode2);
    
}

/* Checks the bit position of GP input 12 (which we have connected to the 2 MHz RC output
from GPIO); if high, return true, else return false.*/
bool is_2M_high(void) {
    if ((GPIO_REG__INPUT & (1 << 12)) == 0x1000) {
        return true;
    }
    return false;
}


/* Waits until the 2 MHz RC signal is high before executing function F.*/
void execute_func( void (*f) (void)) {
    while (!is_2M_high()) {
    }
    (*f)();
}

/* disables all counters.*/
void disable_counters(void) {
    // Disable all counters
    ANALOG_CFG_REG__0 = 0x007F;
}


/* reset counters */
void reset_counters(void) {
    // Reset all counters
    ANALOG_CFG_REG__0 = 0x0000;        
}


/* enable counters */
void enable_counters(void) {
    // Enable all counters
    ANALOG_CFG_REG__0 = 0x3FFF;    
}


void read_counters(unsigned int* count_2M, unsigned int* count_LC, unsigned int* count_32k, unsigned int* count_ringDiv8){

    unsigned int rdata_lsb, rdata_msb;
    
    // Disable all counters
    //execute_func(disable_counters);
    // Disable all counters
    ANALOG_CFG_REG__0 = 0x007F;
    
    /*
    // Read 2M counter
    rdata_lsb = *(unsigned int*)(APB_ANALOG_CFG_BASE + 0x180000);
    rdata_msb = *(unsigned int*)(APB_ANALOG_CFG_BASE + 0x1C0000);
    *count_2M = rdata_lsb + (rdata_msb << 16);
    */
    
    /*
    // Read LC_div counter (via counter4)
    rdata_lsb = *(unsigned int*)(APB_ANALOG_CFG_BASE + 0x200000);
    rdata_msb = *(unsigned int*)(APB_ANALOG_CFG_BASE + 0x240000);
    *count_LC = rdata_lsb + (rdata_msb << 16);
        
    // Read 32k counter
    rdata_lsb = *(unsigned int*)(APB_ANALOG_CFG_BASE + 0x000000);
    rdata_msb = *(unsigned int*)(APB_ANALOG_CFG_BASE + 0x040000);
    *count_32k = rdata_lsb + (rdata_msb << 16);
    */
    
    // ring div 8 counter
    //rdata_lsb = *(unsigned int*)(APB_ANALOG_CFG_BASE + 0x200000);
    //rdata_msb = *(unsigned int*)(APB_ANALOG_CFG_BASE + 0x240000);
    //*count_ringDiv8 = rdata_lsb + (rdata_msb << 16);
    
    // Reset all counters
    //execute_func(reset_counters);
    // Reset all counters
    ANALOG_CFG_REG__0 = 0x0000;    

    // Enable all counters
    //execute_func(enable_counters);
    // Enable all counters
    ANALOG_CFG_REG__0 = 0x3FFF;    
    
    //printf("%d ",*count_LC);
    printf("%d\n",*count_2M); //2 MHz counter
    //printf("%d\n",*count_ringDiv8); //RINGDIV8 counter
    //printf("%d",*count_32k); //32khz counter
}

void read_2M_counter(unsigned int* count_2M) {
    unsigned int rdata_lsb, rdata_msb;
    
    // Disable all counters
    ANALOG_CFG_REG__0 = 0x007F;
    
    // Read 2M counter into the count_2M pointer
    rdata_lsb = *(unsigned int*)(APB_ANALOG_CFG_BASE + 0x180000);
    rdata_msb = *(unsigned int*)(APB_ANALOG_CFG_BASE + 0x1C0000);
    *count_2M = rdata_lsb + (rdata_msb << 16);
    
    // Reset all counters
    ANALOG_CFG_REG__0 = 0x0000;        

    // Enable all counters
    ANALOG_CFG_REG__0 = 0x3FFF;    
    
    //printf("%d\n",*count_2M); //2 MHz counter
}


void add_sample_to_buffer(int new_sample, int offset) {
    int pos = offset % 10;
    buffer_samples[pos] = new_sample;
    return;
}


/* Computes the average of samples in buffer_samples.
If any value in the array is 0, it is not counted.

If the rolling average is outside the calibration threshold, we calibrate the dac.
*/
void compute_2M_rolling_average(void) {
    int index;
    double average;
    int sum = 0;
    int curr_sample;
    int num_legit_samples = 0;
    for (index = 0; index < 10; index++) {
        curr_sample = buffer_samples[index];
        if (curr_sample != 0) {
            sum += curr_sample;
            num_legit_samples += 1;
        }
    }
    average = sum / num_legit_samples;
    //printf("2M Av: %f\n", average);
    rolling_average_2M = average;
    
    // calibrate if necessary
    // calibrate dac if rolling average is out of calibration threshold
    if (!is_2M_within_cal_window() && !first_ten) {
        //cal_2M_RC();
    }
}


void initialize_ASC(void){
    
    ANALOG_CFG_REG__11 = 0x0080;
    
    // The meaning of each bit is explained in scm3_ASC_v9.m script

    ASC[0] = 0xFF800000;   //0-31
    ASC[1] = 0x00400000;   //32-63
    ASC[2] = 0x00000000;   //64-95
    ASC[3] = 0x000E0006;   //96-127
    ASC[4] = 0x48000000;   //128-159
    ASC[5] = 0x00000000;   //160-191
    ASC[6] = 0x00001000;   //192-223
    ASC[7] = 0x0003012B;   //224-255
    ASC[8] = 0x3306FC00;   //256-287
    ASC[9] = 0x00422188;   //288-319
    ASC[10] = 0x88040031;   //320-351
    ASC[11] = 0x113F4081;   //352-383
    ASC[12] = 0x027E8102;   //384-415
    ASC[13] = 0x02605844;   //416-447
    ASC[14] = 0x60010000;   //448-479
    ASC[15] = 0x07EF0803;   //480-511
    ASC[16] = 0x00000000;   //512-543
    ASC[17] = 0x00000000;   //544-575
    ASC[18] = 0x00000000;   //576-607
    ASC[19] = 0x00000000;   //608-639
    ASC[20] = 0x00000000;   //640-671
    ASC[21] = 0x00000000;   //672-703
    ASC[22] = 0x00000000;   //704-735
    ASC[23] = 0x00000000;   //736-767
    ASC[24] = 0x00007CFC;   //768-799
    ASC[25] = 0x20000000;   //800-831
    ASC[26] = 0x00000000;   //832-863
    ASC[27] = 0x00000000;   //864-895
    ASC[28] = 0x00000810;   //896-927
    ASC[29] = 0x007824DB;   //928-959
    ASC[30] = 0x48000807;   //960-991
    ASC[31] = 0xF0300805;   //992-1023
    ASC[32] = 0x7028E07E;   //1024-1055
    ASC[33] = 0x19A4B0C4;   //1056-1087
    ASC[34] = 0x7FFF6840;   //1088-1119
    ASC[35] = 0x00078000;   //1120-1151
    ASC[36] = 0x08000000;   //1152-1183
    ASC[37] = 0x00000000;
    
    analog_scan_chain_write(&ASC[0]);
    analog_scan_chain_load();
}


void update_PN31_byte(unsigned int* current_lfsr){
    int i;
    
    for(i=0; i<8; i++){
        int newbit = (((*current_lfsr >> 30) ^ (*current_lfsr >> 27)) & 1);
        *current_lfsr = ((*current_lfsr << 1) | newbit);    
    }
}


void TX_load_PN_data(unsigned int num_bytes){
    int i;
    for(i=0; i<num_bytes; i++){
        send_packet[i] = (char)(current_lfsr & 0xFF);
    }
    
    RFCONTROLLER_REG__TX_PACK_LEN = num_bytes;
    RFCONTROLLER_REG__CONTROL = 0x1; // "lod"    
}

void TX_load_counter_data(unsigned int num_bytes){
    int i;
    for(i=0; i<num_bytes; i++){
        send_packet[i] = (char)(0x30 + i);
    }
    
    RFCONTROLLER_REG__TX_PACK_LEN = num_bytes;    
    RFCONTROLLER_REG__CONTROL = 0x1; // "lod"
}




