/**
\brief This program shows the use of the "sctimer" bsp module.

Since the bsp modules for different platforms have the same declaration, you
can use this project with any platform.

Load this program onto your board, and start running. It will enable the sctimer. 
The sctimer is periodic, of period SCTIMER_PERIOD ticks. Each time it elapses:
    - the frame debugpin toggles
    - the error LED toggles

\author Tengfei Chang <tengfei.chang@eecs.berkeley.edu>, April 2017.
*/

#include "stdint.h"
#include "string.h"
#include "board.h"
#include "debugpins.h"
#include "leds.h"
#include "sctimer.h"
#include "pwm.h"

//=========================== defines =========================================

#define SCTIMER_PERIOD         65535 // @32kHz = 2s

// turnoff timing
typedef enum {
    TURNOFF_STATE_START = 0,    // turn on at   0ms
    TURNOFF_STATE_1     = 490,  // turn off at  15ms
    TURNOFF_STATE_2     = 599,  // turn off for 18ms
    TURNOFF_STATE_3     = 926,  // turn on for  28ms
    TURNOFF_STATE_4     = 981,  // turn off for 30ms
    TURNOFF_STATE_END   = 1035, // turn on for  31.5ms
} turnoff_timing_t;

// turnon timing
typedef enum {
    TURNON_STATE_START  = 0,    // turn on at   0ms
    TURNON_STATE_1      = 490,  // turn off at  15ms
    TURNON_STATE_2      = 599,  // turn off for 18ms
    TURNON_STATE_3      = 754,  // turn on for  23ms
    TURNON_STATE_4      = 819,  // turn off for 25ms
    TURNON_STATE_END    = 918,  // turn on for  28ms
} turnon_timing_t;

typedef enum {
    APP_STATE_START     = 0x00,
    APP_STATE_1         = 0x01,
    APP_STATE_2         = 0x02,
    APP_STATE_3         = 0x03,
    APP_STATE_4         = 0x04,
    APP_STATE_END       = 0x05,
} app_state_t;

//=========================== variables =======================================

typedef struct { 
    uint32_t     startOfSlot;   // the start time of a turnon/turnoff action
    app_state_t  state;         // state of turnon or turnoff timing
    uint8_t      tunnOnOrOff;   // 1: turn on, 0: turn off
} app_vars_t;

app_vars_t app_vars;

//=========================== prototypes ======================================

void cb_compare(void);

//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int mote_main(void) {
  
    memset(&app_vars,0,sizeof(app_vars_t));
    
    // initialize board.
    board_init();
    pwm_init();
    
    sctimer_set_callback(cb_compare);
    sctimer_setCompare(sctimer_readCounter()+SCTIMER_PERIOD);
    
    while (1) {
        board_sleep();
    }
}

//=========================== callbacks =======================================

void cb_compare(void) {
   
    // toggle pin
    debugpins_frame_toggle();
    
    // toggle error led
    leds_error_toggle();
    
    switch(app_vars.state){
    case APP_STATE_START:
        pwm_enable();
        app_vars.startOfSlot = sctimer_readCounter();
        if (app_vars.tunnOnOrOff){
            sctimer_setCompare(app_vars.startOfSlot+TURNON_STATE_1);
        } else {
            sctimer_setCompare(app_vars.startOfSlot+TURNOFF_STATE_1);
        }
        break;
    case APP_STATE_1:
        pwm_disable();
        if (app_vars.tunnOnOrOff){
            sctimer_setCompare(app_vars.startOfSlot+TURNON_STATE_2);
        } else {
            sctimer_setCompare(app_vars.startOfSlot+TURNOFF_STATE_2);
        }
        break;
    case APP_STATE_2:
        pwm_enable();
        if (app_vars.tunnOnOrOff){
            sctimer_setCompare(app_vars.startOfSlot+TURNON_STATE_3);
        } else {
            sctimer_setCompare(app_vars.startOfSlot+TURNOFF_STATE_3);
        }
        break;
    case APP_STATE_3:
        pwm_disable();
        if (app_vars.tunnOnOrOff){
            sctimer_setCompare(app_vars.startOfSlot+TURNON_STATE_4);
        } else {
            sctimer_setCompare(app_vars.startOfSlot+TURNOFF_STATE_4);
        }
        break;
    case APP_STATE_4:
        pwm_enable();
        if (app_vars.tunnOnOrOff){
            sctimer_setCompare(app_vars.startOfSlot+TURNON_STATE_END);
        } else {
            sctimer_setCompare(app_vars.startOfSlot+TURNOFF_STATE_END);
        }
        break;
    case APP_STATE_END:
        pwm_disable();
        sctimer_setCompare(app_vars.startOfSlot+SCTIMER_PERIOD);
        break;
    }
    
    if (app_vars.state==APP_STATE_END){
        app_vars.state = APP_STATE_START;
        // change turning action
        app_vars.tunnOnOrOff = (app_vars.tunnOnOrOff+1) & 0x01;
    } else {
        app_vars.state++;
    }   
}
