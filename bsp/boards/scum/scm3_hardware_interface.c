#include <stdio.h>
#include "Memory_Map.h"
#include "bucket_o_functions.h"

unsigned int ASC[38] = {0};
unsigned int ASC_FPGA[38] = {0};
        char send_packet[127];

unsigned int current_lfsr = 0x12345678;

// coarse1, coarse2, coarse3, fine, superfine dac settings
unsigned int dac_2M_settings[5] = {31, 31, 29, 2, 2};

unsigned int RX_channel_codes[16] = {0};
unsigned int TX_channel_codes[16] = {0};


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

unsigned char flipChar(unsigned char b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

void set_asc_bit(unsigned int position){

    unsigned int index;
    
    index = position >> 5;
    
    ASC[index] |= 0x80000000 >> (position - (index << 5));
}

void clear_asc_bit(unsigned int position){

    unsigned int index;
    
    index = position >> 5;
    
    ASC[index] &= ~(0x80000000 >> (position - (index << 5)));
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


void LC_FREQCHANGE_ASC(int coarse, int mid, int fine){
   
   //   Inputs:
   //      coarse: 5-bit code (0-31) to control the ~15 MHz step frequency DAC
   //      mid: 5-bit code (0-31) to control the ~800 kHz step frequency DAC
   //      fine: 5-bit code (0-31) to control the ~100 kHz step frequency DAC
   //  Outputs:
   //      none; need to program ASC
    
  // mask to ensure that the coarse, mid, and fine are actually 5-bit
  char coarse_m = (char)(coarse & 0x1F);
  char mid_m = (char)(mid & 0x1F);
  char fine_m = (char)(fine & 0x1F);
   
   unsigned int j;

   //fine_tune 6 946-950 MSB:LSB+dummy
   //mid_tune 6 952-956 MSB:LSB+dummy
   //coarse_tune 6 958-962 MSB:LSB+dummy
   
   // Set tuning control to ASC
   set_asc_bit(964);
   
   // Set fine bits
   for(j=0; j<5; j++){
      if((fine_m >> j) & 0x1)
         set_asc_bit(950-j);
      else
         clear_asc_bit(950-j);
   }
   
   // Set mid bits
   for(j=0; j<5; j++){
      if((mid_m >> j) & 0x1)
         set_asc_bit(956-j);
      else
         clear_asc_bit(956-j);
   }   
   
   // Set coarse bits
   for(j=0; j<5; j++){
      if((coarse_m >> j) & 0x1)
         set_asc_bit(962-j);
      else
         clear_asc_bit(962-j);
   }   
}


void LC_monotonic_ASC(int LC_code, unsigned int type){

   //int coarse_divs = 440;
   //int mid_divs = 31; // For full fine code sweeps
   
   int fine_fix = 0;
   int mid_fix = 0;
   //int coarse_divs = 136;
   int mid_divs = 25; // works for Ioana's board, Fil's board, Brad's other board
   
   //int coarse_divs = 167;
      int coarse_divs = 155;
   //int mid_divs = 27; // works for Brad's board // 25 and 155 worked really well @ low frequency, 27 167 worked great @ high frequency (Brad's board)
   
   int mid;
   int fine;
   int coarse = (((LC_code/coarse_divs + 19) & 0x000000FF));
   
   LC_code = LC_code % coarse_divs;
   //mid = ((((LC_code/mid_divs)*4 + mid_fix) & 0x000000FF)); // works for boards (a)
    mid = ((((LC_code/mid_divs)*3 + mid_fix) & 0x000000FF));
   //mid = ((((LC_code/mid_divs) + mid_fix) & 0x000000FF));
   if (LC_code/mid_divs >= 2) {fine_fix = 0;};
   fine = (((LC_code % mid_divs + fine_fix) & 0x000000FF));
   if (fine > 15){fine++;};
   
   if (type==1) {
      mid = mid-1;
      fine = 5;
   }
   
   LC_FREQCHANGE_ASC(coarse,mid,fine);
   
//   printf("coarse=%d,mid=%d,fine=%d\r\n",coarse,mid,fine);
   
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


// Set frequency for TI 20M oscillator
void set_sys_clk_secondary_freq(unsigned int coarse, unsigned int fine){
   //coarse 0:4 = 860 861 875b 876b 877b
   //fine 0:4 870 871 872 873 874b
   
   int j;
   
   for(j=0; j<=3; j++){
      if((fine >> j) & 0x1)
         set_asc_bit(870+j);
      else
         clear_asc_bit(870+j);
   }   
   if((fine >> 4) & 0x1)
         clear_asc_bit(874);
      else
         set_asc_bit(874);
   
   
      for(j=0; j<=1; j++){
         if((coarse >> j) & 0x1)
            set_asc_bit(860+j);
         else
            clear_asc_bit(860+j);
      }
      for(j=2; j<=4; j++){
         if((coarse >> j) & 0x1)
            clear_asc_bit(873+j);
         else
            set_asc_bit(873+j);
   }         
}


void init_ldo_control(void){
    
    // Analog scan chain setup for radio LDOs
    clear_asc_bit(501); // = scan_pon_if
    clear_asc_bit(502); // = scan_pon_lo
    clear_asc_bit(503); // = scan_pon_pa
    set_asc_bit(504); // = gpio_pon_en_if
    clear_asc_bit(505); // = fsm_pon_en_if
    set_asc_bit(506); // = gpio_pon_en_lo
    clear_asc_bit(507); // = fsm_pon_en_lo 
    clear_asc_bit(508); // = gpio_pon_en_pa
    clear_asc_bit(509); // = fsm_pon_en_pa
    set_asc_bit(510); // = master_ldo_en_if
    set_asc_bit(511); // = master_ldo_en_lo
    set_asc_bit(512); // = master_ldo_en_pa
    clear_asc_bit(513); // = scan_pon_div
    set_asc_bit(514); // = gpio_pon_en_div
    clear_asc_bit(515); // = fsm_pon_en_div
    set_asc_bit(516); // = master_ldo_en_div
}

void GPO_control(unsigned char row1, unsigned char row2, unsigned char row3, unsigned char row4) {
    
    int j;
    
    for (j = 0; j <= 3; j++) { 
    
        if((row1 >> j) & 0x1)
            set_asc_bit(245+j);
        else    
            clear_asc_bit(245+j);
    }
    
    for (j = 0; j <= 3; j++) { 
    
        if((row2 >> j) & 0x1)
            set_asc_bit(249+j);
        else    
            clear_asc_bit(249+j);
    }
    
    for (j = 0; j <= 3; j++) { 
    
        if((row3 >> j) & 0x1)
            set_asc_bit(253+j);
        else    
            clear_asc_bit(253+j);
    }
    
    for (j = 0; j <= 3; j++) { 
    
        if((row4 >> j) & 0x1)
            set_asc_bit(257+j);
        else    
            clear_asc_bit(257+j);
    }
}

void GPI_control(unsigned char row1, unsigned char row2, unsigned char row3, unsigned char row4) {
    
    int j;
    
    for (j = 0; j <= 1; j++) { 
        if((row1 >> j) & 0x1)
            set_asc_bit(261+j);
        else    
            clear_asc_bit(261+j);
    }
    
    for (j = 0; j <= 1; j++) { 
        if((row2 >> j) & 0x1)
            set_asc_bit(263+j);
        else    
            clear_asc_bit(263+j);
    }
    
    for (j = 0; j <= 1; j++) { 
        if((row3 >> j) & 0x1)
            set_asc_bit(265+j);
        else    
            clear_asc_bit(265+j);
    }
    
    for (j = 0; j <= 1; j++) { 
        if((row4 >> j) & 0x1)
            set_asc_bit(267+j);
        else    
            clear_asc_bit(267+j);
    }
}


// Enable output drivers for GPIO based on 'mask'
// '1' = output enabled, so GPO_enables(0xFFFF) enables all output drivers
// GPO enables are active low on-chip
void GPO_enables(unsigned int mask){

    //out_en<0:15> = ASC<1131>,ASC<1133>,ASC<1135>,ASC<1137>,ASC<1140>,ASC<1142>,ASC<1144>,ASC<1146>,...
    //ASC<1115>,ASC<1117>,ASC<1119>,ASC<1121>,ASC<1124>,ASC<1126>,ASC<1128>,ASC<1130>    
    unsigned short asc_locations[16] = {1131,1133,1135,1137,1140,1142,1144,1146,1115,1117,1119,1121,1124,1126,1128,1130};
    unsigned int j;
    
    for (j = 0; j <= 15; j++) { 
    
        if((mask >> j) & 0x1)
            clear_asc_bit(asc_locations[j]);
        else    
            set_asc_bit(asc_locations[j]);
    }
}

// Enable input path for GPIO based on 'mask'
// '1' = input enabled, so GPI_enables(0xFFFF) enables all inputs
// GPI enables are active high on-chip
void GPI_enables(unsigned int mask){

    //in_en<0:15> = ASC<1132>,ASC<1134>,ASC<1136>,ASC<1138>,ASC<1139>,ASC<1141>,ASC<1143>,ASC<1145>,...
    //ASC<1116>,ASC<1118>,ASC<1120>,ASC<1122>,ASC<1123>,ASC<1125>,ASC<1127>,ASC<1129>    
    unsigned short asc_locations[16] = {1132,1134,1136,1138,1139,1141,1143,1145,1116,1118,1120,1122,1123,1125,1127,1129};
    unsigned int j;
    
    for (j = 0; j <= 15; j++) { 
    
        if((mask >> j) & 0x1)
            set_asc_bit(asc_locations[j]);
        else    
            clear_asc_bit(asc_locations[j]);
    }
}

// FPGA

void set_asc_bit_FPGA(unsigned int position){

    unsigned int index;
    
    index = position >> 5;
    
    ASC_FPGA[index] |= 0x80000000 >> (position - (index << 5));
}

void clear_asc_bit_FPGA(unsigned int position){

    unsigned int index;
    
    index = position >> 5;
    
    ASC_FPGA[index] &= ~(0x80000000 >> (position - (index << 5)));
}

void GPO_control_FPGA(unsigned char row1, unsigned char row2, unsigned char row3, unsigned char row4) {
    
    int j;
    
    for (j = 0; j <= 3; j++) { 
    
        if((row1 >> j) & 0x1)
            set_asc_bit_FPGA(245+j);
        else    
            clear_asc_bit_FPGA(245+j);
    }
    
    for (j = 0; j <= 3; j++) { 
    
        if((row2 >> j) & 0x1)
            set_asc_bit_FPGA(249+j);
        else    
            clear_asc_bit_FPGA(249+j);
    }
    
    for (j = 0; j <= 3; j++) { 
    
        if((row3 >> j) & 0x1)
            set_asc_bit_FPGA(253+j);
        else    
            clear_asc_bit_FPGA(253+j);
    }
    
    for (j = 0; j <= 3; j++) { 
    
        if((row4 >> j) & 0x1)
            set_asc_bit_FPGA(257+j);
        else    
            clear_asc_bit_FPGA(257+j);
    }
}

void GPI_control_FPGA(unsigned char row1, unsigned char row2, unsigned char row3, unsigned char row4) {
    
    int j;
    
    for (j = 0; j <= 1; j++) { 
    
        if((row1 >> j) & 0x1)
            set_asc_bit_FPGA(261+j);
        else    
            clear_asc_bit_FPGA(261+j);
    }
    
    for (j = 0; j <= 1; j++) { 
    
        if((row2 >> j) & 0x1)
            set_asc_bit_FPGA(263+j);
        else    
            clear_asc_bit_FPGA(263+j);
    }
    
    for (j = 0; j <= 1; j++) { 
    
        if((row3 >> j) & 0x1)
            set_asc_bit_FPGA(265+j);
        else    
            clear_asc_bit_FPGA(265+j);
    }
    
    for (j = 0; j <= 1; j++) { 
    
        if((row4 >> j) & 0x1)
            set_asc_bit_FPGA(267+j);
        else    
            clear_asc_bit_FPGA(267+j);
    }
}

void analog_scan_chain_write_3B_fromFPGA(unsigned int* scan_bits) {
    
    int i = 0;
    int j = 0;
    unsigned int asc_reg;
    
    for (i=37; i>=0; i--) {
        
        //printf("\n%d,%lX\n",i,scan_bits[i]);
        
        for (j=0; j<32; j++) {

        // Set scan_in (should be inverted)
        if((scan_bits[i] & (0x00000001 << j)) == 0)
            asc_reg = 0x1;    
        else
            asc_reg = 0x0;

        // Write asc_reg to analog_cfg
        ANALOG_CFG_REG__5 = asc_reg;

        // Lower phi1
        asc_reg &= ~(0x2);
        ANALOG_CFG_REG__5 = asc_reg;

        // Toggle phi2
        asc_reg |= 0x4;
        ANALOG_CFG_REG__5 = asc_reg;
        asc_reg &= ~(0x4);
        ANALOG_CFG_REG__5 = asc_reg;

        // Raise phi1
        asc_reg |= 0x2;
        ANALOG_CFG_REG__5 = asc_reg;
        
        }    
    }
}

void analog_scan_chain_load_3B_fromFPGA(void) {
    
    // Assert load signal (and cfg<357>)
    ANALOG_CFG_REG__5 = 0x0028;

    // Lower load signal
    ANALOG_CFG_REG__5 = 0x0020;
}


// Change the reference voltage for the IF LDO
// 0 <= code <= 127
void set_IF_LDO_voltage(int code){

    unsigned int j;

    // ASC<492:498> = if_ldo_rdac<0:6> (<0:6(MSB)>)

    for(j=0; j<=6; j++){
        if((code >> j) & 0x1) {
            set_asc_bit(492+j);
        } else {
            clear_asc_bit(492+j);
        }
    }
}


// Untested function
void set_IF_stg3gm_ASC(unsigned int Igm, unsigned int Qgm){
   
   int j;
   
   // Set all bits to zero
   for(j=0; j<13; j++){
      clear_asc_bit(472+j);
      clear_asc_bit(278+j);
   }
   
   // 472:484 = I stg3 gm 13:1
   for(j=0; j<=Igm; j++){
         set_asc_bit(484-j);
   }   
   
   // 278:290 = Q stg3 gm 1:13
   for(j=0; j<=Qgm; j++){
         clear_asc_bit(278+j);
   }
}

// Adjust the comparator offset trim for I channel
// Valid input range 0-31
void set_IF_comparator_trim_I(unsigned int ptrim, unsigned int ntrim){
   
   int j;
   
   // I comparator N side = 452:456 LSB:MSB
   for(j=0; j<=4; j++){
      if((ntrim >> j) & 0x1)
         set_asc_bit(452+j);
      else
         clear_asc_bit(452+j);
   }   
   
   // I comparator P side = 457:461 LSB:MSB
   for(j=0; j<=4; j++){
      if((ptrim >> j) & 0x1)
         set_asc_bit(457+j);
      else
         clear_asc_bit(457+j);
   }   
}

// Adjust the comparator offset trim for Q channel
// Valid input range 0-31
void set_IF_comparator_trim_Q(unsigned int ptrim, unsigned int ntrim){
   
   int j;
   
   // I comparator N side = 340:344 MSB:LSB
   for(j=0; j<=4; j++){
      if((ntrim >> j) & 0x1)
         set_asc_bit(344-j);
      else
         clear_asc_bit(344-+j);
   }   
   
   // I comparator P side = 335:339 MSB:LSB
   for(j=0; j<=4; j++){
      if((ptrim >> j) & 0x1)
         set_asc_bit(339-j);
      else
         clear_asc_bit(339-j);
   }   
}


// Untested function
void set_IF_gain_ASC(unsigned int Igain, unsigned int Qgain){

    int j;

    // 485:490 = I code 0:5
    for(j=0; j<=4; j++){
        if((Igain >> j) & 0x1) {
            set_asc_bit(485+j);
        } else {
            clear_asc_bit(485+j);
        }
    }   

    // 272:277 = Q code 5:0
    for(j=0; j<=5; j++){
        if((Qgain >> j) & 0x1) {
            set_asc_bit(277-j);
        } else {
            clear_asc_bit(277-j);
        }
    }
}


void radio_init_rx_MF(){
   
    //int j;
    unsigned int mask1, mask2;
    unsigned int tau_shift, e_k_shift, correlation_threshold;

    // IF uses ASC<271:500>, mask off outside that range
    mask1 = 0xFFFC0000;
    mask2 = 0x000007FF;
    ASC[8] &= mask1;
    ASC[15] &= mask2;

    // Same settings as used for 122418 ADC data captures
    ASC[8] |= (0x4050FFE0 & ~mask1);    //256-287
    ASC[9] = 0x00422188;   //288-319
    ASC[10] = 0x88040031;   //320-351
    ASC[11] = 0x113B4081;   //352-383
    ASC[12] = 0x027E8102;   //384-415
    ASC[13] = 0x03ED4844;   //416-447
    ASC[14] = 0x60010000;   //448-479
    ASC[15] |= (0xFFE02E03 & ~mask2);   //480-511

    // Set clock mux to internal RC oscillator
    clear_asc_bit(424);
    set_asc_bit(425);

    // Set gain for I and Q
    set_IF_gain_ASC(63,63);

    // Set gm for stg3 ADC drivers
    set_IF_stg3gm_ASC(7, 7); //(I, Q)

    // Set comparator trims
    set_IF_comparator_trim_I(0, 5); //(p,n)
    set_IF_comparator_trim_Q(15, 0); //(p,n)

    // Setup baseband

    // Choose MF demod
    // ASC<0:1> = [0 0]
    clear_asc_bit(0);
    clear_asc_bit(1);

    // IQ source select
    // '0' = from radio
    // '1' = from GPIO
    clear_asc_bit(96);

    // AGC Setup

    // ASC<100> = envelope detector  
    // '0' to choose envelope detector, 
    // '1' chooses original scm3 overload detector
    clear_asc_bit(100);

    // VGA gain select mux {102=MSB, 101=LSB}
    // Chooses the source of gain control signals connected to analog
    // 00 = AGC FSM
    // 01 or 10 = analog_cfg
    // 11 = GPIN
    set_asc_bit(101);
    set_asc_bit(102);

    // Activate TIA only mode
    // '1' = only control gain of TIA
    // '0' = control gain of TIA and stage1/2
    set_asc_bit(97);

    // Memory mapped config registers
    //analog_cfg[239:224]   AGC      {gain_imbalance_select 1, gain_offset 3, vga_ctrl_Q_analogcfg 6, vga_ctrl_I_analogcfg 6}            ANALOG_CFG_REG__14
    //analog_cfg[255:240]   AGC      {envelope_threshold 4, wait_time 12}      ANALOG_CFG_REG__15
    // gain_imbalance_select
    // '0' = subtract 'gain_offset' from Q channel
    // '1' = subtract 'gain_offset' from I channel
    // envelope_threshold = the max-min value of signal that will cause gain reduction
    // wait_time = how long FSM waits for settling before making another adjustment
    ANALOG_CFG_REG__14 = 0x0000;
    ANALOG_CFG_REG__15 = 0xA00F;

    // MF/CDR
    // Choose output polarity of demod
    set_asc_bit(103);

    // CDR feedback parameters
    tau_shift = 11;
    e_k_shift = 2;
    ANALOG_CFG_REG__3 = (tau_shift << 11) | (e_k_shift << 7);

    // Threshold used for packet detection
    correlation_threshold = 14;
    ANALOG_CFG_REG__9 = correlation_threshold;

    // Mux select bits to choose internal demod or external clk/data from gpio
    // '0' = on chip, '1' = external from GPIO
    clear_asc_bit(269);
    clear_asc_bit(270);

    // Set LDO reference voltage
    set_IF_LDO_voltage(0);

    // Set RST_B to analog_cfg[75]
    set_asc_bit(240);

    // Set RST_B = 1 (it is active low)
    //ANALOG_CFG_REG__4 = 0x2800;   

    // Enable the polyphase filter
    // The polyphase is required for RX and is ideally off in TX
    // Changing it requires programming the ASC in between TX and RX modes
    // So for now just leaving it always enabled
    // Note this may trash the TX efficiency
    // And will also significantly affect the frequency change between TX and RX
    // ASC[30] |= 0x00100000;
    set_asc_bit(971);
    
}

void radio_init_tx(){
   
   // Set up 15.4 modulation source
   // ----
   // For FPGA, the TX modulation comes in at the external pad so need to set the mod_logic mux to route this signal for modulation
   // mod_logic<3:0> = ASC<996:999>
   // The two LSBs change the mux from cortex mod source to pad
   // The other bits are used for inverting the modulation bitstream
   // With these settings, the TX should start at +500 kHz above the channel frequency
   // A '1' data bit then causes the TX to decrease in frequency by 1 MHz (this generates proper 15.4 output)
   // If for some reason you wanted to start 500 kHz below the channel and step up by 1 MHz for a '1', then need to change the settings here
   // In the IC version, comment these two lines out (they switch modulation source to the pad)   
   set_asc_bit(997);
   set_asc_bit(996);
   set_asc_bit(998);
   set_asc_bit(999);
   
   // Make sure the BLE modulation mux is not also modulating the BLE DAC at the same time
   // Bit 1013 sets the BLE mod dac to cortex, since we are using the pad for 15.4 here
   // In the IC version, comment this line out (ie, leave the ble mod source as the pad since 15.4 will use the cortex)   
   set_asc_bit(1013);
   // ----

   
   // Set 15.4 modulation tone spacing
   // ----
   // The correct tone spacing is 1 MHz.  This requires adjusting the cap DAC in the TX
   // The settings below are probably close enough
   //mod_15_4_tune<2:0> = ASC<1002:1000>
   set_asc_bit(1000);
   set_asc_bit(1001);
   set_asc_bit(1002);
   
   // set dummy bit to 1
   set_asc_bit(1003);
   // ----



   // If you need to adjust the tone spacing, turn on the LO and PA, and uncomment the lines below one at a time to force the transmitter to each of its two output tones
   // Then adjust mod_15_4_tune until the spacing is close to 1 MHz
   // Note, I haven't tested this
   // -----------------
   // Force TX to output the 'high' FSK tone
   //set_asc_bit(999);
   //clear_asc_bit(998);
   
   // Force TX to output the 'low' FSK tone
   //clear_asc_bit(999);
   //set_asc_bit(998);
   // -----------------

   
   // Need to set analog_cfg<183> to 1 to select 15.4 for chips out
   ANALOG_CFG_REG__11 = 0x0080;
   
   // Set current in LC tank
   set_LC_current(100);
   
   // Set LDO voltages for PA and LO
   set_PA_supply(63);
   set_LO_supply(63,0);
}

void radio_init_divider(unsigned int div_value){

   int j;
   
   // Set divider LDO value to max
   set_DIV_supply(0,0);

   // Disable /5 prescaler
   clear_asc_bit(1023);      //en
   set_asc_bit(1024);   //enb
   
   // Enable /2 prescaler
   set_asc_bit(1022);   //en
   clear_asc_bit(1021);      //enb


   
   
   //pre_dyn 2 0 1 = eb 0 1 2
   //eb2 turns on the other /2 (active low)
   //eb1 and eb0 turn on the other /5 thing --leave these high
   //pre_dyn<5:0> = asc<1030:1025>
   set_asc_bit(1025);
   set_asc_bit(1026);   // set this low to turn on the other /2; must disable all other dividers
   set_asc_bit(1027);
   
   // Activate 8MHz/20MHz output
   set_asc_bit(1033);
   

//   // Enable static divider
//   set_asc_bit(1061);
//   
//   // Set sel12 = 1 (choose whether x2 is active)
//   set_asc_bit(1012);
//      
//   // Set divider controls to ASC
//   set_asc_bit(1081);
//   
//   // Release divider reset
//   set_asc_bit(1062);
//   
//   // Setting static divider N value
//   // div_static_select starts at ASC(1049) and is 18 bits long
//   // [static_code(11:16) static_code(5:10) static_en rstb static_code(1:4)]
//   // The code is also inverted
//   
//   div_value = ~div_value & 0xFFFF;
//   
//   for(j=0; j<=5; j++){
//      if((div_value >> (j+10)) & 0x1)
//         set_asc_bit(1049+j);
//      else
//         clear_asc_bit(1049+j);
//   }
//   
//   for(j=0; j<=5; j++){
//      if((div_value >> (j+4)) & 0x1)
//         set_asc_bit(1054+j);
//      else
//         clear_asc_bit(1054+j);
//   }
//      
//      for(j=0; j<=3; j++){
//      if((div_value >> j) & 0x1)
//         set_asc_bit(1063+j);
//      else
//         clear_asc_bit(1063+j);
//   }
}

// set IF clock frequency
void set_IF_clock_frequency(int coarse, int fine, int high_range){

    //Coarse and fine frequency tune, binary weighted
    //ASC<427:431> = RC_coarse<4:0> (<4(MSB):0>)
    //ASC<433:437> = RC_fine<4:0>   (<4(MSB):0>)

    unsigned int j;

    for(j=0; j<=4; j++){
        if((coarse >> j) & 0x1) {
            set_asc_bit(431-j);
        } else {
            clear_asc_bit(431-j);
        }
    }
    
    for(j=0; j<=4; j++){
        if((fine >> j) & 0x1) {
            set_asc_bit(437-j);
        } else {
            clear_asc_bit(437-j);
        }
    }

    //Switch between high and low speed ranges for IF RC:
    //'1' = high range
    //ASC<726> = RC_high_speed_mode 
    if(high_range==1) {
        set_asc_bit(726);
    } else {
        clear_asc_bit(726);
    }
}

void radio_enable_LO(void){
    
    // Turn on only LO via memory mapped register
    ANALOG_CFG_REG__10 = 0x0008;
}

void radio_disable_all(void){
    
    // Turn off LDOs
    ANALOG_CFG_REG__10 = 0x0000;
}


unsigned int build_RX_channel_table(unsigned int channel_11_LC_code){
    
    unsigned int rdata_lsb,rdata_msb;
    int t,ii=0;
    unsigned int count_LC[16] = {0};
    unsigned int count_targets[17] = {0};
    
    RX_channel_codes[0] = channel_11_LC_code;
    
    //for(ii=0; ii<16; ii++){
    while(ii<16) {
    
        LC_monotonic_ASC(RX_channel_codes[ii],0);
        analog_scan_chain_write_3B_fromFPGA(&ASC[0]);
        analog_scan_chain_load_3B_fromFPGA();
                    
        // Reset all counters
        ANALOG_CFG_REG__0 = 0x0000;
        
        // Enable all counters
        ANALOG_CFG_REG__0 = 0x3FFF;    
        
        // Count for some arbitrary amount of time
        for(t=1; t<16000; t++);
        
        // Disable all counters
        ANALOG_CFG_REG__0 = 0x007F;

        // Read count result
        rdata_lsb = *(unsigned int*)(APB_ANALOG_CFG_BASE + 0x280000);
        rdata_msb = *(unsigned int*)(APB_ANALOG_CFG_BASE + 0x2C0000);
        count_LC[ii] = rdata_lsb + (rdata_msb << 16);
    
        count_targets[ii+1] = ((961+(ii+1)*2) * count_LC[0]) / 961;
        
        // Adjust LC_code to match new target
        if(ii>0){
            
            if(count_LC[ii] < (count_targets[ii] - 20)){
                RX_channel_codes[ii]++;
            }
            else{
                RX_channel_codes[ii+1] = RX_channel_codes[ii] + 40;
                ii++;
            }                    
        }
        
        if(ii==0){
                RX_channel_codes[ii+1] = RX_channel_codes[ii] + 40;
                ii++;
        }
    }
    
//    for(ii=0; ii<16; ii++){
//        printf("RX ch=%d,  count_LC=%d,  count_targets=%d,  RX_channel_codes=%d\r\n",ii+11,count_LC[ii],count_targets[ii],RX_channel_codes[ii]);
//    }
    
    return count_LC[0];
}


void build_TX_channel_table(unsigned int channel_11_LC_code, unsigned int count_LC_RX_ch11){
    
    unsigned int rdata_lsb,rdata_msb;
    int t,ii=0;
    unsigned int count_LC[16] = {0};
    unsigned int count_targets[17] = {0};
    
    unsigned short nums[16] = {802,904,929,269,949,434,369,578,455,970,139,297,587,109,373,159};
    unsigned short dens[16] = {801,901,924,267,940,429,364,569,447,951,136,290,572,106,362,154};    
        
    
    // Need to adjust here for shift from PA
    TX_channel_codes[0] = channel_11_LC_code+30;
    
    
    //for(ii=0; ii<16; ii++){
    while(ii<16) {
    
        LC_monotonic_ASC(TX_channel_codes[ii],1);
        analog_scan_chain_write_3B_fromFPGA(&ASC[0]);
        analog_scan_chain_load_3B_fromFPGA();
                    
        // Reset all counters
        ANALOG_CFG_REG__0 = 0x0000;
        
        // Enable all counters
        ANALOG_CFG_REG__0 = 0x3FFF;    
        
        // Count for some arbitrary amount of time
        for(t=1; t<16000; t++);
        
        // Disable all counters
        ANALOG_CFG_REG__0 = 0x007F;

        // Read count result
        rdata_lsb = *(unsigned int*)(APB_ANALOG_CFG_BASE + 0x280000);
        rdata_msb = *(unsigned int*)(APB_ANALOG_CFG_BASE + 0x2C0000);
        count_LC[ii] = rdata_lsb + (rdata_msb << 16);
        
        // Until figure out why modulation spacing is only 800kHz, only set 400khz above RF channel
        count_targets[ii] = (nums[ii] * count_LC_RX_ch11) / dens[ii];
        //count_targets[ii] = ((24054 + ii*50) * count_LC_RX_ch11) / 24025;
        //count_targets[ii] = ((24055 + ii*50) * count_LC_RX_ch11) / 24025;
        
        if(count_LC[ii] < (count_targets[ii] - 5)){
            TX_channel_codes[ii]++;
        }
        else{
            TX_channel_codes[ii+1] = TX_channel_codes[ii] + 40;
            ii++;
        }                    
    }
    
//    for(ii=0; ii<16; ii++){
//        printf("\r\nTX ch=%d,  count_LC=%d,  count_targets=%d,  TX_channel_codes=%d\r\n",ii+11,count_LC[ii],count_targets[ii],TX_channel_codes[ii]);
//    }
}

void build_channel_table(unsigned int channel_11_LC_code){
    
    unsigned int count_LC_RX_ch11;

    // Make sure in RX mode first

    count_LC_RX_ch11 = build_RX_channel_table(channel_11_LC_code);

    //printf("--\n");

    // Switch over to TX mode

    // Turn polyphase off for TX
    clear_asc_bit(971);

    // Hi-Z mixer wells for TX
    set_asc_bit(298);
    set_asc_bit(307);

    // Analog scan chain setup for radio LDOs for RX
    clear_asc_bit(504); // = gpio_pon_en_if
    set_asc_bit(506); // = gpio_pon_en_lo
    set_asc_bit(508); // = gpio_pon_en_pa

    build_TX_channel_table(channel_11_LC_code,count_LC_RX_ch11);
    
    radio_disable_all();
}

unsigned int read_IF_estimate(void){
    // Check valid flag
    if(ANALOG_CFG_REG__16 & 0x400) {
        return ANALOG_CFG_REG__16 & 0x3FF;
    } else {
        return 0;
    }
}

