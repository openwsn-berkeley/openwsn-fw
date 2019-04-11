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

extern unsigned int RX_channel_codes[16];
extern unsigned int TX_channel_codes[16];

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

void board_optical_calibration(void);
void build_channel_table(unsigned int channel_11_LC_code);

//=========================== interrupt ========================================

void optical_sfd_isr(void);

//=========================== main =============================================

extern int mote_main(void);

int main(void) {
    return mote_main();
}

//=========================== public ==========================================

void board_init(void) {
    uint8_t eui[8];
    uint8_t calc_crc;
    bool    optical_cal_finished;
    
    board_optical_calibration();
    
    // measured in SwarmLab with room temperature
    RX_channel_codes[0] = 360;
    TX_channel_codes[0] = 410;
    
    // set priority of interrupts
    
    IPR0 = 0xFF;    // uart has lowest priority
    IPR6 = 0x0F;    // priority for radio
    IPR7 = 0x00;    // priority for rf_timer
    
    // initialize bsp modules
    debugpins_init();
    leds_init();
    uart_init();
    sctimer_init();
    radio_init();
    eui64_get(eui);
}

void board_sleep(void) {
    // not sure how to enter a sleep mode
}

void board_reset(void) {
    // not sure how the reset is triggered
}

//=========================== private =========================================

// ==== optical calibration

void board_optical_calibration(void){
    
    uint8_t i;

    // crc checking, need to check why crc failed
    
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
    
    // Disable LF_CLOCK
    set_asc_bit(553); //LF_CLOCK
    //set_asc_bit(830); //HF_CLOCK
   
    // Set initial coarse/fine on HF_CLOCK
    //coarse 0:4 = 860 861 875b 876b 877b
    //fine 0:4 870 871 872 873 874b
    set_sys_clk_secondary_freq(HF_CLOCK_coarse, HF_CLOCK_fine);   //Close to 20MHz at room temp

    // Need to set crossbar so HF_CLOCK comes out divider_out_integ (gpio5, bank10)
    // This is connected to JA9 on FPGA which is HF_CLOCK
    set_asc_bit(1163);

    // Set passthrough on divide_out_integ divider
    set_asc_bit(40);


// Uncomment these two lines to use an on-chip RC oscillator as the HCLK, otherwise it will be an FPGA clock
//-------
   // Set HCLK source as HF_CLOCK
   //set_asc_bit(1147);   
   
   // Set RFTimer source as HF_CLOCK
   //set_asc_bit(1151);
//-------

    // HF_CLOCK will be trimmed to 20MHz, so set RFTimer div value to 40 to get 500kHz (inverted, so 1101 0111)
    set_asc_bit(49);
    set_asc_bit(48);
    clear_asc_bit(47);
    set_asc_bit(46);
    clear_asc_bit(45);
    set_asc_bit(44);
    set_asc_bit(43);
    set_asc_bit(42);
   
    // Set 2M RC as source for chip CLK
    set_asc_bit(1156);

    // Route IF ADC_CLK out of BLE_PDA_clk for intitial optical calibration 
    // (Can't use ADC_CLK and optical signals at the same time since they are all in the first group of four)
    // This pin is shared with Q_LC[2], so that pin was connected to counter 4 in FPGA
    // Comment these two lines out in IC version
    set_asc_bit(1180);
    set_asc_bit(1181);
   
    // Then set the divider for BLE_PDA_clk to passthrough
    // Comment this line out in IC version
    set_asc_bit(524);

    // Enable 32k for cal
    set_asc_bit(623);

    // Enable passthrough on chip CLK divider
    set_asc_bit(41);
   
    // Init counter setup - set all to analog_cfg control
    // ASC[0] is leftmost
    // ASC[0] |= 0xFF800000; 
    for(i=2; i<22; i++) {
        set_asc_bit(i);
    }
    
    // Init RX
    radio_init_rx_MF();
    
    // Init TX
    radio_init_tx();
      
    // Set initial IF ADC clock frequency
    set_IF_clock_frequency(IF_coarse, IF_fine, 0);

    // Set initial TX clock frequency
    set_2M_RC_frequency(31, 31, 21, 17, RC2M_superfine);
   
    // Turn on RC 2M for cal
    set_asc_bit(1114);
      
    // Set initial LO frequency
    LC_monotonic_ASC(LC_code);

    // Init divider settings
    radio_init_divider(2000);
    
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
    // all the 16 pins are controlled as GPO: refer to the SCuM3B final sheet
    GPO_control_FPGA(6,6,6,6);
    
    // Program analog scan chain on FPGA
    analog_scan_chain_write(&ASC_FPGA[0]);
    analog_scan_chain_load();
    
    radio_enable_LO();
    
    // Enable optical SFD interrupt for optical calibration
    ISER = 0x0800;
    
    while(optical_cal_finished == 0);
    optical_cal_finished = 0;
}

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

// ========================== interrupt ========================================


// This interrupt goes off when the optical register holds the value {221, 176, 231, 47}
// This interrupt is used to synchronize to the start of a data transfer
// Need to make sure a new bit has been clocked in prior to returning from this ISR, or else it will immediately execute again
void optical_sfd_isr(void){
    
    //printf("Optical SFD interrupt triggered\n");
    // Enable the 32bit optical interrupt
    //ISER = 0x4;

    unsigned int rdata_lsb, rdata_msb; 
    unsigned int count_LC, count_32k, count_2M, count_HFclock, count_IF;
    
    // Disable all counters
    ANALOG_CFG_REG__0 = 0x007F;

    optical_cal_iteration++;

    // Read 2M counter
    rdata_lsb = *(unsigned int*)(APB_ANALOG_CFG_BASE + 0x180000);
    rdata_msb = *(unsigned int*)(APB_ANALOG_CFG_BASE + 0x1C0000);
    count_2M = rdata_lsb + (rdata_msb << 16);
      
    // Read LC_div counter (via counter4)
    rdata_lsb = *(unsigned int*)(APB_ANALOG_CFG_BASE + 0x280000);
    rdata_msb = *(unsigned int*)(APB_ANALOG_CFG_BASE + 0x2C0000);
    count_LC = rdata_lsb + (rdata_msb << 16);
      
    // Read 32k counter
    rdata_lsb = *(unsigned int*)(APB_ANALOG_CFG_BASE + 0x000000);
    rdata_msb = *(unsigned int*)(APB_ANALOG_CFG_BASE + 0x040000);
    count_32k = rdata_lsb + (rdata_msb << 16);

    // Read HF_CLOCK counter
    rdata_lsb = *(unsigned int*)(APB_ANALOG_CFG_BASE + 0x100000);
    rdata_msb = *(unsigned int*)(APB_ANALOG_CFG_BASE + 0x140000);
    count_HFclock = rdata_lsb + (rdata_msb << 16);

    // Read IF ADC_CLK counter
    // This is a convoluted way to get access for initial optical cal
    // Normally this clock would be read from counter 6, but can't get access to ADC_CLK and optical signals at same time
    // So workaround is to plug it into counter 4 for initial calibration
    rdata_lsb = *(unsigned int*)(APB_ANALOG_CFG_BASE + 0x200000);
    rdata_msb = *(unsigned int*)(APB_ANALOG_CFG_BASE + 0x240000);
    count_IF = rdata_lsb + (rdata_msb << 16);
    // For the IC version, comment the above 3 lines and uncomment the 3 below
    //rdata_lsb = *(unsigned int*)(APB_ANALOG_CFG_BASE + 0x300000);
    //rdata_msb = *(unsigned int*)(APB_ANALOG_CFG_BASE + 0x340000);
    //count_IF = rdata_lsb + (rdata_msb << 16);

    // Reset all counters
    ANALOG_CFG_REG__0 = 0x0000;      
   
    // Enable all counters
    ANALOG_CFG_REG__0 = 0x3FFF;   

    // Update HF CLOCK if needed
    if(count_HFclock < 199600) {
        HF_CLOCK_fine--;
    }
    if(count_HFclock > 20040000) {
        HF_CLOCK_fine++;
    }
    set_sys_clk_secondary_freq(HF_CLOCK_coarse, HF_CLOCK_fine);
   
    // Start finer steps
    if(count_LC > LC_target + 60){
        LC_code -= 1;
        LC_monotonic_ASC(LC_code);
    } else {
        if(count_LC < LC_target - 60){
            LC_code += 1;
            LC_monotonic_ASC(LC_code);
        } 
    }
    
    // Do correction on 2M RC
    if(count_2M > (200100)) {
        RC2M_superfine += 1;
    }
    if(count_2M < (199900)) {
        RC2M_superfine -= 1;
    }
    set_2M_RC_frequency(31, 31, 21, 17, RC2M_superfine);
    
    // Do correction on IF RC clock
    if(count_IF > (1600000+300)) {
        IF_fine += 1;
    }
    if(count_IF < (1600000-300)) {
       IF_fine -= 1;
    }
    set_IF_clock_frequency(IF_coarse, IF_fine, 0);

    analog_scan_chain_write_3B_fromFPGA(&ASC[0]);
    analog_scan_chain_load_3B_fromFPGA();
    
    if(optical_cal_iteration == 25){
        // disable optical interrupts
        ICER = 0x0800;
        
        // mark as finished
        optical_cal_iteration = 0;
        optical_cal_finished = 1;
        
        build_channel_table(LC_code);
        
        radio_disable_all();

        // Halt all counters
        ANALOG_CFG_REG__0 = 0x0000;   
    }
}