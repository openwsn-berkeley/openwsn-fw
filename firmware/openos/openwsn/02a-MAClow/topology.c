#include "openwsn.h"
#include "topology.h"
#include "idmanager.h"

//=========================== defines =========================================

#define TOPOLOGY_MOTE1 0x41
#define TOPOLOGY_MOTE2 0xB9
#define TOPOLOGY_MOTE3 0xEE
#define TOPOLOGY_MOTE4 0x80
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
         if (ieee802514_header->src.addr_64b[7]==TOPOLOGY_MOTE2 ||
             ieee802514_header->src.addr_64b[7]==TOPOLOGY_MOTE4) {
            returnVal=TRUE;
         } else {
            returnVal=FALSE;
         }
         break;
       case TOPOLOGY_MOTE4:
         if (ieee802514_header->src.addr_64b[7]==TOPOLOGY_MOTE3) {
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