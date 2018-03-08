/**
\brief CoAP infrared application

\author Tengfei Chang <tengfei.chang@inria.fr>, December, 2017
*/

#ifndef __CINFRARED_H
#define __CINFRARED_H

/**
\addtogroup AppCoAP
\{
\addtogroup cinfrared
\{
*/

#include "opendefs.h"
#include "opencoap.h"
#include "schedule.h"

//=========================== define ==========================================


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

//=========================== typedef =========================================

//=========================== variables =======================================

typedef struct {
    coap_resource_desc_t        desc;
    uint32_t                    startOfSlot;   // the start time of a turnon/turnoff action
    app_state_t                 state;         // state of turnon or turnoff timing
    uint8_t                     tunnOnOrOff;   // 1: turn on, 0: turn off
    opentimers_id_t             timerId;
} cinfrared_vars_t;

//=========================== prototypes ======================================

void cinfrared_init(void);

/**
\}
\}
*/

#endif
