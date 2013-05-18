/**
\brief Source code of the Python openwsn module, written in C.
*/

#ifndef __OPENWSNMODULE_H
#define __OPENWSNMODULE_H

// Python
#include <Python.h>
#include "structmember.h"
// OpenWSN
#include "openserial.h"
#include "opentimers.h"
#include "scheduler.h"
#include "IEEE802154E.h"
#include "neighbors.h"
#include "res.h"
#include "schedule.h"
#include "icmpv6echo.h"
#include "icmpv6rpl.h"
#include "opencoap.h"
#include "opentcp.h"
#include "ohlone.h"
#include "tcpinject.h"
#include "idmanager.h"
#include "openqueue.h"
#include "openrandom.h"

enum {
   //===== from client to server
   // board
   OPENSIM_CMD_board_init = 0,
   OPENSIM_CMD_board_sleep,
   OPENSIM_CMD_board_reset,
   // bsp_timer
   OPENSIM_CMD_bsp_timer_init,
   OPENSIM_CMD_bsp_timer_reset,
   OPENSIM_CMD_bsp_timer_scheduleIn,
   OPENSIM_CMD_bsp_timer_cancel_schedule,
   OPENSIM_CMD_bsp_timer_get_currentValue,
   // debugpins
   OPENSIM_CMD_debugpins_init,
   OPENSIM_CMD_debugpins_frame_toggle,
   OPENSIM_CMD_debugpins_frame_clr,
   OPENSIM_CMD_debugpins_frame_set,
   OPENSIM_CMD_debugpins_slot_toggle,
   OPENSIM_CMD_debugpins_slot_clr,
   OPENSIM_CMD_debugpins_slot_set,
   OPENSIM_CMD_debugpins_fsm_toggle,
   OPENSIM_CMD_debugpins_fsm_clr,
   OPENSIM_CMD_debugpins_fsm_set,
   OPENSIM_CMD_debugpins_task_toggle,
   OPENSIM_CMD_debugpins_task_clr,
   OPENSIM_CMD_debugpins_task_set,
   OPENSIM_CMD_debugpins_isr_toggle,
   OPENSIM_CMD_debugpins_isr_clr,
   OPENSIM_CMD_debugpins_isr_set,
   OPENSIM_CMD_debugpins_radio_toggle,
   OPENSIM_CMD_debugpins_radio_clr,
   OPENSIM_CMD_debugpins_radio_set,
   // eui64
   OPENSIM_CMD_eui64_get,
   // leds
   OPENSIM_CMD_leds_init,
   OPENSIM_CMD_leds_error_on,
   OPENSIM_CMD_leds_error_off,
   OPENSIM_CMD_leds_error_toggle,
   OPENSIM_CMD_leds_error_isOn,
   OPENSIM_CMD_leds_error_blink,
   OPENSIM_CMD_leds_radio_on,
   OPENSIM_CMD_leds_radio_off,
   OPENSIM_CMD_leds_radio_toggle,
   OPENSIM_CMD_leds_radio_isOn,
   OPENSIM_CMD_leds_sync_on,
   OPENSIM_CMD_leds_sync_off,
   OPENSIM_CMD_leds_sync_toggle,
   OPENSIM_CMD_leds_sync_isOn,
   OPENSIM_CMD_leds_debug_on,
   OPENSIM_CMD_leds_debug_off,
   OPENSIM_CMD_leds_debug_toggle,
   OPENSIM_CMD_leds_debug_isOn,
   OPENSIM_CMD_leds_all_on,
   OPENSIM_CMD_leds_all_off,
   OPENSIM_CMD_leds_all_toggle,
   OPENSIM_CMD_leds_circular_shift,
   OPENSIM_CMD_leds_increment,
   // radio
   OPENSIM_CMD_radio_init,
   OPENSIM_CMD_radio_reset,
   OPENSIM_CMD_radio_startTimer,
   OPENSIM_CMD_radio_getTimerValue,
   OPENSIM_CMD_radio_setTimerPeriod,
   OPENSIM_CMD_radio_getTimerPeriod,
   OPENSIM_CMD_radio_setFrequency,
   OPENSIM_CMD_radio_rfOn,
   OPENSIM_CMD_radio_rfOff,
   OPENSIM_CMD_radio_loadPacket,
   OPENSIM_CMD_radio_txEnable,
   OPENSIM_CMD_radio_txNow,
   OPENSIM_CMD_radio_rxEnable,
   OPENSIM_CMD_radio_rxNow,
   OPENSIM_CMD_radio_getReceivedFrame,
   // radiotimer
   OPENSIM_CMD_radiotimer_init,
   OPENSIM_CMD_radiotimer_start,
   OPENSIM_CMD_radiotimer_getValue,
   OPENSIM_CMD_radiotimer_setPeriod,
   OPENSIM_CMD_radiotimer_getPeriod,
   OPENSIM_CMD_radiotimer_schedule,
   OPENSIM_CMD_radiotimer_cancel,
   OPENSIM_CMD_radiotimer_getCapturedTime,
   // uart
   OPENSIM_CMD_uart_init,
   OPENSIM_CMD_uart_enableInterrupts,
   OPENSIM_CMD_uart_disableInterrupts,
   OPENSIM_CMD_uart_clearRxInterrupts,
   OPENSIM_CMD_uart_clearTxInterrupts,
   OPENSIM_CMD_uart_writeByte,
   OPENSIM_CMD_uart_readByte,
   // last
   OPENSIM_CMD_LAST
};

/**
\brief Memory footprint of an OpenMote instance.
*/
struct OpenMote {
   PyObject_HEAD // No ';' allows since in macro
   //===== callbacks
   PyObject*            callback[OPENSIM_CMD_LAST];
   //===== state
   // l7
   ohlone_vars_t        ohlone_vars;
   tcpinject_vars_t     tcpinject_vars;
   // l4
   icmpv6echo_vars_t    icmpv6echo_vars;
   icmpv6rpl_vars_t     icmpv6rpl_vars;
   opencoap_vars_t      opencoap_vars;
   tcp_vars_t           tcp_vars;
   // l3
   // l2b
   neighbors_vars_t     neighbors_vars;
   res_vars_t           res_vars;
   schedule_vars_t      schedule_vars;
   schedule_dbg_t       schedule_dbg;
   // l2a
   ieee154e_vars_t      ieee154e_vars;
   ieee154e_stats_t     ieee154e_stats;
   ieee154e_dbg_t       ieee154e_dbg;
   // cross-layer
   idmanager_vars_t     idmanager_vars;
   openqueue_vars_t     openqueue_vars;
   // drivers
   opentimers_vars_t    opentimers_vars;
   random_vars_t        random_vars;
   openserial_vars_t    openserial_vars;
   // kernel
   scheduler_vars_t     scheduler_vars;
   scheduler_dbg_t      scheduler_dbg;
};

#endif