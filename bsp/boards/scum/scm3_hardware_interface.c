#include <stdio.h>
#include "Memory_Map.h"

unsigned int ASC[38] = {0};
unsigned int ASC_FPGA[38] = {0};
        char send_packet[127];

unsigned int current_lfsr = 0x12345678;

// coarse1, coarse2, coarse3, fine, superfine dac settings
unsigned int dac_2M_settings[5] = {31, 31, 29, 2, 2};


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

void analog_scan_chain_load() {
    
    // Assert load signal (and cfg<357>)
    ANALOG_CFG_REG__22 = 0x0028;

    // Lower load signal
    ANALOG_CFG_REG__22 = 0x0020;

}
/* sets the 2 MHz RC DAC frequency. 
-updates the local dac settings array
-flips endianness and sets the appropriate bits in the scanchain array
-writes it to the scanchain
-loads the scanchain
*/
void set_2M_RC_frequency(int coarse1, int coarse2, int coarse3, int fine, int superfine) {
    
    unsigned int newval;
    unsigned int newcoarse1, newcoarse2, newcoarse3, newfine, newsuperfine;
    
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

    //write to analog scanchain and load
    //analog_scan_chain_write(&ASC[0]);
    //analog_scan_chain_load();
    
    //print_2MHz_DAC();
}


/* Initializes the 2MHz DAC with values set in the dac_2M_settings array. */
void initialize_2M_DAC(void) {
    set_2M_RC_frequency(dac_2M_settings[0], dac_2M_settings[1], dac_2M_settings[2], dac_2M_settings[3], dac_2M_settings[4]);
    // printf("Initialized 2MHz DAC\n");
    // print_2MHz_DAC();
}

void read_counters(unsigned int* count_2M, unsigned int* count_LC, unsigned int* count_32k){

    unsigned int rdata_lsb, rdata_msb;//, count_LC, count_32k;
    
    // Disable all counters
    ANALOG_CFG_REG__0 = 0x007F;
        
    // Read 2M counter
    rdata_lsb = *(unsigned int*)(APB_ANALOG_CFG_BASE + 0x180000);
    rdata_msb = *(unsigned int*)(APB_ANALOG_CFG_BASE + 0x1C0000);
    *count_2M = rdata_lsb + (rdata_msb << 16);
        
    // Read LC_div counter (via counter4)
    rdata_lsb = *(unsigned int*)(APB_ANALOG_CFG_BASE + 0x200000);
    rdata_msb = *(unsigned int*)(APB_ANALOG_CFG_BASE + 0x240000);
    *count_LC = rdata_lsb + (rdata_msb << 16);
        
    // Read 32k counter
    rdata_lsb = *(unsigned int*)(APB_ANALOG_CFG_BASE + 0x000000);
    rdata_msb = *(unsigned int*)(APB_ANALOG_CFG_BASE + 0x040000);
    *count_32k = rdata_lsb + (rdata_msb << 16);
    
    // Reset all counters
    ANALOG_CFG_REG__0 = 0x0000;        
    
    // Enable all counters
    ANALOG_CFG_REG__0 = 0x3FFF;    
    
    //printf("LC_count=%X\n",count_LC);
    //printf("2M_count=%X\n",count_2M);
    //printf("32k_count=%X\n\n",count_32k);

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
    
    RFCONTROLLER_REG__TX_PACK_LEN = num_bytes + 2;
    RFCONTROLLER_REG__CONTROL = 0x1; // "lod"    
}

void TX_load_counter_data(unsigned int num_bytes){
    int i;
    for(i=0; i<num_bytes; i++){
        send_packet[i] = (char)(0x30 + i);
    }
    
    RFCONTROLLER_REG__TX_PACK_LEN = num_bytes + 2;    
    RFCONTROLLER_REG__CONTROL = 0x1; // "lod"
}
