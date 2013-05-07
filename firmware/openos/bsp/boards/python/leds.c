/**
\brief PC-specific definition of the "leds" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, April 2012.
*/

#include "leds.h"

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void leds_init() {
   
   // send request to server and get reply
   /*
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_leds_init,
                                    0,
                                    0,
                                    0,
                                    0);
   */
   // TODO: replace by call to Python
}

void leds_error_on() {
   
   // send request to server and get reply
   /*
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_leds_error_on,
                                    0,
                                    0,
                                    0,
                                    0);
   */
   // TODO: replace by call to Python
}
void leds_error_off() {
   
   // send request to server and get reply
   /*
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_leds_error_off,
                                    0,
                                    0,
                                    0,
                                    0);
   */
   // TODO: replace by call to Python
}
void leds_error_toggle() {
   
   // send request to server and get reply
   /*
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_leds_error_toggle,
                                    0,
                                    0,
                                    0,
                                    0);
   */
   // TODO: replace by call to Python
}
uint8_t leds_error_isOn() {
   //opensim_repl_error_isOn_t replparams;
   
   // send request to server and get reply
   /*
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_leds_error_isOn,
                                    0,
                                    0,
                                    &replparams,
                                    sizeof(opensim_repl_error_isOn_t));
   */
   // TODO: replace by call to Python
   
   //return replparams.isOn;
   return 0;//poipoi
}
void leds_error_blink() {
   
   // send request to server and get reply
   /*
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_leds_error_blink,
                                    0,
                                    0,
                                    0,
                                    0);
   */
   // TODO: replace by call to Python
}

void leds_radio_on() {
   
   // send request to server and get reply
   /*
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_leds_radio_on,
                                    0,
                                    0,
                                    0,
                                    0);
   */
   // TODO: replace by call to Python
}
void leds_radio_off() {
   
   // send request to server and get reply
   /*
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_leds_radio_off,
                                    0,
                                    0,
                                    0,
                                    0);
   */
   // TODO: replace by call to Python
}
void leds_radio_toggle() {
   
   // send request to server and get reply
   /*
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_leds_radio_toggle,
                                    0,
                                    0,
                                    0,
                                    0);
   */
   // TODO: replace by call to Python
}
uint8_t leds_radio_isOn() {
   //opensim_repl_radio_isOn_t replparams;
   
   // send request to server and get reply
   /*
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_leds_radio_isOn,
                                    0,
                                    0,
                                    &replparams,
                                    sizeof(opensim_repl_radio_isOn_t));
   */
   // TODO: replace by call to Python
   
   //return replparams.isOn;
   return 0;//poipoi
}

// green
void leds_sync_on() {
   
   // send request to server and get reply
   /*
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_leds_sync_on,
                                    0,
                                    0,
                                    0,
                                    0);
   */
   // TODO: replace by call to Python
}
void leds_sync_off() {
   
   // send request to server and get reply
   /*
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_leds_sync_off,
                                    0,
                                    0,
                                    0,
                                    0);
   */
   // TODO: replace by call to Python
}
void leds_sync_toggle() {
   
   // send request to server and get reply
   /*
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_leds_sync_toggle,
                                    0,
                                    0,
                                    0,
                                    0);
   */
   // TODO: replace by call to Python
}
uint8_t leds_sync_isOn() {
   //opensim_repl_sync_isOn_t replparams;
   
   // send request to server and get reply
   /*
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_leds_sync_isOn,
                                    0,
                                    0,
                                    &replparams,
                                    sizeof(opensim_repl_sync_isOn_t));
   */
   // TODO: replace by call to Python
   
   //return replparams.isOn;
   return 0;//poipoi
}

// yellow
void leds_debug_on() {
   
   // send request to server and get reply
   /*
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_leds_debug_on,
                                    0,
                                    0,
                                    0,
                                    0);
   */
   // TODO: replace by call to Python
}
void leds_debug_off() {
   
   // send request to server and get reply
   /*
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_leds_debug_off,
                                    0,
                                    0,
                                    0,
                                    0);
   */
   // TODO: replace by call to Python
}
void leds_debug_toggle() {
   
   // send request to server and get reply
   /*
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_leds_debug_toggle,
                                    0,
                                    0,
                                    0,
                                    0);
   */
   // TODO: replace by call to Python
}
uint8_t leds_debug_isOn() {
   //opensim_repl_debug_isOn_t replparams;
   
   // send request to server and get reply
   /*
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_leds_debug_isOn,
                                    0,
                                    0,
                                    &replparams,
                                    sizeof(opensim_repl_debug_isOn_t));
   */
   // TODO: replace by call to Python
   
   //return replparams.isOn;
   return 0;//poipoi
}

void leds_all_on() {
   
   // send request to server and get reply
   /*
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_leds_all_on,
                                    0,
                                    0,
                                    0,
                                    0);
   */
   // TODO: replace by call to Python
}
void leds_all_off() {
   
   // send request to server and get reply
   /*
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_leds_all_off,
                                    0,
                                    0,
                                    0,
                                    0);
   */
   // TODO: replace by call to Python
}
void leds_all_toggle() {
   
   // send request to server and get reply
   /*
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_leds_all_toggle,
                                    0,
                                    0,
                                    0,
                                    0);
   */
   // TODO: replace by call to Python
}

void leds_circular_shift() {
   
   // send request to server and get reply
   /*
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_leds_circular_shift,
                                    0,
                                    0,
                                    0,
                                    0);
   */
   // TODO: replace by call to Python
}

void leds_increment() {
   
   // send request to server and get reply
   /*
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_leds_increment,
                                    0,
                                    0,
                                    0,
                                    0);
   */
   // TODO: replace by call to Python
}

//=========================== private =========================================