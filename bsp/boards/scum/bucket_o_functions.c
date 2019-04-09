#include <stdio.h>
#include <stdlib.h>
#include "Memory_Map.h"

extern unsigned int ASC[38]; // initialize the scan chain in memory


// -----------------------------------------------
// scan chain setting modifications
// -----------------------------------------------

void LC_FREQCHANGE(int coarse, int mid, int fine){

    //   Inputs:
    //      coarse: 5-bit code (0-31) to control the ~15 MHz step frequency DAC
    //      mid: 5-bit code (0-31) to control the ~800 kHz step frequency DAC
    //      fine: 5-bit code (0-31) to control the ~100 kHz step frequency DAC
    //  Outputs:
    //      none, it programs the LC radio frequency immediately
    
    // mask to ensure that the coarse, mid, and fine are actually 5-bit
    char coarse_m = (char)(coarse & 0x1F);
    char mid_m = (char)(mid & 0x1F);
    char fine_m = (char)(fine & 0x1F);

    // flip the bit order to make it fit more easily into the ACFG registers
    unsigned int coarse_f = (unsigned int)(flipChar(coarse_m));
    unsigned int mid_f = (unsigned int)(flipChar(mid_m));
    unsigned int fine_f = (unsigned int)(flipChar(fine_m));

    // initialize registers
    unsigned int fcode = 0x00000000;   // contains everything but LSB of the fine DAC
    unsigned int fcode2 = 0x00000000;  // contains the LSB of the fine DAC
   
    fine_f &= 0x000000FF;
    mid_f &= 0x000000FF;
    coarse_f &= 0x000000FF;

    //printf("%d\n",fine_m);
    //printf("%d\n",mid_m);
    //printf("%d\n",coarse_m);
       
    fcode |= (unsigned int)((fine_f & 0x78) << 9);
    fcode |= (unsigned int)(mid_f << 3);
    fcode |= (unsigned int)(coarse_f >> 3);

    fcode2 |= (unsigned int)((fine_f&0x80) >> 7);
    
    //printf("%X\n",fcode);
    //printf("%X\n",fcode2);
    
    // ACFG_LO_ADDR   = [ f1 | f2 | f3 | f4 | md | m0 | m1 | m2 | m3 | m4 | cd | c0 | c1 | c2 | c3 | c4 ]
    // ACFG_LO_ADDR_2 = [ xx | xx | xx | xx | xx | xx | xx | xx | xx | xx | xx | xx | xx | xx | fd | f0 ]
    
    // set the memory and prevent any overwriting of other analog config
    ACFG_LO__ADDR = fcode;
    ACFG_LO__ADDR_2 = fcode2;
    
}
void LC_monotonic(int LC_code){

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
   
   LC_FREQCHANGE(coarse,mid,fine);
   
}


void disable_polyphase_ASC() {
   ASC[30] &= 0xFFEFFFFF;
}

void enable_polyphase_ASC() {
   ASC[30] |= 0x00100000;
}

void disable_div_power_ASC() {
   ASC[16] &= 0xB7FFFFFF;
}
void enable_div_power_ASC() {
   ASC[16] |= 0x48000000;
}

void ext_clk_ble_ASC() {
   ASC[32] |= 0x00080000;
}
void int_clk_ble_ASC() {
   ASC[32] &= 0xFFF7FFFF;
}


void enable_1mhz_ble_ASC() {
   ASC[32] &= 0xFFF9FFFF;
}

void disable_1mhz_ble_ASC() {
   ASC[32] |= 0x00060000;
}

void set_LC_current(unsigned int current) {
   unsigned int current_msb = (current & 0x000000F0) >> 4;
   unsigned int current_lsb = (current & 0x0000000F) << 28;
   
   ASC[30] &= 0xFFFFFFF0;
   ASC[30] |= current_msb;
   
   ASC[31] &= 0x0FFFFFFF;
   ASC[31] |= current_lsb;
}
void set_PA_supply(unsigned int code) {
   // 7-bit setting (between 0 and 127)
   // MSB is a "panic" bit that engages the high-voltage settings
   unsigned int code_ASC = ((~code)&0x0000007F) << 13;
   ASC[30] &= 0xFFF01FFF;
   ASC[30] |= code_ASC;
   
}
void set_LO_supply(unsigned int code, unsigned char panic) {
   // 7-bit setting (between 0 and 127)
   // MSB is a "panic" bit that engages the high-voltage settings
   unsigned int code_ASC = ((~code)&0x0000007F) << 5;
   ASC[30] &= 0xFFFFF017;
   ASC[30] |= code_ASC;
}
void set_DIV_supply(unsigned int code, unsigned char panic) {
   // 7-bit setting (between 0 and 127)
   // MSB is a "panic" bit that engages the high-voltage settings
   unsigned int code_ASC = ((~code)&0x0000007F) << 5;
   ASC[30] &= 0xFFF01FFF;
   ASC[30] |= code_ASC;
}

void prescaler(int code) {
   // code is a number between 0 and 5
   // 0 -> disable pre-scaler entirely
   // 1 -> enable div-by-5 back-up pre-scaler
   // 2 -> enable div-by-2 back-up pre-scaler
   // 3 -> enable dynamic pre-scaler version 1 (div-by-5, strong)
   // 4 -> enable dynamic pre-scaler version 2 (div-by-2, strong)
   // 5 -> enable dynamic pre-scaler version 3 (div-by-5, weak)
   
   if (code == 0) {
      // disable div-by-5 backup, disable div-by-2 backup, disable dynamic pre-scaler
      ASC[31] |= 0x00000004;
      ASC[31] &= 0xFFFFFFFD; // disable div-by-5 backup
      ASC[32] |= 0x80000000;
      ASC[31] &= 0xFFFFFFFE; // disable div-by-2 backup
      ASC[32] |= 0x70000000; // disable all of the dynamic pre-scalers
   }
   else if (code == 1) {
      // enable div-by-5 backup, disable div-by-2 backup, disable dynamic pre-scaler
      ASC[31] |= 0x00000002;
      ASC[31] &= 0xFFFFFFFB; // enable div-by-5 backup
      ASC[32] |= 0x80000000;
      ASC[31] &= 0xFFFFFFFE; // disable div-by-2 backup
      ASC[32] |= 0x70000000; // disable all of the dynamic pre-scalers
   }
   else if (code == 2) {
      // disable div-by-5 backup, enable div-by-2 backup, disable dynamic pre-scaler
      ASC[31] |= 0x00000004;
      ASC[31] &= 0xFFFFFFFD; // disable div-by-5 backup
      ASC[32] &= 0x7FFFFFFF;
      ASC[31] |= 0x00000001; // enable div-by-2 backup
      ASC[32] |= 0x70000000; // disable all of the dynamic pre-scalers
   }
   else if (code == 3) {
      // disable div-by-5 backup, disable div-by-2 backup, enable setting #1 of dynamic pre-scaler
      ASC[31] |= 0x00000004;
      ASC[31] &= 0xFFFFFFFD; // disable div-by-5 backup
      ASC[32] |= 0x80000000;
      ASC[31] &= 0xFFFFFFFE; // disable div-by-2 backup
      ASC[32] &= 0xBFFFFFFF; // enable first bit of pre-scaler
   }
   else if (code == 4) {
      // disable div-by-5 backup, disable div-by-2 backup, enable setting #2 of dynamic pre-scaler
      ASC[31] |= 0x00000004;
      ASC[31] &= 0xFFFFFFFD; // disable div-by-5 backup
      ASC[32] |= 0x80000000;
      ASC[31] &= 0xFFFFFFFE; // disable div-by-2 backup
      ASC[32] &= 0xDFFFFFFF; // enable second bit of pre-scaler
   }
   else if (code == 5) {
      // disable div-by-5 backup, disable div-by-2 backup, enable setting #3 of dynamic pre-scaler
      ASC[31] |= 0x00000004;
      ASC[31] &= 0xFFFFFFFD; // disable div-by-5 backup
      ASC[32] |= 0x80000000;
      ASC[31] &= 0xFFFFFFFE; // disable div-by-2 backup
      ASC[32] &= 0x9FFFFFFF; // enable third bit of pre-scaler
   }
}
