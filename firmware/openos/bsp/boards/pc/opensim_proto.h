/**
\brief Protocol between the OpenSim client and server.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, April 2012.
*/

#ifndef __OPENSIM_PROTO_H
#define __OPENSIM_PROTO_H

#include "board_info.h"
#include "stdint.h"

//=========================== define ==========================================

//=========================== enums ===========================================

typedef enum {
   OPENSIM_ERR_NONE                         = 0,
} opensim_rc_t;

typedef enum {
   //===== from client to server
   // board
   OPENSIM_CMD_board_init                   = 0,
   OPENSIM_CMD_board_sleep                  = 1,
   // bsp_timer
   OPENSIM_CMD_bsp_timer_init               = 2,
   OPENSIM_CMD_bsp_timer_reset              = 3,
   OPENSIM_CMD_bsp_timer_scheduleIn         = 4,
   OPENSIM_CMD_bsp_timer_cancel_schedule    = 5,
   OPENSIM_CMD_bsp_timer_get_currentValue   = 6,
   // debugpins
   OPENSIM_CMD_debugpins_init               = 7,
   OPENSIM_CMD_debugpins_frame_toggle       = 8,
   OPENSIM_CMD_debugpins_frame_clr          = 9,
   OPENSIM_CMD_debugpins_frame_set          = 10,
   OPENSIM_CMD_debugpins_slot_toggle        = 11,
   OPENSIM_CMD_debugpins_slot_clr           = 12,
   OPENSIM_CMD_debugpins_slot_set           = 13,
   OPENSIM_CMD_debugpins_fsm_toggle         = 14,
   OPENSIM_CMD_debugpins_fsm_clr            = 15,
   OPENSIM_CMD_debugpins_fsm_set            = 16,
   OPENSIM_CMD_debugpins_task_toggle        = 17,
   OPENSIM_CMD_debugpins_task_clr           = 18,
   OPENSIM_CMD_debugpins_task_set           = 19,
   OPENSIM_CMD_debugpins_isr_toggle         = 20,
   OPENSIM_CMD_debugpins_isr_clr            = 21,
   OPENSIM_CMD_debugpins_isr_set            = 22,
   OPENSIM_CMD_debugpins_radio_toggle       = 23,
   OPENSIM_CMD_debugpins_radio_clr          = 24,
   OPENSIM_CMD_debugpins_radio_set          = 25,
   // eui64
   OPENSIM_CMD_eui64_get                    = 26,
   // leds
   OPENSIM_CMD_leds_init                    = 27,
   OPENSIM_CMD_leds_error_on                = 28,
   OPENSIM_CMD_leds_error_off               = 29,
   OPENSIM_CMD_leds_error_toggle            = 30,
   OPENSIM_CMD_leds_error_isOn              = 31,
   OPENSIM_CMD_leds_radio_on                = 32,
   OPENSIM_CMD_leds_radio_off               = 33,
   OPENSIM_CMD_leds_radio_toggle            = 34,
   OPENSIM_CMD_leds_radio_isOn              = 35,
   OPENSIM_CMD_leds_sync_on                 = 36,
   OPENSIM_CMD_leds_sync_off                = 37,
   OPENSIM_CMD_leds_sync_toggle             = 38,
   OPENSIM_CMD_leds_sync_isOn               = 39,
   OPENSIM_CMD_leds_debug_on                = 40,
   OPENSIM_CMD_leds_debug_off               = 41,
   OPENSIM_CMD_leds_debug_toggle            = 42,
   OPENSIM_CMD_leds_debug_isOn              = 43,
   OPENSIM_CMD_leds_all_on                  = 44,
   OPENSIM_CMD_leds_all_off                 = 45,
   OPENSIM_CMD_leds_all_toggle              = 46,
   OPENSIM_CMD_leds_circular_shift          = 47,
   OPENSIM_CMD_leds_increment               = 48,
   // radio
   OPENSIM_CMD_radio_init                   = 49,
   OPENSIM_CMD_radio_reset                  = 50,
   OPENSIM_CMD_radio_startTimer             = 51,
   OPENSIM_CMD_radio_getTimerValue          = 52,
   OPENSIM_CMD_radio_setTimerPeriod         = 53,
   OPENSIM_CMD_radio_getTimerPeriod         = 54,
   OPENSIM_CMD_radio_setFrequency           = 55,
   OPENSIM_CMD_radio_rfOn                   = 56,
   OPENSIM_CMD_radio_rfOff                  = 57,
   OPENSIM_CMD_radio_loadPacket             = 58,
   OPENSIM_CMD_radio_txEnable               = 59,
   OPENSIM_CMD_radio_txNow                  = 60,
   OPENSIM_CMD_radio_rxEnable               = 61,
   OPENSIM_CMD_radio_rxNow                  = 62,
   OPENSIM_CMD_radio_getReceivedFrame       = 63,
   // radiotimer
   OPENSIM_CMD_radiotimer_init              = 64,
   OPENSIM_CMD_radiotimer_start             = 65,
   OPENSIM_CMD_radiotimer_getValue          = 66,
   OPENSIM_CMD_radiotimer_setPeriod         = 67,
   OPENSIM_CMD_radiotimer_getPeriod         = 68,
   OPENSIM_CMD_radiotimer_schedule          = 69,
   OPENSIM_CMD_radiotimer_cancel            = 70,
   OPENSIM_CMD_radiotimer_getCapturedTime   = 71,
   // uart
   OPENSIM_CMD_uart_init                    = 72,
   OPENSIM_CMD_uart_enableInterrupts        = 73,
   OPENSIM_CMD_uart_disableInterrupts       = 74,
   OPENSIM_CMD_uart_clearRxInterrupts       = 75,
   OPENSIM_CMD_uart_clearTxInterrupts       = 76,
   OPENSIM_CMD_uart_writeByte               = 77,
   OPENSIM_CMD_uart_readByte                = 78,
   // supply
   //===== from server to client
   // board
   // bsp_timer
   OPENSIM_CMD_bsp_timer_isr                = 100,
   // debugpins
   // eui64
   // leds
   // radio
   OPENSIM_CMD_radio_isr_startFrame         = 101,
   OPENSIM_CMD_radio_isr_endFrame           = 102,
   // radiotimer
   OPENSIM_CMD_radiotimer_isr_compare       = 103,
   OPENSIM_CMD_radiotimer_isr_overflow      = 104,
   // uart
   OPENSIM_CMD_uart_isr_tx                  = 105,
   OPENSIM_CMD_uart_isr_rx                  = 106,
   // supply
   OPENSIM_CMD_supply_on                    = 107,
   OPENSIM_CMD_supply_off                   = 108
} opensim_commandId_t;

//=========================== typedef =========================================

typedef struct {
   uint8_t    cmdId;
} opensim_requ_hdr_t;

typedef struct {
   uint8_t    rc;
} opensim_repl_hdr_t;

//--------------------------- from client to server ---------------------------

//=== board
// init
// sleep

//=== bsp_timer
// init
// reset
// scheduleIn
typedef struct {
   PORT_TIMER_WIDTH delayTicks;
} opensim_requ_bsp_timer_scheduleIn_t;
// cancel_schedule
// get_currentValue
typedef struct {
   uint16_t value;
} opensim_repl_bsp_timer_get_currentValue_t;

//=== debugpins
// init
// frame_toggle
// frame_clr
// frame_set
// slot_toggle
// slot_clr
// slot_set
// fsm_toggle
// fsm_clr
// fsm_set
// task_toggle
// task_clr
// task_set
// isr_toggle
// isr_clr
// isr_set
// radio_toggle
// radio_clr
// radio_set

//=== eui64
// get
typedef struct {
   uint8_t eui64[8];
} opensim_repl_eui64_get_t;

//=== leds
// init
// error_on
// error_off
// error_toggle
// error_isOn
typedef struct {
   uint8_t isOn;
} opensim_repl_error_isOn_t;
// radio_on
// radio_off
// radio_toggle
// radio_isOn
typedef struct {
   uint8_t isOn;
} opensim_repl_radio_isOn_t;
// sync_on
// sync_off
// sync_toggle
// sync_isOn
typedef struct {
   uint8_t isOn;
} opensim_repl_sync_isOn_t;
// debug_on
// debug_off
// debug_toggle
// debug_isOn
typedef struct {
   uint8_t isOn;
} opensim_repl_debug_isOn_t;
// all_on
// all_off
// all_toggle
// circular_shift
// increment

//=== radio
// init
// reset
// startTimer
typedef struct {
   PORT_TIMER_WIDTH period;
} opensim_requ_radio_startTimer_t;
// getTimerValue
typedef struct {
   PORT_TIMER_WIDTH value;
} opensim_repl_radio_getTimerValue_t;
// setTimerPeriod
typedef struct {
   PORT_TIMER_WIDTH period;
} opensim_requ_radio_setTimerPeriod_t;
// getTimerPeriod
typedef struct {
   PORT_TIMER_WIDTH value;
} opensim_repl_radio_getTimerPeriod_t;
// setFrequency
typedef struct {
   uint8_t frequency;
} opensim_requ_radio_setFrequency_t;
// rfOn
// rfOff
// loadPacket
typedef struct {
   uint8_t txBuffer[127];
} opensim_requ_radio_loadPacket_t;
// txEnable
// txNow
// rxEnable
// rxNow
// getReceivedFrame
typedef struct {
   uint8_t rxBuffer[128];
   uint8_t len;
    int8_t rssi;
   uint8_t lqi;
   uint8_t crc;
} opensim_repl_radio_getReceivedFrame_t;

//=== radiotimer
// init
// start
typedef struct {
   uint16_t period;
} opensim_requ_radiotimer_start_t;
// getValue
typedef struct {
   uint16_t value;
} opensim_repl_radiotimer_getValue_t;
// setPeriod
typedef struct {
   uint16_t period;
} opensim_requ_radiotimer_setPeriod_t;
// getPeriod
typedef struct {
   uint16_t period;
} opensim_repl_radiotimer_getPeriod_t;
// schedule
typedef struct {
   uint16_t offset;
} opensim_requ_radiotimer_schedule_t;
// cancel
// getCapturedTime
typedef struct {
   uint16_t capturedTime;
} opensim_repl_radiotimer_getCapturedTime_t;

//=== uart
// init
// enableInterrupts
// disableInterrupts
// clearRxInterrupts
// clearTxInterrupts
// writeByte
typedef struct {
   uint8_t byteToWrite;
} opensim_requ_uart_writeByte_t;
// readByte
typedef struct {
   uint8_t byteRead;
} opensim_repl_uart_readByte_t;

//--------------------------- from client to server ---------------------------

typedef struct {
   uint16_t capturedTime;
} opensim_intr_radio_startOfFrame_t;
typedef struct {
   uint16_t capturedTime;
} opensim_intr_radio_endOfFrame_t;


//=========================== prototypes ======================================

#endif