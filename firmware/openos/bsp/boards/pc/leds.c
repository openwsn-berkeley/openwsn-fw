/**
\brief PC-specific definition of the "leds" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, April 2012.
*/

#include "leds.h"
#include "opensim_proto.h"

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void leds_init() {
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_leds_init,
                                    0,
                                    0,
                                    0,
                                    0);
}

void    leds_error_on() {
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_leds_error_on,
                                    0,
                                    0,
                                    0,
                                    0);
}
void    leds_error_off() {
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_leds_error_off,
                                    0,
                                    0,
                                    0,
                                    0);
}
void    leds_error_toggle() {
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_leds_error_toggle,
                                    0,
                                    0,
                                    0,
                                    0);
}
uint8_t leds_error_isOn() {
   // poipoipoipoi
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_leds_error_isOn,
                                    0,
                                    0,
                                    0,
                                    0);
}

void    leds_radio_on() {
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_leds_radio_on,
                                    0,
                                    0,
                                    0,
                                    0);
}
void    leds_radio_off() {
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_leds_radio_off,
                                    0,
                                    0,
                                    0,
                                    0);
}
void    leds_radio_toggle() {
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_leds_radio_toggle,
                                    0,
                                    0,
                                    0,
                                    0);
}
uint8_t leds_radio_isOn() {
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_leds_radio_isOn,
                                    0,
                                    0,
                                    0,
                                    0);
   // poipoi
}

// green
void    leds_sync_on() {
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_leds_sync_on,
                                    0,
                                    0,
                                    0,
                                    0);
}
void    leds_sync_off() {
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_leds_sync_off,
                                    0,
                                    0,
                                    0,
                                    0);
}
void    leds_sync_toggle() {
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_leds_sync_toggle,
                                    0,
                                    0,
                                    0,
                                    0);
}
uint8_t leds_sync_isOn() {
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_leds_sync_isOn,
                                    0,
                                    0,
                                    0,
                                    0);
}

// yellow
void    leds_debug_on() {
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_leds_debug_on,
                                    0,
                                    0,
                                    0,
                                    0);
}
void    leds_debug_off() {
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_leds_debug_off,
                                    0,
                                    0,
                                    0,
                                    0);
}
void    leds_debug_toggle() {
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_leds_debug_toggle,
                                    0,
                                    0,
                                    0,
                                    0);
}
uint8_t leds_debug_isOn() {
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_leds_debug_isOn,
                                    0,
                                    0,
                                    0,
                                    0);
   // poipoipoi
}

void leds_all_on() {
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_leds_all_on,
                                    0,
                                    0,
                                    0,
                                    0);
}
void leds_all_off() {
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_leds_all_off,
                                    0,
                                    0,
                                    0,
                                    0);
}
void leds_all_toggle() {
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_leds_all_toggle,
                                    0,
                                    0,
                                    0,
                                    0);
}

void leds_circular_shift() {
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_leds_circular_shift,
                                    0,
                                    0,
                                    0,
                                    0);
}

void leds_increment() {
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_leds_increment,
                                    0,
                                    0,
                                    0,
                                    0);
}

//=========================== private =========================================