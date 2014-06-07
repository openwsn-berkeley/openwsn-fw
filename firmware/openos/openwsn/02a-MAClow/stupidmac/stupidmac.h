/**
\brief Implementation of stupidMAC

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, August 2010
*/

#ifndef __STUPIDMAC_H
#define __STUPIDMAC_H

#include "openwsn.h"

enum {
   //default
   S_IDLE_LISTENING      =     0,
   //transmitter
   S_TX_TXDATAPREPARE    =     1,
   S_TX_TXDATAREADY      =     2,
   S_TX_TXDATA           =     3,
   S_TX_RXACK            =     4,
   //receiver
   S_RX_TXACKPREPARE     =     5,
   S_RX_TXACKREADY       =     6,
   S_RX_TXACK            =     7,
};

enum {
   IMMEDIATELY           =     1, //used as timer value which is very small
   WATCHDOG_PREPARESEND  = 16000, //500ms
};

enum {
   WAS_ACKED             =   TRUE,
   WAS_NOT_ACKED         =  FALSE,
};

//timer wait times (in 1/32768 seconds), slow version
/*enum {
   PERIODICTIMERPERIOD   =   9828, // 300ms
   MINBACKOFF            =   6552, // 200ms
   ACK_WAIT_TIME         =   3276, // 100ms
};*/

//timer wait times (in 1/32768 seconds), fast version
enum {
   PERIODICTIMERPERIOD   =    982, // 30ms
   MINBACKOFF            =    655, // 20ms
   ACK_WAIT_TIME         =    327, // 10ms
};

void    stupidmac_init();
error_t stupidmac_send(OpenQueueEntry_t* msg);
void    stupidmac_sendDone(OpenQueueEntry_t* msg, error_t error);
void    stupidmac_packet_received(OpenQueueEntry_t* pkt);
void    stupidmac_sendDone(OpenQueueEntry_t* packetReceived, error_t error);
void    timer_mac_backoff_fired();
void    timer_mac_watchdog_fired();
void    timer_mac_periodic_fired();
bool    stupidmac_debugPrint();

#endif
