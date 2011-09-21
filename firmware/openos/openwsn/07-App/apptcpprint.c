#include "openwsn.h"
#include "apptcpprint.h"
#include "openserial.h"
#include "openqueue.h"
#include "tcp.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void apptcpprint_init() {
}

bool apptcpprint_shouldIlisten(){
   return TRUE;
}

void apptcpprint_receive(OpenQueueEntry_t* msg) {
   openserial_printData((uint8_t*)(msg->payload),msg->length);
   //close TCP session, but keep listening
   tcp_close();
   openqueue_freePacketBuffer(msg);
}

void apptcpprint_connectDone(error_t error) {
}

void apptcpprint_sendDone(OpenQueueEntry_t* msg, error_t error) {
}

bool apptcpprint_debugPrint() {
   return FALSE;
}

//=========================== private =========================================