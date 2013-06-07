#include "openwsn.h"
#include "topology.h"
#include "idmanager.h"

//=========================== defines =========================================

#define TOPOLOGY_MOTE1 0x83
#define TOPOLOGY_MOTE2 0xaa
#define TOPOLOGY_MOTE3 0xb6

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

bool topology_isAcceptablePacket(ieee802154_header_iht* ieee802514_header) {
   bool returnVal;
   switch (idmanager_getMyID(ADDR_64B)->addr_64b[7]) {
      case TOPOLOGY_MOTE1:
         if (ieee802514_header->src.addr_64b[7]==TOPOLOGY_MOTE2) {
            returnVal=TRUE;
         } else {
            returnVal=FALSE;
         }
         break;
      case TOPOLOGY_MOTE2:
         if (ieee802514_header->src.addr_64b[7]==TOPOLOGY_MOTE1 ||
             ieee802514_header->src.addr_64b[7]==TOPOLOGY_MOTE3) {
            returnVal=TRUE;
         } else {
            returnVal=FALSE;
         }
         break;
      case TOPOLOGY_MOTE3:
         if (ieee802514_header->src.addr_64b[7]==TOPOLOGY_MOTE2) {
            returnVal=TRUE;
         } else {
            returnVal=FALSE;
         }
         break;
      default:
         returnVal=TRUE;
   }
   return returnVal;
}

//=========================== private =========================================
