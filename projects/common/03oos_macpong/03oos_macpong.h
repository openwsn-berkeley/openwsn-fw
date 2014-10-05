/**
\brief This project runs the full OpenWSN stack on the GINA2.2b/c and GINA
       basestations platforms.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, August 2010
*/

#ifndef __openwsn_H
#define __openwsn_H

#include "opendefs.h"
#include "iphc.h"

void bridge_sendDone(OpenQueueEntry_t* pkt, error_t error);
void bridge_receive(OpenQueueEntry_t* packetReceived);

#endif

