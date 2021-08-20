#ifndef __USENDPACKET_H
#define __USENDPACKET_H

/**
\addtogroup AppUdp
\{
\addtogroup usendpacket
\{
*/

//#include "opentimers.h"
//#include "openudp.h"
#include "opentimers.h"
#include "../../openstack/04-TRAN/openudp.h"

//=========================== define ==========================================

#define USENDPACKET_PERIOD_MS 5000

//=========================== typedef =========================================

//=========================== variables =======================================

typedef struct {
    opentimers_id_t     timerId;   ///< periodic timer which triggers transmission
    uint16_t             counter;  ///< incrementing counter which is written into the packet
    uint16_t              period;  ///< uinject packet sending period>
    udp_resource_desc_t     desc;  ///< resource descriptor for this module, used to register at UDP stack
    bool      busySendingUsendpacket;  ///< TRUE when busy sending an uinject
} usendpacket_vars_t;
//=========================== prototypes ======================================

void usendpacket_init(void);
void usendpacket_sendDone(OpenQueueEntry_t* msg, owerror_t error);
void usendpacket_receive(OpenQueueEntry_t* msg);
/**
\}
\}
*/

#endif
