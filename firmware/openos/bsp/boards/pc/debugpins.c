/**
\brief PC-specific definition of the "debugpins" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, April 2012.
*/

#include "debugpins.h"
#include "opensim_proto.h"

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void debugpins_init() {
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_debugpins_init,
                                    0,
                                    0,
                                    0,
                                    0);
}

void debugpins_frame_toggle() {
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_debugpins_frame_toggle,
                                    0,
                                    0,
                                    0,
                                    0);
}
void debugpins_frame_clr() {
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_debugpins_frame_clr,
                                    0,
                                    0,
                                    0,
                                    0);
}
void debugpins_frame_set() {
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_debugpins_frame_set,
                                    0,
                                    0,
                                    0,
                                    0);
}

void debugpins_slot_toggle() {
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_debugpins_slot_toggle,
                                    0,
                                    0,
                                    0,
                                    0);
}
void debugpins_slot_clr() {
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_debugpins_slot_clr,
                                    0,
                                    0,
                                    0,
                                    0);
}
void debugpins_slot_set() {
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_debugpins_slot_set,
                                    0,
                                    0,
                                    0,
                                    0);
}

void debugpins_fsm_toggle() {
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_debugpins_fsm_toggle,
                                    0,
                                    0,
                                    0,
                                    0);
}
void debugpins_fsm_clr() {
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_debugpins_fsm_clr,
                                    0,
                                    0,
                                    0,
                                    0);
}
void debugpins_fsm_set() {
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_debugpins_fsm_set,
                                    0,
                                    0,
                                    0,
                                    0);
}

void debugpins_task_toggle() {
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_debugpins_task_toggle,
                                    0,
                                    0,
                                    0,
                                    0);
}
void debugpins_task_clr() {
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_debugpins_task_clr,
                                    0,
                                    0,
                                    0,
                                    0);
}
void debugpins_task_set() {
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_debugpins_task_set,
                                    0,
                                    0,
                                    0,
                                    0);
}

void debugpins_isr_toggle() {
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_debugpins_isr_toggle,
                                    0,
                                    0,
                                    0,
                                    0);
}
void debugpins_isr_clr() {
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_debugpins_isr_clr,
                                    0,
                                    0,
                                    0,
                                    0);
}
void debugpins_isr_set() {
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_debugpins_isr_set,
                                    0,
                                    0,
                                    0,
                                    0);
}

void debugpins_radio_toggle() {
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_debugpins_radio_toggle,
                                    0,
                                    0,
                                    0,
                                    0);
}
void debugpins_radio_clr() {
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_debugpins_radio_clr,
                                    0,
                                    0,
                                    0,
                                    0);
}
void debugpins_radio_set() {
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_debugpins_radio_set,
                                    0,
                                    0,
                                    0,
                                    0);
}