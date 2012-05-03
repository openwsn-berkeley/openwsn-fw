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
   OPENSIM_CMD_bsp_timer_scheduleIn         = 4, // param
   OPENSIM_CMD_bsp_timer_cancel_schedule    = 5,
   // debugpins
   OPENSIM_CMD_debugpins_init               = 6,
   OPENSIM_CMD_debugpins_frame_toggle       = 7,
   OPENSIM_CMD_debugpins_frame_clr          = 8,
   OPENSIM_CMD_debugpins_frame_set          = 9,
   OPENSIM_CMD_debugpins_slot_toggle        = 10,
   OPENSIM_CMD_debugpins_slot_clr           = 11,
   OPENSIM_CMD_debugpins_slot_set           = 12,
   OPENSIM_CMD_debugpins_fsm_toggle         = 13,
   OPENSIM_CMD_debugpins_fsm_clr            = 14,
   OPENSIM_CMD_debugpins_fsm_set            = 15,
   OPENSIM_CMD_debugpins_task_toggle        = 16,
   OPENSIM_CMD_debugpins_task_clr           = 17,
   OPENSIM_CMD_debugpins_task_set           = 18,
   OPENSIM_CMD_debugpins_isr_toggle         = 19,
   OPENSIM_CMD_debugpins_isr_clr            = 20,
   OPENSIM_CMD_debugpins_isr_set            = 21,
   OPENSIM_CMD_debugpins_radio_toggle       = 22,
   OPENSIM_CMD_debugpins_radio_clr          = 23,
   OPENSIM_CMD_debugpins_radio_set          = 24,
   // eui64
   OPENSIM_CMD_eui64_get                    = 25, // requires answer
   // leds
   OPENSIM_CMD_leds_init                    = 26,
   OPENSIM_CMD_leds_error_on                = 27,
   OPENSIM_CMD_leds_error_off               = 28,
   OPENSIM_CMD_leds_error_toggle            = 29,
   OPENSIM_CMD_leds_error_isOn              = 30, // requires answer
   OPENSIM_CMD_leds_radio_on                = 31,
   OPENSIM_CMD_leds_radio_off               = 32,
   OPENSIM_CMD_leds_radio_toggle            = 33,
   OPENSIM_CMD_leds_radio_isOn              = 34, // requires answer
   OPENSIM_CMD_leds_sync_on                 = 35,
   OPENSIM_CMD_leds_sync_off                = 36,
   OPENSIM_CMD_leds_sync_toggle             = 37,
   OPENSIM_CMD_leds_sync_isOn               = 38, // requires answer
   OPENSIM_CMD_leds_debug_on                = 39,
   OPENSIM_CMD_leds_debug_off               = 40,
   OPENSIM_CMD_leds_debug_toggle            = 41,
   OPENSIM_CMD_leds_debug_isOn              = 42, // requires answer
   OPENSIM_CMD_leds_all_on                  = 43,
   OPENSIM_CMD_leds_all_off                 = 44,
   OPENSIM_CMD_leds_all_toggle              = 45,
   OPENSIM_CMD_leds_circular_shift          = 46,
   OPENSIM_CMD_leds_increment               = 47,
   // radio
   OPENSIM_CMD_radio_init                   = 48,
   OPENSIM_CMD_radio_reset                  = 49,
   OPENSIM_CMD_radio_startTimer             = 50,
   OPENSIM_CMD_radio_getTimerValue          = 51, // requires answer
   OPENSIM_CMD_radio_setTimerPeriod         = 52,
   OPENSIM_CMD_radio_getTimerPeriod         = 53, // requires answer
   OPENSIM_CMD_radio_setFrequency           = 54,
   OPENSIM_CMD_radio_rfOn                   = 55,
   OPENSIM_CMD_radio_rfOff                  = 56,
   OPENSIM_CMD_radio_loadPacket             = 57,
   OPENSIM_CMD_radio_txEnable               = 58,
   OPENSIM_CMD_radio_txNow                  = 59,
   OPENSIM_CMD_radio_rxEnable               = 60,
   OPENSIM_CMD_radio_rxNow                  = 61,
   OPENSIM_CMD_radio_getReceivedFrame       = 62, // requires answer
   // radiotimer
   OPENSIM_CMD_radiotimer_init              = 63,
   OPENSIM_CMD_radiotimer_start             = 64,
   OPENSIM_CMD_radiotimer_getValue          = 65, // requires answer
   OPENSIM_CMD_radiotimer_setPeriod         = 66,
   OPENSIM_CMD_radiotimer_getPeriod         = 67, // requires answer
   OPENSIM_CMD_radiotimer_schedule          = 68,
   OPENSIM_CMD_radiotimer_cancel            = 69,
   OPENSIM_CMD_radiotimer_getCapturedTime   = 70, // requires answer
   // uart
   OPENSIM_CMD_uart_init                    = 71,
   OPENSIM_CMD_uart_enableInterrupts        = 72,
   OPENSIM_CMD_uart_disableInterrupts       = 73,
   OPENSIM_CMD_uart_clearRxInterrupts       = 74,
   OPENSIM_CMD_uart_clearTxInterrupts       = 75,
   OPENSIM_CMD_uart_writeByte               = 76,
   OPENSIM_CMD_uart_readByte                = 77, // requires answer
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
} opensim_command_t;

//=========================== typedef =========================================

typedef struct {
   uint8_t    cmdId;
} opensim_requ_hdr_t;

typedef struct {
   uint8_t    rc;
} opensim_repl_hdr_t;

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
typedef opensim_requ_hdr_t   opensim_requ_leds_init_t;
typedef opensim_repl_hdr_t   opensim_repl_leds_init_t;
// error_on
typedef opensim_requ_hdr_t   opensim_requ_leds_error_on_t;
typedef opensim_repl_hdr_t   opensim_repl_leds_error_on_t;
// error_off
typedef opensim_requ_hdr_t   opensim_requ_leds_error_off_t;
typedef opensim_repl_hdr_t   opensim_repl_leds_error_off_t;
// error_toggle
typedef opensim_requ_hdr_t   opensim_requ_leds_error_toggle_t;
typedef opensim_repl_hdr_t   opensim_repl_leds_error_toggle_t;
// error_isOn
typedef opensim_requ_hdr_t   opensim_requ_leds_error_isOn_t;
typedef opensim_repl_hdr_t   opensim_repl_leds_error_isOn_t;
// radio_on
typedef opensim_requ_hdr_t   opensim_requ_leds_radio_on_t;
typedef opensim_repl_hdr_t   opensim_repl_leds_radio_on_t;
// radio_off
typedef opensim_requ_hdr_t   opensim_requ_leds_radio_off_t;
typedef opensim_repl_hdr_t   opensim_repl_leds_radio_off_t;
// radio_toggle
typedef opensim_requ_hdr_t   opensim_requ_leds_radio_toggle_t;
typedef opensim_repl_hdr_t   opensim_repl_leds_radio_toggle_t;
// radio_isOn
typedef opensim_requ_hdr_t   opensim_requ_leds_radio_isOn_t;
typedef opensim_repl_hdr_t   opensim_repl_leds_radio_isOn_t;
// sync_on
typedef opensim_requ_hdr_t   opensim_requ_leds_sync_on_t;
typedef opensim_repl_hdr_t   opensim_repl_leds_sync_on_t;
// sync_off
typedef opensim_requ_hdr_t   opensim_requ_leds_sync_off_t;
typedef opensim_repl_hdr_t   opensim_repl_leds_sync_off_t;
// sync_toggle
typedef opensim_requ_hdr_t   opensim_requ_leds_sync_toggle_t;
typedef opensim_repl_hdr_t   opensim_repl_leds_sync_toggle_t;
// sync_isOn
typedef opensim_requ_hdr_t   opensim_requ_leds_sync_isOn_t;
typedef opensim_repl_hdr_t   opensim_repl_leds_sync_isOn_t;
// debug_on
typedef opensim_requ_hdr_t   opensim_requ_leds_debug_on_t;
typedef opensim_repl_hdr_t   opensim_repl_leds_debug_on_t;
// debug_off
typedef opensim_requ_hdr_t   opensim_requ_leds_debug_off_t;
typedef opensim_repl_hdr_t   opensim_repl_leds_debug_off_t;
// debug_toggle
typedef opensim_requ_hdr_t   opensim_requ_leds_debug_toggle_t;
typedef opensim_repl_hdr_t   opensim_repl_leds_debug_toggle_t;
// debug_isOn
typedef opensim_requ_hdr_t   opensim_requ_leds_debug_isOn_t;
typedef opensim_repl_hdr_t   opensim_repl_leds_debug_isOn_t;
// all_on
typedef opensim_requ_hdr_t   opensim_requ_leds_all_on_t;
typedef opensim_repl_hdr_t   opensim_repl_leds_all_on_t;
// all_off
typedef opensim_requ_hdr_t   opensim_requ_leds_all_off_t;
typedef opensim_repl_hdr_t   opensim_repl_leds_all_off_t;
// all_toggle
typedef opensim_requ_hdr_t   opensim_requ_leds_all_toggle_t;
typedef opensim_repl_hdr_t   opensim_repl_leds_all_toggle_t;
// circular_shift
typedef opensim_requ_hdr_t   opensim_requ_leds_circular_shift_t;
typedef opensim_repl_hdr_t   opensim_repl_leds_circular_shift_t;
// increment
typedef opensim_requ_hdr_t   opensim_requ_leds_increment_t;
typedef opensim_repl_hdr_t   opensim_repl_leds_increment_t;
//=== radio
// init
typedef opensim_requ_hdr_t   opensim_requ_radio_init_t;
typedef opensim_repl_hdr_t   opensim_repl_radio_init_t;
// reset
typedef opensim_requ_hdr_t   opensim_requ_radio_reset_t;
typedef opensim_repl_hdr_t   opensim_repl_radio_reset_t;
// startTimer
typedef opensim_requ_hdr_t   opensim_requ_radio_startTimer_t;
typedef opensim_repl_hdr_t   opensim_repl_radio_startTimer_t;
// getTimerValue
typedef opensim_requ_hdr_t   opensim_requ_radio_getTimerValue_t;
typedef opensim_repl_hdr_t   opensim_repl_radio_getTimerValue_t;
// setTimerPeriod
typedef opensim_requ_hdr_t   opensim_requ_radio_setTimerPeriod_t;
typedef opensim_repl_hdr_t   opensim_repl_radio_setTimerPeriod_t;
// getTimerPeriod
typedef opensim_requ_hdr_t   opensim_requ_adio_getTimerPeriod_t;
typedef opensim_repl_hdr_t   opensim_repl_adio_getTimerPeriod_t;
// setFrequency
typedef opensim_requ_hdr_t   opensim_requ_radio_setFrequency_t;
typedef opensim_repl_hdr_t   opensim_repl_radio_setFrequency_t;
// rfOn
typedef opensim_requ_hdr_t   opensim_requ_radio_rfOn_t;
typedef opensim_repl_hdr_t   opensim_repl_radio_rfOn_t;
// rfOff
typedef opensim_requ_hdr_t   opensim_requ_radio_rfOff_t;
typedef opensim_repl_hdr_t   opensim_repl_radio_rfOff_t;
// loadPacket
typedef opensim_requ_hdr_t   opensim_requ_radio_loadPacket_t;
typedef opensim_repl_hdr_t   opensim_repl_radio_loadPacket_t;
// txEnable
typedef opensim_requ_hdr_t   opensim_requ_radio_txEnable_t;
typedef opensim_repl_hdr_t   opensim_repl_radio_txEnable_t;
// txNow
typedef opensim_requ_hdr_t   opensim_requ_radio_txNow_t;
typedef opensim_repl_hdr_t   opensim_repl_radio_txNow_t;
// rxEnable
typedef opensim_requ_hdr_t   opensim_requ_radio_rxEnable_t;
typedef opensim_repl_hdr_t   opensim_repl_radio_rxEnable_t;
// rxNow
typedef opensim_requ_hdr_t   opensim_requ_radio_rxNow_t;
typedef opensim_repl_hdr_t   opensim_repl_radio_rxNow_t;
// getReceivedFrame
typedef opensim_requ_hdr_t   opensim_requ_radio_getReceivedFrame_t;
typedef opensim_repl_hdr_t   opensim_repl_radio_getReceivedFrame_t;
//=== radiotimer
// init
typedef opensim_requ_hdr_t   opensim_requ_radiotimer_init_t;
typedef opensim_repl_hdr_t   opensim_repl_radiotimer_init_t;
// start
typedef opensim_requ_hdr_t   opensim_requ_radiotimer_start_t;
typedef opensim_repl_hdr_t   opensim_repl_radiotimer_start_t;
// getValue
typedef opensim_requ_hdr_t   opensim_requ_radiotimer_getValue_t;
typedef opensim_repl_hdr_t   opensim_repl_radiotimer_getValue_t;
// setPeriod
typedef opensim_requ_hdr_t   opensim_requ_radiotimer_setPeriod_t;
typedef opensim_repl_hdr_t   opensim_repl_radiotimer_setPeriod_t;
// getPeriod
typedef opensim_requ_hdr_t   opensim_requ_radiotimer_getPeriod_t;
typedef opensim_repl_hdr_t   opensim_repl_radiotimer_getPeriod_t;
// schedule
typedef opensim_requ_hdr_t   opensim_requ_radiotimer_schedule_t;
typedef opensim_repl_hdr_t   opensim_repl_radiotimer_schedule_t;
// cancel
typedef opensim_requ_hdr_t   opensim_requ_radiotimer_cancel_t;
typedef opensim_repl_hdr_t   opensim_repl_radiotimer_cancel_t;
// getCapturedTime
typedef opensim_requ_hdr_t   opensim_requ_radiotimer_getCapturedTime_t;
typedef opensim_repl_hdr_t   opensim_repl_radiotimer_getCapturedTime_t;
//=== uart
// init
typedef opensim_requ_hdr_t   opensim_requ_uart_init_t;
typedef opensim_repl_hdr_t   opensim_repl_uart_init_t;
// enableInterrupts
typedef opensim_requ_hdr_t   opensim_requ_uart_enableInterrupts_t;
typedef opensim_repl_hdr_t   opensim_repl_uart_enableInterrupts_t;
// disableInterrupts
typedef opensim_requ_hdr_t   opensim_requ_uart_disableInterrupts_t;
typedef opensim_repl_hdr_t   opensim_repl_uart_disableInterrupts_t;
// clearRxInterrupts
typedef opensim_requ_hdr_t   opensim_requ_uart_clearRxInterrupts_t;
typedef opensim_repl_hdr_t   opensim_repl_uart_clearRxInterrupts_t;
// clearTxInterrupts
typedef opensim_requ_hdr_t   opensim_requ_uart_clearTxInterrupts_t;
typedef opensim_repl_hdr_t   opensim_repl_uart_clearTxInterrupts_t;
// writeByte
typedef opensim_requ_hdr_t   opensim_requ_uart_writeByte_t;
typedef opensim_repl_hdr_t   opensim_repl_uart_writeByte_t;
// readByte
typedef opensim_requ_hdr_t   opensim_requ_uart_readByte_t;
typedef opensim_repl_hdr_t   opensim_repl_uart_readByte_t;

//=========================== prototypes ======================================

#endif