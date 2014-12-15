/**
\brief Source code of the Python openwsn module, written in C.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, May 2013.
*/

#ifndef __OPENWSNMODULE_H
#define __OPENWSNMODULE_H

// Python
#include <Python.h>
#include "structmember.h"
// OpenWSN
#include "openserial_obj.h"
#include "opentimers_obj.h"
#include "scheduler_obj.h"
#include "IEEE802154E_obj.h"
#include "adaptive_sync_obj.h"
#include "neighbors_obj.h"
#include "processIE_obj.h"
#include "sixtop_obj.h"
#include "schedule_obj.h"
#include "icmpv6echo_obj.h"
#include "icmpv6rpl_obj.h"
#include "opencoap_obj.h"
#include "opentcp_obj.h"
#include "idmanager_obj.h"
#include "openqueue_obj.h"
#include "openrandom_obj.h"
// applications
#include "c6t_obj.h"
#include "cexample_obj.h"
#include "cinfo_obj.h"
#include "cleds_obj.h"
#include "cstorm_obj.h"
#include "cwellknown_obj.h"
#include "rrt_obj.h"
#include "techo_obj.h"
//#include "tohlone_obj.h"
//#include "tohlone_obj.h"
#include "uecho_obj.h"

// notifications sent from the C mote to the Python BSP
enum {
   // board
   MOTE_NOTIF_board_init = 0,
   MOTE_NOTIF_board_sleep,
   MOTE_NOTIF_board_reset,
   // bsp_timer
   MOTE_NOTIF_bsp_timer_init,
   MOTE_NOTIF_bsp_timer_reset,
   MOTE_NOTIF_bsp_timer_scheduleIn,
   MOTE_NOTIF_bsp_timer_cancel_schedule,
   MOTE_NOTIF_bsp_timer_get_currentValue,
   // debugpins
   MOTE_NOTIF_debugpins_init,
   MOTE_NOTIF_debugpins_frame_toggle,
   MOTE_NOTIF_debugpins_frame_clr,
   MOTE_NOTIF_debugpins_frame_set,
   MOTE_NOTIF_debugpins_slot_toggle,
   MOTE_NOTIF_debugpins_slot_clr,
   MOTE_NOTIF_debugpins_slot_set,
   MOTE_NOTIF_debugpins_fsm_toggle,
   MOTE_NOTIF_debugpins_fsm_clr,
   MOTE_NOTIF_debugpins_fsm_set,
   MOTE_NOTIF_debugpins_task_toggle,
   MOTE_NOTIF_debugpins_task_clr,
   MOTE_NOTIF_debugpins_task_set,
   MOTE_NOTIF_debugpins_isr_toggle,
   MOTE_NOTIF_debugpins_isr_clr,
   MOTE_NOTIF_debugpins_isr_set,
   MOTE_NOTIF_debugpins_radio_toggle,
   MOTE_NOTIF_debugpins_radio_clr,
   MOTE_NOTIF_debugpins_radio_set,
   MOTE_NOTIF_debugpins_ka_clr,
   MOTE_NOTIF_debugpins_ka_set,
   MOTE_NOTIF_debugpins_syncPacket_clr,
   MOTE_NOTIF_debugpins_syncPacket_set,
   MOTE_NOTIF_debugpins_syncAck_clr,
   MOTE_NOTIF_debugpins_syncAck_set,
   MOTE_NOTIF_debugpins_debug_clr,
   MOTE_NOTIF_debugpins_debug_set,
   // eui64
   MOTE_NOTIF_eui64_get,
   // leds
   MOTE_NOTIF_leds_init,
   MOTE_NOTIF_leds_error_on,
   MOTE_NOTIF_leds_error_off,
   MOTE_NOTIF_leds_error_toggle,
   MOTE_NOTIF_leds_error_isOn,
   MOTE_NOTIF_leds_error_blink,
   MOTE_NOTIF_leds_radio_on,
   MOTE_NOTIF_leds_radio_off,
   MOTE_NOTIF_leds_radio_toggle,
   MOTE_NOTIF_leds_radio_isOn,
   MOTE_NOTIF_leds_sync_on,
   MOTE_NOTIF_leds_sync_off,
   MOTE_NOTIF_leds_sync_toggle,
   MOTE_NOTIF_leds_sync_isOn,
   MOTE_NOTIF_leds_debug_on,
   MOTE_NOTIF_leds_debug_off,
   MOTE_NOTIF_leds_debug_toggle,
   MOTE_NOTIF_leds_debug_isOn,
   MOTE_NOTIF_leds_all_on,
   MOTE_NOTIF_leds_all_off,
   MOTE_NOTIF_leds_all_toggle,
   MOTE_NOTIF_leds_circular_shift,
   MOTE_NOTIF_leds_increment,
   // radio
   MOTE_NOTIF_radio_init,
   MOTE_NOTIF_radio_reset,
   MOTE_NOTIF_radio_startTimer,
   MOTE_NOTIF_radio_getTimerValue,
   MOTE_NOTIF_radio_setTimerPeriod,
   MOTE_NOTIF_radio_getTimerPeriod,
   MOTE_NOTIF_radio_setFrequency,
   MOTE_NOTIF_radio_rfOn,
   MOTE_NOTIF_radio_rfOff,
   MOTE_NOTIF_radio_loadPacket,
   MOTE_NOTIF_radio_txEnable,
   MOTE_NOTIF_radio_txNow,
   MOTE_NOTIF_radio_rxEnable,
   MOTE_NOTIF_radio_rxNow,
   MOTE_NOTIF_radio_getReceivedFrame,
   // radiotimer
   MOTE_NOTIF_radiotimer_init,
   MOTE_NOTIF_radiotimer_start,
   MOTE_NOTIF_radiotimer_getValue,
   MOTE_NOTIF_radiotimer_setPeriod,
   MOTE_NOTIF_radiotimer_getPeriod,
   MOTE_NOTIF_radiotimer_schedule,
   MOTE_NOTIF_radiotimer_cancel,
   MOTE_NOTIF_radiotimer_getCapturedTime,
   // uart
   MOTE_NOTIF_uart_init,
   MOTE_NOTIF_uart_enableInterrupts,
   MOTE_NOTIF_uart_disableInterrupts,
   MOTE_NOTIF_uart_clearRxInterrupts,
   MOTE_NOTIF_uart_clearTxInterrupts,
   MOTE_NOTIF_uart_writeByte,
   MOTE_NOTIF_uart_writeCircularBuffer_FASTSIM,
   MOTE_NOTIF_uart_writeBufferByLen_FASTSIM,
   MOTE_NOTIF_uart_readByte,
   // last
   MOTE_NOTIF_LAST
};

typedef void (*uart_tx_cbt)(OpenMote* self);
typedef void (*uart_rx_cbt)(OpenMote* self);

typedef struct {
   uart_tx_cbt     txCb;
   uart_rx_cbt     rxCb;
} uart_icb_t;

typedef void (*bsp_timer_cbt)(OpenMote* self);

typedef struct {
   bsp_timer_cbt   cb;
} bsp_timer_icb_t;

typedef void (*radiotimer_capture_cbt)(OpenMote* self, PORT_TIMER_WIDTH timestamp);

typedef struct {
   radiotimer_capture_cbt    startFrame_cb;
   radiotimer_capture_cbt    endFrame_cb;
} radio_icb_t;

typedef void (*radiotimer_compare_cbt)(OpenMote* self);

typedef struct {
   radiotimer_compare_cbt    overflow_cb;
   radiotimer_compare_cbt    compare_cb;
} radiotimer_icb_t;

/**
\brief Memory footprint of an OpenMote instance.
*/
struct OpenMote {
   PyObject_HEAD // No ';' allows since in macro
   //===== callbacks to Python
   PyObject*            callback[MOTE_NOTIF_LAST];
   //===== internal C callbacks
   uart_icb_t           uart_icb;
   bsp_timer_icb_t      bsp_timer_icb;
   radio_icb_t          radio_icb;
   radiotimer_icb_t     radiotimer_icb;
   //===== openstack
   // l4
   icmpv6echo_vars_t    icmpv6echo_vars;
   icmpv6rpl_vars_t     icmpv6rpl_vars;
   opencoap_vars_t      opencoap_vars;
   tcp_vars_t           tcp_vars;
   // l3
   // l2b
   sixtop_vars_t        sixtop_vars;
   neighbors_vars_t     neighbors_vars;
   schedule_vars_t      schedule_vars;
   // l2a
   adaptive_sync_vars_t adaptive_sync_vars;
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
   //===== openapps
   c6t_vars_t           c6t_vars;
   cexample_vars_t      cexample_vars;
   cinfo_vars_t         cinfo_vars;
   cleds_vars_t         cleds_vars;
   cstorm_vars_t        cstorm_vars;
   cwellknown_vars_t    cwellknown_vars;
   rrt_vars_t           rrt_vars;
   //tohlone_vars_t       tohlone_vars;
};

#endif