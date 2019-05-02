/**
\brief SCuM-specific definition of the "radio" bsp module.

\author Tengfei Chang <tengfei.chang@inria.fr>, August 2016.
*/


#include "board.h"
#include "radio.h"
#include "sctimer.h"
#include "debugpins.h"
#include "leds.h"
#include "memory_map.h"
#include "scm3_hardware_interface.h"

//=========================== defines =========================================

#define MAXLENGTH_TRX_BUFFER    128 //// 1B length, 125B data, 2B CRC

// ==== default crc check result and rssi value

#define DEFAULT_CRC_CHECK       0x01    // this is an arbitrary value for now
#define DEFAULT_RSSI            -50     // this is an arbitrary value for now
#define DEFAULT_FREQ             11     // since the LC calibration has some problem, just use the channel 11 for now

// ==== for calibration

#define  IF_FREQ_UPDATE_TIMEOUT  10
#define  LO_FREQ_UPDATE_TIMEOUT  10
#define  FILTER_WINDOWS_LEN      11
#define  FIR_COEFF_SCALE         512 // determined by FIR_coeff

// ==== for recognizing panid

#define  LEN_PKT_INDEX           0x00
#define  PANID_LBYTE_PKT_INDEX   0x04
#define  PANID_HBYTE_PKT_INDEX   0x05
#define  DEFAULT_PANID           0xcafe

//=========================== variables =======================================

extern unsigned int ASC[38];
extern unsigned int RX_channel_codes[16];
extern unsigned int TX_channel_codes[16];

extern unsigned int LC_code;

typedef struct {
    radio_capture_cbt         startFrame_cb;
    radio_capture_cbt         endFrame_cb;
    uint8_t                   radio_tx_buffer[MAXLENGTH_TRX_BUFFER] __attribute__ ((aligned (4)));
    uint8_t                   radio_rx_buffer[MAXLENGTH_TRX_BUFFER] __attribute__ ((aligned (4)));
    radio_state_t             state; 
    uint8_t                   current_frequency;
    bool                      crc_ok;
} radio_vars_t;

radio_vars_t radio_vars;

unsigned char   FIR_coeff[FILTER_WINDOWS_LEN]           = {4,   16, 37, 64, 87, 96, 87, 64, 37, 16,  4};
unsigned int    IF_estimate_history[FILTER_WINDOWS_LEN] = {500,500,500,500,500,500,500,500,500,500,500};
signed short    cdr_tau_history[FILTER_WINDOWS_LEN]     = {0,    0,  0,  0,  0,  0,  0,  0,  0,  0,  0};
unsigned short  IF_freq_freshness_timeout               =  0;
unsigned short  LO_freq_freshness_timeout               =  0;
signed short    cdr_tau_value;
unsigned int    LQI_chip_errors;
unsigned int    IF_estimate;

// MF IF clock settings
extern unsigned int    IF_clk_target;
extern unsigned int    IF_coarse;
extern unsigned int    IF_fine;

//=========================== prototypes ======================================

void radio_calibration(void);

//=========================== public ==========================================

//===== admin

void radio_init(void) {

    // clear variables
    memset(&radio_vars,0,sizeof(radio_vars_t));
    
    // change state
    radio_vars.state                = RADIOSTATE_STOPPED;

    // Enable radio interrupts in NVIC
    ISER = 0x40;
    
#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
    // enable sfd done and send done interruptions of tranmission
    // enable sfd done and receiving done interruptions of reception
    RFCONTROLLER_REG__INT_CONFIG    = TX_LOAD_DONE_INT_EN           |   \
                                      TX_SFD_DONE_INT_EN            |   \
                                      TX_SEND_DONE_INT_EN           |   \
                                      RX_SFD_DONE_INT_EN            |   \
                                      RX_DONE_INT_EN                |   \
                                      TX_SFD_DONE_RFTIMER_PULSE_EN  |   \
                                      TX_SEND_DONE_RFTIMER_PULSE_EN |   \
                                      RX_SFD_DONE_RFTIMER_PULSE_EN  |   \
                                      RX_DONE_RFTIMER_PULSE_EN;
#else
    // enable sfd done and send done interruptions of tranmission
    RFCONTROLLER_REG__INT_CONFIG    = TX_SFD_DONE_INT_EN            |   \
                                      TX_SEND_DONE_INT_EN           |   \
                                      RX_SFD_DONE_INT_EN            |   \
                                      RX_DONE_INT_EN;
#endif
    // Enable all errors
    RFCONTROLLER_REG__ERROR_CONFIG  = TX_OVERFLOW_ERROR_EN          |   \
                                      TX_CUTOFF_ERROR_EN            |   \
                                      RX_OVERFLOW_ERROR_EN          |   \
                                      RX_CRC_ERROR_EN               |   \
                                      RX_CUTOFF_ERROR_EN;
    
    // change state
    radio_vars.state                = RADIOSTATE_RFOFF;
}

void radio_setStartFrameCb(radio_capture_cbt cb) {
    radio_vars.startFrame_cb    = cb;
}

void radio_setEndFrameCb(radio_capture_cbt cb) {
    radio_vars.endFrame_cb      = cb;
}

//===== reset

void radio_reset(void) {
    // reset SCuM radio module
    PORT_PIN_RADIO_RESET_LOW();
}

//===== RF admin


// Call this to setup RX, followed quickly by a TX ack
// Note that due to the two ASC program cycles, this function takes about 27ms to execute (@5MHz HCLK)
void setFrequencyRX(unsigned int channel){
    
    // Set LO code for RX channel
    LC_monotonic_ASC(RX_channel_codes[channel-11],0);
    
    //printf("chan code = %d\n", RX_channel_codes[channel-11]);
    
    // On FPGA, have to use the chip's GPIO outputs for radio signals
    // Note that can't reprogram while the RX is active
    GPO_control(2,10,1,1);

    // Turn polyphase on for RX
    set_asc_bit(971);

    // Enable mixer for RX
    clear_asc_bit(298);
    clear_asc_bit(307);

    // Analog scan chain setup for radio LDOs for RX
    set_asc_bit(504); // = gpio_pon_en_if
    set_asc_bit(506); // = gpio_pon_en_lo
    clear_asc_bit(508); // = gpio_pon_en_pa
    clear_asc_bit(514); // = gpio_pon_en_div

    // Write and load analog scan chain
    analog_scan_chain_write_3B_fromFPGA(&ASC[0]);
    analog_scan_chain_load_3B_fromFPGA();
}


// Call this to setup TX, followed quickly by a RX ack
void setFrequencyTX(unsigned int channel){
    
    // Set LO code for TX channel
    LC_monotonic_ASC(TX_channel_codes[channel-11],1);

    // Turn polyphase off for TX
    clear_asc_bit(971);

    // Hi-Z mixer wells for TX
    set_asc_bit(298);
    set_asc_bit(307);

    // Analog scan chain setup for radio LDOs for TX
    clear_asc_bit(504); // = gpio_pon_en_if
    set_asc_bit(506); // = gpio_pon_en_lo
    set_asc_bit(508); // = gpio_pon_en_pa
    clear_asc_bit(514); // = gpio_pon_en_div

    // Write and load analog scan chain
    analog_scan_chain_write_3B_fromFPGA(&ASC[0]);
    analog_scan_chain_load_3B_fromFPGA();
}

void radio_setFrequency(uint8_t frequency, uint8_t tx_or_rx) {
    // change state
    radio_vars.state = RADIOSTATE_SETTING_FREQUENCY;
    
//    radio_vars.current_frequency = frequency;
    radio_vars.current_frequency = DEFAULT_FREQ;
    
    switch(tx_or_rx){
    case 0x01:
        setFrequencyTX(radio_vars.current_frequency);
        break;
    case 0x02:
        setFrequencyRX(radio_vars.current_frequency);
        break;
    default:
        // shouldn't happen
        break;
    }
    
    // change state
    radio_vars.state = RADIOSTATE_FREQUENCY_SET;
}

void radio_rfOn(void) {
    // clear reset pin
    RFCONTROLLER_REG__CONTROL   &= ~RX_RESET;
}

void radio_rfOff(void) {
    // change state
    radio_vars.state            = RADIOSTATE_TURNING_OFF;

    // turn SCuM radio off
    RFCONTROLLER_REG__CONTROL   = RX_RESET;
    
    // Hold digital baseband in reset
    ANALOG_CFG_REG__4 = 0x2000;

    // Turn off LDOs
    ANALOG_CFG_REG__10 = 0x0000;
    
    // wiggle debug pin
    debugpins_radio_clr();
    leds_radio_off();

    // change state
    radio_vars.state            = RADIOSTATE_RFOFF;
}

//===== TX

#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
void radio_loadPacket_prepare(uint8_t* packet, uint16_t len){
    
    radio_vars.state = RADIOSTATE_LOADING_PACKET;
    
    memcpy(&radio_vars.radio_tx_buffer[0],packet,len);
    
    RFCONTROLLER_REG__TX_DATA_ADDR  = &(radio_vars.radio_tx_buffer[0]);
    RFCONTROLLER_REG__TX_PACK_LEN   = len;
    
    // will be loaded when load timer fired, change the state in advance
    radio_vars.state = RADIOSTATE_PACKET_LOADED;
}
#endif

void radio_loadPacket(uint8_t* packet, uint16_t len) {
    uint8_t i;
    // change state
    radio_vars.state = RADIOSTATE_LOADING_PACKET;
    
    memcpy(&radio_vars.radio_tx_buffer[0],packet,len);

    // load packet in TXFIFO
    RFCONTROLLER_REG__TX_DATA_ADDR  = &(radio_vars.radio_tx_buffer[0]);
    RFCONTROLLER_REG__TX_PACK_LEN   = len;

    RFCONTROLLER_REG__CONTROL       = TX_LOAD;

    // add some delay for loading
    for (i=0;i<0xff;i++);
    
    radio_vars.state = RADIOSTATE_PACKET_LOADED;

}

void radio_txEnable(void) {
    // change state
    radio_vars.state = RADIOSTATE_ENABLING_TX;

    // turn on LO, PA, and AUX LDOs
    ANALOG_CFG_REG__10 = 0x00A8;
    
    // wiggle debug pin
    debugpins_radio_set();
    leds_radio_on();

    // change state
    radio_vars.state = RADIOSTATE_TX_ENABLED;
}

void radio_txNow(void) {

    // change state
    RFCONTROLLER_REG__CONTROL = TX_SEND;
}

//===== RX

void radio_rxPacket_prepare(void){
    DMA_REG__RF_RX_ADDR         = &(radio_vars.radio_rx_buffer[0]);
}

void radio_rxEnable(void) {
    
    // change state
    radio_vars.state            = RADIOSTATE_ENABLING_RX;
    // Turn on LO, IF, and AUX LDOs via memory mapped register
    ANALOG_CFG_REG__10 = 0x0098;
    
    // reset
    RFCONTROLLER_REG__CONTROL   = RX_RESET;
    
    // set receiving buffer address
    DMA_REG__RF_RX_ADDR         = &(radio_vars.radio_rx_buffer[0]);
    
    // Reset digital baseband
    ANALOG_CFG_REG__4 = 0x2000;
    ANALOG_CFG_REG__4 = 0x2800;
    // start to listen
    RFCONTROLLER_REG__CONTROL   = RX_START;
    
    // wiggle debug pin
    debugpins_radio_set();
    leds_radio_on();
    
    // change state
    radio_vars.state            = RADIOSTATE_LISTENING;
}

void radio_rxEnable_scum(void){
    // change state
    radio_vars.state            = RADIOSTATE_ENABLING_RX;
    
    // wiggle debug pin
    debugpins_radio_set();
    leds_radio_on();
    
    // change state
    radio_vars.state            = RADIOSTATE_LISTENING;
}

void radio_rxNow(void) {
    // nothing to do
}

void radio_getReceivedFrame(uint8_t* pBufRead,
                            uint8_t* pLenRead,
                            uint8_t  maxBufLen,
                             int8_t* pRssi,
                            uint8_t* pLqi,
                               bool* pCrc) {
    
    //===== crc
    *pCrc           = radio_vars.crc_ok;
   
    //===== rssi
    *pRssi          = DEFAULT_RSSI;
    
    //===== length
    *pLenRead       = radio_vars.radio_rx_buffer[0];
    
    //===== packet 
    if (*pLenRead<=maxBufLen) {
        memcpy(pBufRead,&(radio_vars.radio_rx_buffer[1]),*pLenRead);
    }
}

//=========================== private =========================================

void radio_calibration(void) {

    int8_t   i;
    uint8_t  packet_len;
    uint16_t panid;
    uint32_t IF_est_filtered;
    int32_t  chip_rate_error_ppm;
    int32_t  chip_rate_error_ppm_filtered;
    int32_t  sum;
    
    packet_len = radio_vars.radio_rx_buffer[LEN_PKT_INDEX];
    panid      = (uint16_t)(radio_vars.radio_rx_buffer[PANID_LBYTE_PKT_INDEX]);
    panid     |= ((uint16_t)(radio_vars.radio_rx_buffer[PANID_HBYTE_PKT_INDEX]))<<8;
    
    if( radio_vars.crc_ok){
    
        // When updating LO and IF clock frequncies, must wait long enough for the changes to propagate before changing again
        // Need to receive as many packets as there are taps in the FIR filter
        IF_freq_freshness_timeout++;
        LO_freq_freshness_timeout++;
        
//        printf("IF_estimate=%d, cdr_tau_value=%d  LQI_chip_errors=%d\r\n",IF_estimate,cdr_tau_value,LQI_chip_errors);
        
        // ==== FIR filter for cdr tau slope ====
        
        // the IF clock is for receiver chip clock
        
        sum = 0;
        
        // A tau value of 0 indicates there is no rate mistmatch between the TX and RX chip clocks
        // The cdr_tau_value corresponds to the number of samples that were added or dropped by the CDR
        // Each sample point is 1/16MHz = 62.5ns
        // Need to estimate ppm error for each packet, then FIR those values to make tuning decisions
        // error_in_ppm = 1e6 * (#adjustments * 62.5ns) / (packet length (bytes) * 64 chips/byte * 500ns/chip)
        // Which can be simplified to (#adjustments * 15625) / (packet length * 8)
        
        chip_rate_error_ppm = (cdr_tau_value * 15625) / (packet_len * 8);
        
        // Shift old samples
        for (i=FILTER_WINDOWS_LEN-2; i>=0; i--){
            cdr_tau_history[i+1] = cdr_tau_history[i];        
        }
        
        // New sample
        cdr_tau_history[0] = chip_rate_error_ppm;
        
        // Do FIR convolution
        for (i=0; i<FILTER_WINDOWS_LEN; i++){
            sum = sum + cdr_tau_history[i] * FIR_coeff[i];
        }
        
        // Divide by 512 (sum of the coefficients) to scale output
        chip_rate_error_ppm_filtered = sum / FIR_COEFF_SCALE;
        
        // The IF clock frequency steps are about 2000ppm, so make an adjustment only if the error is larger than 1000ppm
        // Must wait long enough between changes for FIR to settle (at least 10 packets)
        // Need to add some handling here in case the IF_fine code will rollover with this change (0 <= IF_fine <= 31)
        if(IF_freq_freshness_timeout >= IF_FREQ_UPDATE_TIMEOUT){
            if(chip_rate_error_ppm_filtered > 1000) {
                set_IF_clock_frequency(IF_coarse, IF_fine++, 0);
            }
            if(chip_rate_error_ppm_filtered < -1000) {
                set_IF_clock_frequency(IF_coarse, IF_fine--, 0);
            }
            IF_freq_freshness_timeout = 0;
            
//            printf("chip_rate_error_ppm = %d, chip_rate_error_ppm_filtered = %d \r\n", chip_rate_error_ppm,   chip_rate_error_ppm_filtered);
        }
            
        // ==== FIR filter for IF estimate ====
        
        sum = 0;
        
        // The IF estimate reports how many zero crossings (both pos and neg) there were in a 100us period
        // The IF should on average be 2.5 MHz, which means the IF estimate will return ~500 when there is no IF error
        // Each tick is roughly 5 kHz of error
        
        // Only make adjustments when the chip error rate is <10% (this value was picked as an arbitrary choice)
        // While packets can be received at higher chip error rates, the average IF estimate tends to be less accurate
        // Estimated chip_error_rate = LQI_chip_errors/256 (assuming the packet length was at least 8 symbols)
        
        // Shift old samples
        for (i=FILTER_WINDOWS_LEN-2; i>=0; i--){
            IF_estimate_history[i+1] = IF_estimate_history[i];        
        }
        
        // New sample
        IF_estimate_history[0] = IF_estimate;

        // Do FIR convolution
        for (i=0; i<FILTER_WINDOWS_LEN; i++){
            sum = sum + IF_estimate_history[i] * FIR_coeff[i];        
        }
        IF_est_filtered = sum / FIR_COEFF_SCALE;
        
//        printf("IF_estimate = %d, IF_est_filtered = %d \r\n", IF_estimate, IF_est_filtered);

        // The LO frequency steps are about ~80-100 kHz, so make an adjustment only if the error is larger than that
        // These hysteresis bounds (+/- X) have not been optimized
        // Must wait long enough between changes for FIR to settle (at least as many packets as there are taps in the FIR)
        // For now, assume that TX/RX should both be updated, even though the IF information is only from the RX code
        if(LO_freq_freshness_timeout >= LO_FREQ_UPDATE_TIMEOUT){
            if(IF_est_filtered > 520){
                RX_channel_codes[radio_vars.current_frequency - 11]++; 
//                TX_channel_codes[radio_vars.current_frequency - 11]++; 
            }
            if(IF_est_filtered < 480){
                RX_channel_codes[radio_vars.current_frequency - 11]--; 
//                TX_channel_codes[radio_vars.current_frequency - 11]--; 
            }

            LO_freq_freshness_timeout = 0;
            
//            printf("IF_estimate         = %d, IF_est_filtered              = %d \r\n", IF_estimate,           IF_est_filtered);
        }
        
//        printf("RX_channel_codes[0]=%d, TX_channel_codes[0]=%d \r\n",RX_channel_codes[0], TX_channel_codes[0]);
        
        // Write and load analog scan chain
        
        analog_scan_chain_write_3B_fromFPGA(&ASC[0]);
        analog_scan_chain_load_3B_fromFPGA();
    }
}

//=========================== callbacks =======================================

//=========================== interrupt handlers ==============================

kick_scheduler_t radio_isr(void) {
    
    PORT_TIMER_WIDTH capturedTime;
    
    PORT_TIMER_WIDTH irq_status = RFCONTROLLER_REG__INT;
    PORT_TIMER_WIDTH irq_error  = RFCONTROLLER_REG__ERROR;

    
    debugpins_isr_set();
    
#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
#else
    capturedTime                = sctimer_readCounter();
#endif
    if (irq_status & TX_SFD_DONE_INT || irq_status & RX_SFD_DONE_INT){
        // SFD is just sent or received, check the specific interruption and 
        // change the radio state accordingly
        if (irq_status & TX_SFD_DONE_INT) {
#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
            // get the capture Time from capture register
            capturedTime = TIMER_COUNTER_CONVERT_RFTIMER_CLK_TO_32K(RFTIMER_REG__CAPTURE0);
#endif
            RFCONTROLLER_REG__INT_CLEAR = TX_SFD_DONE_INT;
            // a SFD is just sent, update radio state
            radio_vars.state    = RADIOSTATE_TRANSMITTING;
        }
        if (irq_status & RX_SFD_DONE_INT) {
#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
            // get the capture Time from capture register
            capturedTime = TIMER_COUNTER_CONVERT_RFTIMER_CLK_TO_32K(RFTIMER_REG__CAPTURE1);
#endif
            RFCONTROLLER_REG__INT_CLEAR = RX_SFD_DONE_INT;
            // a SFD is just received, update radio state
            radio_vars.state    = RADIOSTATE_RECEIVING;
        }
        
        if (radio_vars.startFrame_cb!=NULL) {
            // call the callback
            radio_vars.startFrame_cb(capturedTime);
            debugpins_isr_clr();
            // kick the OS
            return KICK_SCHEDULER;
        }
    }
    
    if (irq_status & TX_SEND_DONE_INT || irq_status & RX_DONE_INT){
        if (irq_status & TX_SEND_DONE_INT) {
#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
            // get the capture Time from capture register
            capturedTime = TIMER_COUNTER_CONVERT_RFTIMER_CLK_TO_32K(RFTIMER_REG__CAPTURE2);
#endif
            RFCONTROLLER_REG__INT_CLEAR = TX_SEND_DONE_INT;
        }
        if (irq_status & RX_DONE_INT) {
#ifdef SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT
            // get the capture Time from capture register
            capturedTime = TIMER_COUNTER_CONVERT_RFTIMER_CLK_TO_32K(RFTIMER_REG__CAPTURE3);
#endif
            RFCONTROLLER_REG__INT_CLEAR = RX_DONE_INT;
            
            // record calibration related value
            
            // Only record IF estimate, LQI, and CDR tau for valid packets
            IF_estimate = read_IF_estimate();
            LQI_chip_errors    = ANALOG_CFG_REG__21 & 0xFF; //read_LQI();
        
            // Read the value of tau debug at end of packet
            // Do this later in the ISR to make sure this register has settled before trying to read it
            // (the register is on the adc clock domain)
            cdr_tau_value = ANALOG_CFG_REG__25;
            
            // check crc
            
            if (irq_error & RX_CRC_ERROR) {
                radio_vars.crc_ok = FALSE;
            } else {
                radio_vars.crc_ok = TRUE;
            }
        }
        // the packet transmission or reception is done,
        // update the radio state
        radio_vars.state        = RADIOSTATE_TXRX_DONE;
        if (radio_vars.endFrame_cb!=NULL) {
            // call the callback
            radio_vars.endFrame_cb(capturedTime);
            
            if (irq_status & RX_DONE_INT) {
                radio_calibration();
                radio_rxEnable();
            }
            
            debugpins_isr_clr();
            // kick the OS
            return KICK_SCHEDULER;
        } else {
            while(1);
        }
    }
    
    if (irq_status & TX_LOAD_DONE_INT){
        RFCONTROLLER_REG__INT_CLEAR = TX_LOAD_DONE_INT;
    }
    
    if (irq_error != 0) {
        // error happens during the operation of radio. Print out the error here. 
        // To Be Done. add error description deifinition for this type of errors.
        RFCONTROLLER_REG__ERROR_CLEAR = irq_error;
        
        radio_rxEnable();
    }
    debugpins_isr_clr();
    return DO_NOT_KICK_SCHEDULER;
}
