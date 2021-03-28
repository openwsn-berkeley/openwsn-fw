/**
\addtogroup BSP
\{
\addtogroup board
\{

\brief Cross-platform declaration "board" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
\author Timothy Claeys <timothy.claeys@inria.fr>, September 2020.
*/

#ifndef OPENWSN_INTERFACE_H
#define OPENWSN_INTERFACE_H

#include <Python.h>

//========================= typedefs ==========================================

//=========================== enums ===========================================

// notifications sent from the C mote to the Python BSP
enum {
    // board
    MOTE_NOTIF_board_init,
    MOTE_NOTIF_board_sleep,
    MOTE_NOTIF_board_reset,
    MOTE_NOTIF_board_slot_sync,
    MOTE_NOTIF_board_msg_sync,
    MOTE_NOTIF_board_ack_sync,
    // sctimer
    MOTE_NOTIF_sctimer_init,
    MOTE_NOTIF_sctimer_setCompare,
    MOTE_NOTIF_sctimer_set_callback,
    MOTE_NOTIF_sctimer_readCounter,
    MOTE_NOTIF_sctimer_enable,
    MOTE_NOTIF_sctimer_disable,
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
    MOTE_NOTIF_radio_setFrequency,
    MOTE_NOTIF_radio_rfOn,
    MOTE_NOTIF_radio_rfOff,
    MOTE_NOTIF_radio_loadPacket,
    MOTE_NOTIF_radio_txEnable,
    MOTE_NOTIF_radio_txNow,
    MOTE_NOTIF_radio_rxEnable,
    MOTE_NOTIF_radio_rxNow,
    MOTE_NOTIF_radio_getReceivedFrame,
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
    MOTE_NOTIF_uart_setCTS,
    // last
    MOTE_NOTIF_LAST
};

#if _WIN32
PyObject *callbacks[MOTE_NOTIF_LAST];
#else
extern PyObject *callbacks[MOTE_NOTIF_LAST];
#endif

// radio
void radio_intr_startOfFrame(uint32_t capturedTime);

void radio_intr_endOfFrame(uint32_t capturedTime);

// sctimer
void sctimer_intr_compare(void);

// uart
void uart_intr_tx(void);

void uart_intr_rx(void);

void uart_writeBufferByLen_FASTSIM(uint8_t *buffer, uint16_t);

#endif /* OPENWSN_OPENWSNMODULE_H */