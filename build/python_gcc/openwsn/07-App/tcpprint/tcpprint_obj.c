/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:11:11.906309.
*/
#include "openwsn_obj.h"
#include "tcpprint_obj.h"
#include "openserial_obj.h"
#include "openqueue_obj.h"
#include "opentcp_obj.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void tcpprint_init(OpenMote* self) {
}

bool tcpprint_shouldIlisten(OpenMote* self){
   return TRUE;
}

void tcpprint_receive(OpenMote* self, OpenQueueEntry_t* msg) {
 openserial_printData(self, (uint8_t*)(msg->payload),msg->length);
   //close TCP session, but keep listening
 opentcp_close(self);
 openqueue_freePacketBuffer(self, msg);
}

void tcpprint_connectDone(OpenMote* self, owerror_t error) {
}

void tcpprint_sendDone(OpenMote* self, OpenQueueEntry_t* msg, owerror_t error) {
}

bool tcpprint_debugPrint(OpenMote* self) {
   return FALSE;
}

//=========================== private =========================================