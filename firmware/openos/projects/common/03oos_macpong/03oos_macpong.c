/**
\brief This project runs the full OpenWSN stack.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, August 2010
*/

#include "openwsn.h"
//board
#include "board.h"
#include "leds.h"
//openwsn
#include "openwsn.h"
#include "scheduler.h"
#include "packetfunctions.h"
#include "openqueue.h"
#include "IEEE802154E.h"
#include "idmanager.h"
#include "openserial.h"



//=========================== initialization ==================================

int mote_main(void) {
   board_init();
   scheduler_init();
   openwsn_init();
   scheduler_start();
   return 0; // this line should never be reached
}

//=========================== IPHC spoofing ===================================

void iphc_sendDone(OpenQueueEntry_t* msg, error_t error) {
   msg->owner = COMPONENT_IPHC;
   openqueue_freePacketBuffer(msg);
}

void iphc_receive(OpenQueueEntry_t* msg) {
   msg->owner = COMPONENT_IPHC;
   openqueue_freePacketBuffer(msg);
}

void tcpinject_trigger() {
}
void udpinject_trigger() {
}
void icmpv6echo_trigger() {
}
void icmpv6router_trigger() {
}
void icmpv6rpl_trigger() {
}
void openbridge_trigger() {
}

void openbridge_init() {
}
void iphc_init() {
}
void forwarding_init() {
}
void icmpv6_init() {
}
void icmpv6echo_init() {
}
void icmpv6router_init() {
}
void icmpv6rpl_init() {
}
void opentcp_init() {
}
void openudp_init() {
}
void opencoap_init() {
}
void tcpecho_init() {
}
void tcpinject_init() {
}
void tcpprint_init() {
}
void ohlone_init() {
}
void udpecho_init() {
}
void udpinject_init() {
}
void udpprint_init() {
}
void udprand_init() {
}