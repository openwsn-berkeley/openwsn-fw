/**
\brief This program shows the use of the "radio" bsp module.
*/

#include "stdint.h"
#include "string.h"
#include "board.h"
#include "radio.h"
#include "radio_df.h"
#include "leds.h"
#include "uart.h"
#include "sctimer.h"
#include "math.h"
#include "debugpins.h"

//=========================== defines =========================================

#define NUM_SAMPLES           SAMPLE_MAXCNT
#define NUM_SAMPLES_REFERENCE 64
#define LENGTH_PACKET         125+LENGTH_CRC // maximum length is 127 bytes
#define CHANNEL               0             // 24ghz: 11 = 2.405GHz, subghz: 11 = 865.325 in  FSK operating mode #1
#define FREQUENCY             
#define LENGTH_SERIAL_FRAME   ((NUM_SAMPLES*4)+9)   // length of the serial frame

#define DF_ENABLE             1
#define CALCULATE_ON_BOARD    1
#define INVAILD_ANGLE         361.0

#define ANGLE_HISTORY_LEN     64


//=========================== variables =======================================

typedef struct {
    uint8_t    num_radioTimerCompare;
    uint8_t    num_startFrame;
    uint8_t    num_endFrame;
} app_dbg_t;

app_dbg_t app_dbg;

typedef struct {
    // rx packet
    volatile    uint8_t    rxpk_done;
                uint8_t    rxpk_buf[LENGTH_PACKET];
                uint8_t    rxpk_len;
                uint8_t    rxpk_num;
                int8_t     rxpk_rssi;
                uint8_t    rxpk_lqi;
                bool       rxpk_crc;
                uint8_t    rxpk_freq_offset;
               uint32_t    sample_buffer[NUM_SAMPLES];
               uint16_t    num_samples;
                uint8_t    setting_coarse;
                uint8_t    setting_mid;
                uint8_t    setting_fine;
    // uart
                uint8_t    uart_txFrame[LENGTH_SERIAL_FRAME];
               uint16_t    uart_lastTxByte;
    volatile    uint8_t    uart_done;

    // phase data
                int16_t    phase_data[NUM_SAMPLES];
                int16_t    reference_data[NUM_SAMPLES+NUM_SAMPLES_REFERENCE];
                int16_t    angles_1[(NUM_SAMPLES-NUM_SAMPLES_REFERENCE)/4];
                int16_t    angles_2[(NUM_SAMPLES-NUM_SAMPLES_REFERENCE)/4];
                uint8_t    antenna_array_id;

    // store the angle
                  double    angle_array_1[ANGLE_HISTORY_LEN]; // determine signal from left or right (depends on orientation of ANT board)
                  double    angle_array_2[ANGLE_HISTORY_LEN]; // determine signal from front or back (depends on orientation of ANT board)
                  uint8_t   angle_index;

} app_vars_t;

app_vars_t app_vars;

//=========================== prototypes ======================================

// radiotimer
void cb_radioTimerOverflows(void);
// radio
void cb_startFrame(PORT_TIMER_WIDTH timestamp);
void cb_endFrame(PORT_TIMER_WIDTH timestamp);
// uart
void cb_uartTxDone(void);
uint8_t cb_uartRxCb(void);
// aoa
double calculate_aoa(void);
bool  variation_check(int16_t* data, uint8_t length);

//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int mote_main(void) {

    uint8_t i;
    double   angle_array_x;

    // clear local variables
    memset(&app_vars,0,sizeof(app_vars_t));

    // initialize board
    board_init();

    // add callback functions radio
    radio_setStartFrameCb(cb_startFrame);
    radio_setEndFrameCb(cb_endFrame);

    // setup UART
    uart_setCallbacks(cb_uartTxDone,cb_uartRxCb);

    // prepare radio
    radio_rfOn();
    // freq type only effects on scum port
    radio_setFrequency(CHANNEL, FREQ_RX);

#ifdef DF_ENABLE
    //radio_configure_switch_antenna_array();
    radio_configure_direction_finding_antenna_switch();
    radio_configure_direction_finding_manual();
#endif

    // switch in RX
    radio_rxEnable();
    radio_rxNow();

    while (1) {

        // sleep while waiting for at least one of the rxpk_done to be set

        app_vars.rxpk_done = 0;
        while (app_vars.rxpk_done==0) {
            board_sleep();
        }

        // if I get here, I just received a packet

        //===== send notification over serial port

        // led
        leds_error_on();

        // format frame to send over serial port

        for (i=0;i<NUM_SAMPLES;i++) {
            app_vars.uart_txFrame[4*i+0] = (app_vars.sample_buffer[i] >>24) & 0x000000ff;
            app_vars.uart_txFrame[4*i+1] = (app_vars.sample_buffer[i] >>16) & 0x000000ff;
            app_vars.uart_txFrame[4*i+2] = (app_vars.sample_buffer[i] >> 8) & 0x000000ff;
            app_vars.uart_txFrame[4*i+3] = (app_vars.sample_buffer[i] >> 0) & 0x000000ff;
            // get the phase data
            app_vars.phase_data[i]       = (app_vars.sample_buffer[i]     ) & 0x0000ffff;
        }
        
        app_vars.uart_txFrame[4*i+0]     = app_vars.rxpk_rssi;
        // record scum settings for transmitting
        app_vars.uart_txFrame[4*i+1]     = app_vars.setting_coarse;
        app_vars.uart_txFrame[4*i+2]     = app_vars.setting_mid;
        app_vars.uart_txFrame[4*i+3]     = app_vars.setting_fine;

        app_vars.uart_txFrame[4*i+4]     = app_vars.antenna_array_id;

        app_vars.uart_txFrame[4*i+5]     = 0xff;
        app_vars.uart_txFrame[4*i+6]     = 0xff;
        app_vars.uart_txFrame[4*i+7]     = 0xff;
        app_vars.uart_txFrame[4*i+8]     = 0xff;

        app_vars.uart_done          = 0;
        app_vars.uart_lastTxByte    = 0;

        // send app_vars.uart_txFrame over UART
        uart_clearTxInterrupts();
        uart_clearRxInterrupts();
        uart_enableInterrupts();
        uart_writeByte(app_vars.uart_txFrame[app_vars.uart_lastTxByte]);
        while (app_vars.uart_done==0); // busy wait to finish
        uart_disableInterrupts();

#if CALCULATE_ON_BOARD == 1
        angle_array_x = calculate_aoa();
        if (angle_array_x != INVAILD_ANGLE) {
            app_vars.angle_array_1[app_vars.angle_index] = angle_array_x;
            app_vars.angle_index = (app_vars.angle_index+1) & 0x3f;

            if (angle_array_x > 0) {
                // green led on board
                debugpins_slot_set();
                debugpins_fsm_clr();
            } else {
                // red led on board
                debugpins_fsm_set();
                debugpins_slot_clr();
            }

        }
#endif

        // led
        leds_error_off();
    }
}

//=========================== private =========================================

bool    variation_check(int16_t* data, uint8_t length){
    return TRUE;
}

double calculate_phase_diff(uint8_t shift){

    uint16_t i;
    int16_t diff;
    int16_t sum;
    double avg_phase_diff;

    int16_t phase_one_ant[(NUM_SAMPLES-NUM_SAMPLES_REFERENCE)/4];
    int16_t reference_one_ant[(NUM_SAMPLES-NUM_SAMPLES_REFERENCE)/4];
    int16_t phase_diff[(NUM_SAMPLES-NUM_SAMPLES_REFERENCE)/4];

    for (i=0;i<3;i++) {
        memcpy(&phase_one_ant[8*i], &app_vars.phase_data[8*(8+shift+4*i)],sizeof(int16_t)*8);
        memcpy(&phase_one_ant[8*i], &app_vars.phase_data[8*(8+shift+4*i)],sizeof(int16_t)*8);
        memcpy(&phase_one_ant[8*i], &app_vars.phase_data[8*(8+shift+4*i)],sizeof(int16_t)*8);
    }
    for (i=0;i<3;i++) {
        memcpy(&reference_one_ant[8*i], &app_vars.reference_data[8*(8+shift+4*i)],sizeof(int16_t)*8);
        memcpy(&reference_one_ant[8*i], &app_vars.reference_data[8*(8+shift+4*i)],sizeof(int16_t)*8);
        memcpy(&reference_one_ant[8*i], &app_vars.reference_data[8*(8+shift+4*i)],sizeof(int16_t)*8);
    }
    for (i=0;i<(NUM_SAMPLES-NUM_SAMPLES_REFERENCE)/4;i++) {
        diff = reference_one_ant[i] - phase_one_ant[i];
        if (diff <= -201) {
            diff += 402;
        }
        if (diff >= 201) {
            diff -= 402;
        }
        phase_diff[i] = diff;
    }

    //---- DO NOT calculate angle if the phase diff variate too much
    if (
        variation_check(phase_diff, (NUM_SAMPLES-NUM_SAMPLES_REFERENCE)/4) == 0
    ){
        return INVAILD_ANGLE;
    }

    for (i=0;i<(NUM_SAMPLES-NUM_SAMPLES_REFERENCE)/4;i++) {
        sum += phase_diff[i];
    }
    avg_phase_diff = (double)(sum)/((NUM_SAMPLES-NUM_SAMPLES_REFERENCE)/4);

    return avg_phase_diff;
}

#define SPEED_OF_LIGHT          300000000
#define NUM_SAMPLES_SWITCH_SLOT 8
#define ANT_DISTANCE            0.035

double calculate_aoa(void) {

    int16_t diff;
    int16_t sum;
    double avg_phase_diff_1;
    double avg_phase_diff_3;
    int16_t reference_data_temp[NUM_SAMPLES+NUM_SAMPLES_REFERENCE];
    
    uint8_t i;
    uint8_t wave_index_start, wave_index_end;
    double   wave_length; // in meter
    uint32_t IF;
    uint32_t frequency;

    uint8_t num_reference_sample;

    double   angle;
    double   acos_x_1;
    double   acos_x_3;
    double   angle_1;
    double   angle_3;

    uint8_t shift;

    wave_index_start = 0;
    wave_index_end   = 0;
    num_reference_sample = 64;
    //==== find a complete wave of the signal
    for (i=0; i<num_reference_sample-1; i++) {
        if (app_vars.phase_data[i] - app_vars.phase_data[i+1] > 300) {
            if (wave_index_start == 0) {
                wave_index_start = i + 1;
            } else {
                wave_index_end   = i + 1;
                break;
            }
        }
    }

    frequency       = radio_get_frequency();
    if (
        wave_index_start != 0 && 
        wave_index_end   != 0 &&
        wave_index_end - wave_index_start > 20 &&
        wave_index_end - wave_index_start < 44
    ) {
        IF          = 8000000/(wave_index_end-wave_index_start);
        frequency   = frequency*1000000 + IF;
        wave_length = (double)SPEED_OF_LIGHT / frequency;
    } else {
        return INVAILD_ANGLE;
    }

    //===== generate the target phase data when reference antenna is used through CTE)
    memcpy(reference_data_temp,     app_vars.phase_data, sizeof(int16_t)*wave_index_end);
    i = wave_index_end;
    while (i<(NUM_SAMPLES+NUM_SAMPLES_SWITCH_SLOT)){
        memcpy(
            &reference_data_temp[i],
            &app_vars.phase_data[wave_index_start],
            sizeof(int16_t)*(wave_index_end-wave_index_start)
        );
        i += (wave_index_end-wave_index_start);
    }

    memcpy(
        app_vars.reference_data,
        reference_data_temp,
        sizeof(int16_t)*num_reference_sample
    );
    memcpy(
        &app_vars.reference_data[num_reference_sample],
        &reference_data_temp[num_reference_sample+NUM_SAMPLES_SWITCH_SLOT],
        sizeof(int16_t)*(NUM_SAMPLES-num_reference_sample)
    );
    
    //==== generate phase diff between ant_x.2 and ant_x.1; ant_x.3 and ant_x.2

    //---- generate phase diff between ant_x.2 and ant_x.1

    shift = 0;
    avg_phase_diff_1 = calculate_phase_diff(shift);

    //---- generate phase diff between ant_x.3 and ant_x.2

    shift = 2;
    avg_phase_diff_3 = calculate_phase_diff(shift);

    //==== calculate the angle

    acos_x_1  = (avg_phase_diff_1/402.0) * wave_length / ANT_DISTANCE;
    if (acos_x_1 >= -1 && acos_x_1 <= 1) {
        angle_1  = acos(acos_x_1);
    } else {
        return INVAILD_ANGLE;
    }

    acos_x_3  = (avg_phase_diff_3/402.0) * wave_length / ANT_DISTANCE;
    if (acos_x_3 >= -1 && acos_x_3 <= 1){
        angle_3  = acos(acos_x_3);
    } else {
        return INVAILD_ANGLE;
    }

    angle = 2*tan(angle_1)*tan(angle_3)/(tan(angle_1)+tan(angle_3));
    angle = 180 * atan(angle) / M_PI;

    if (angle>0){
        // green led on board
        debugpins_slot_set();
        debugpins_fsm_clr();
    } else {
        // red led on board
        debugpins_fsm_set();
        debugpins_slot_clr();
    }

    return angle;
}

//=========================== callbacks =======================================

//===== radio

void cb_startFrame(PORT_TIMER_WIDTH timestamp) {

    leds_sync_on();
    // update debug stats
    app_dbg.num_startFrame++;
}

void cb_endFrame(PORT_TIMER_WIDTH timestamp) {

    uint8_t  i;
    bool     expectedFrame;

    // update debug stats
    app_dbg.num_endFrame++;

    memset(&app_vars.rxpk_buf[0],0,LENGTH_PACKET);

    app_vars.rxpk_freq_offset = radio_getFrequencyOffset();

    // get packet from radio
    radio_getReceivedFrame(
        app_vars.rxpk_buf,
        &app_vars.rxpk_len,
        sizeof(app_vars.rxpk_buf),
        &app_vars.rxpk_rssi,
        &app_vars.rxpk_lqi,
        &app_vars.rxpk_crc
    );
    app_vars.num_samples    = radio_get_df_samples(app_vars.sample_buffer,NUM_SAMPLES);
    app_vars.setting_coarse = app_vars.rxpk_buf[2];
    app_vars.setting_mid    = app_vars.rxpk_buf[3];
    app_vars.setting_fine   = app_vars.rxpk_buf[4];

    if (app_vars.rxpk_crc && app_vars.rxpk_len==5) {

        // indicate I just received a packet from bsp_radio_tx mote
        app_vars.rxpk_done = 1;

        app_vars.antenna_array_id = radio_get_antenna_array_id();
        
#ifdef DF_ENABLE
        //radio_configure_switch_antenna_array();
        radio_configure_direction_finding_antenna_switch();
        radio_configure_direction_finding_manual();
#endif
    }

    radio_rxEnable();
    radio_rxNow();

    // led
    leds_sync_off();
}

//===== uart

void cb_uartTxDone(void) {

    uart_clearTxInterrupts();

    // prepare to send the next byte
    app_vars.uart_lastTxByte++;

    if (app_vars.uart_lastTxByte<LENGTH_SERIAL_FRAME) {
        uart_writeByte(app_vars.uart_txFrame[app_vars.uart_lastTxByte]);
    } else {
        app_vars.uart_done=1;
    }
}

uint8_t cb_uartRxCb(void) {

    //  uint8_t byte;
    uart_clearRxInterrupts();
    return 1;
}
