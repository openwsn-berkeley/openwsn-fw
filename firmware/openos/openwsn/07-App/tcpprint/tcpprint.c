#include "openwsn.h"
#include "tcpprint.h"
#include "openserial.h"
#include "openqueue.h"
#include "opentcp.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void tcpprint_init() {
}

bool tcpprint_shouldIlisten(){
   return TRUE;
}

void tcpprint_receive(OpenQueueEntry_t* msg) {
   openserial_printData((uint8_t*)(msg->payload),msg->length);
   //close TCP session, but keep listening
   opentcp_close();
   openqueue_freePacketBuffer(msg);
}

void tcpprint_connectDone(error_t error) {
}

void tcpprint_sendDone(OpenQueueEntry_t* msg, error_t error) {
}

bool tcpprint_debugPrint() {
   return FALSE;
}

//=========================== private =========================================