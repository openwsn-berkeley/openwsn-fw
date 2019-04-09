/**
\brief SCuM-specific definition of the "board" bsp module.

\author Tengfei Chang <tengfei.chang@inria.fr>, August 2016.
*/

#include "memory_map.h"
#include "board.h"
#include "debugpins.h"
// bsp modules
#include "leds.h"
#include "uart.h"
#include "radio.h"
#include "eui64.h"
#include "sctimer.h"
#include "scm3_hardware_interface.h"
//=========================== definition ======================================

//=========================== variables =======================================

extern char send_packet[127];
extern unsigned int ASC[38];
extern unsigned int ASC_FPGA[38];

// Bootloader will insert length and pre-calculated CRC at these memory addresses    
#define crc_value         (*((unsigned int *) 0x0000FFFC))
#define code_length       (*((unsigned int *) 0x0000FFF8))

// LC freq = 2.405G
unsigned int LC_target = 2001200; //2002083;//2004000;//2004000; //801900; 
unsigned int LC_code = 360;

unsigned int HF_CLOCK_fine = 15;
unsigned int HF_CLOCK_coarse = 5;
unsigned int RC2M_superfine = 16;

// MF IF clock settings
unsigned int IF_clk_target = 1600000;
unsigned int IF_coarse = 22;
unsigned int IF_fine = 17;

unsigned int cal_iteration = 0;
unsigned int run_test_flag = 0;
unsigned int num_packets_to_test = 1;

unsigned short optical_cal_iteration = 0, optical_cal_finished = 0;

unsigned short doing_initial_packet_search;
unsigned short current_RF_channel;
unsigned short do_debug_print = 0;

//=========================== prototypes ======================================

unsigned reverse(unsigned x);
unsigned int crc32c(unsigned char *message, unsigned int length);

void init_ldo_control(void);

void set_asc_bit(unsigned int position);
void clear_asc_bit(unsigned int position);
void GPO_control(unsigned char row1, unsigned char row2, unsigned char row3, unsigned char row4);
void GPI_control(unsigned char row1, unsigned char row2, unsigned char row3, unsigned char row4);
void GPO_enables(unsigned int mask);
void GPI_enables(unsigned int mask);

void set_asc_bit_FPGA(unsigned int position);
void clear_asc_bit_FPGA(unsigned int position);
void GPO_control_FPGA(unsigned char row1, unsigned char row2, unsigned char row3, unsigned char row4);
void GPI_control_FPGA(unsigned char row1, unsigned char row2, unsigned char row3, unsigned char row4);

void analog_scan_chain_write_3B_fromFPGA(unsigned int* scan_bits);
void analog_scan_chain_load_3B_fromFPGA(void);

//=========================== main ============================================

extern int mote_main(void);

int main(void) {
    return mote_main();
}

//=========================== public ==========================================

void board_init(void) {
    uint8_t eui[8];
    uint8_t calc_crc;
    uint8_t i;
    
    // crc checking
    
//    calc_crc = crc32c(0x0000,code_length);

//    if(calc_crc != crc_value){
//        printf("failed\r\n");
//        while(1);
//    } else {
//        printf("success\r\n");
//    }
    
    // SCM3B Analog Scan Chain Initialization
    
    // Init LDO control - RX enabled by GPIO_PON
    init_ldo_control();
    
    // the following gpio control is for the chip, it does't work with current scum3b
    
    // Init GPIO - for access to all clocks
    // Select input banks for GPIO_PON and interrupts
    GPI_control(1,0,1,0);
    
    // Setup access to clocks for cal
    // Need to keep first row set to bank 0 for optical bootload to keep working
    GPO_control(0,10,8,10);
    
    // Only GPIO_PON is an input
    GPI_enables(0x0002);
    GPO_enables(0xFFFD);
    
    // Program analog scan chain on SCM3B
    analog_scan_chain_write_3B_fromFPGA(&ASC[0]);
    analog_scan_chain_load_3B_fromFPGA();
    
    // FPGA Analog Scan Chain Initialization

    // Copy settings from 3B ASC
    for(i=0;i<=38;i++){
        ASC_FPGA[i] = ASC[i];
    }

    // Set up GPIO settings for FPGA
    // There are no direction controls on FPGA; hard-wired
    
    // the following gpio control will be set on the FPGA
    // all the 16 pins are controlled as GPO: check the SCuM3B final sheet
    GPO_control_FPGA(6,6,6,6);
    
    // Program analog scan chain on FPGA
    analog_scan_chain_write(&ASC_FPGA[0]);
    analog_scan_chain_load();
    
    // set priority of interrupts
    
    IPR0 = 0xFF;    // uart has lowest priority
    IPR6 = 0x0F;    // priority for radio
    IPR7 = 0x00;    // priority for rf_timer
    
    // initialize bsp modules
    debugpins_init();
    leds_init();
    uart_init();
//    sctimer_init();
//    radio_init();
//    eui64_get(eui);
}

void board_sleep(void) {
    // not sure how to enter a sleep mode
}

void board_reset(void) {
    // not sure how the reset is triggered
}

//=========================== private =========================================

// Reverses (reflects) bits in a 32-bit word.
unsigned reverse(unsigned x) {
   x = ((x & 0x55555555) <<  1) | ((x >>  1) & 0x55555555);
   x = ((x & 0x33333333) <<  2) | ((x >>  2) & 0x33333333);
   x = ((x & 0x0F0F0F0F) <<  4) | ((x >>  4) & 0x0F0F0F0F);
   x = (x << 24) | ((x & 0xFF00) << 8) |
       ((x >> 8) & 0xFF00) | (x >> 24);
   return x;
}

// Computes 32-bit crc from a starting address over 'length' dwords
unsigned int crc32c(unsigned char *message, unsigned int length) {
    int i, j;
    unsigned int byte, crc;
    
    i = 0;
    crc = 0xFFFFFFFF;
    while (i < length) {
        byte = message[i];            // Get next byte.
        byte = reverse(byte);         // 32-bit reversal.
        for (j = 0; j <= 7; j++) {    // Do eight times.
            if ((int)(crc ^ byte) < 0){
              crc = (crc << 1) ^ 0x04C11DB7;
            } else {
                crc = crc << 1;
            }
            byte = byte << 1;          // Ready next msg bit.
        }
        i = i + 1;
    }
    return reverse(~crc);
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
