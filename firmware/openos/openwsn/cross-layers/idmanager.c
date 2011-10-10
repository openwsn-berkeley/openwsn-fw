#include "openwsn.h"
#include "idmanager.h"
#include "packetfunctions.h"
#include "openserial.h"
#include "neighbors.h"

//=========================== variables =======================================

typedef struct {
   bool          isDAGroot;
   bool          isBridge;
   open_addr_t   my16bID;
   open_addr_t   my64bID;
   open_addr_t   myPANID;
   open_addr_t   myPrefix;
} idmanager_vars_t;

idmanager_vars_t idmanager_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

void idmanager_init() {
   idmanager_vars.isDAGroot            = FALSE;
   idmanager_vars.isBridge             = FALSE;
   idmanager_vars.myPANID.type         = ADDR_PANID;
   idmanager_vars.myPANID.panid[0]     = 0x4e;
   idmanager_vars.myPANID.panid[1]     = 0xff;//poipoi hardcoded
   idmanager_vars.myPrefix.type        = ADDR_PREFIX;
   idmanager_vars.myPrefix.prefix[0]   = 0x00;
   idmanager_vars.myPrefix.prefix[1]   = 0x00;
   idmanager_vars.myPrefix.prefix[2]   = 0x00;
   idmanager_vars.myPrefix.prefix[3]   = 0x00;
   idmanager_vars.myPrefix.prefix[4]   = 0x00;
   idmanager_vars.myPrefix.prefix[5]   = 0x00;
   idmanager_vars.myPrefix.prefix[6]   = 0x00;
   idmanager_vars.myPrefix.prefix[7]   = 0x00;
   idmanager_vars.my64bID.type         = ADDR_64B;
   idmanager_vars.my64bID.addr_64b[0]  = *(&eui64+0);
   idmanager_vars.my64bID.addr_64b[1]  = *(&eui64+1);
   idmanager_vars.my64bID.addr_64b[2]  = *(&eui64+2);
   idmanager_vars.my64bID.addr_64b[3]  = *(&eui64+3);
   idmanager_vars.my64bID.addr_64b[4]  = *(&eui64+4);
   idmanager_vars.my64bID.addr_64b[5]  = *(&eui64+5);
   idmanager_vars.my64bID.addr_64b[6]  = *(&eui64+6);
   idmanager_vars.my64bID.addr_64b[7]  = *(&eui64+7);
   packetfunctions_mac64bToMac16b(&idmanager_vars.my64bID,&idmanager_vars.my16bID);
   
   // DEBUG_MOTEID_MASTER is DAGroot and bridge
   if (idmanager_vars.my16bID.addr_16b[1]==DEBUG_MOTEID_MASTER) {
      idmanager_vars.isDAGroot         = TRUE;
      idmanager_vars.isBridge          = TRUE;
   }
}

__monitor bool idmanager_getIsDAGroot() {
   return idmanager_vars.isDAGroot;
}

__monitor void idmanager_setIsDAGroot(bool newRole) {
   idmanager_vars.isDAGroot = newRole;
   neighbors_updateMyDAGrankAndNeighborPreference();
}

bool idmanager_getIsBridge() {
   return idmanager_vars.isBridge;
}

void idmanager_setIsBridge(bool newRole) {
   idmanager_vars.isBridge = newRole;
}

__monitor open_addr_t* idmanager_getMyID(uint8_t type) {
   switch (type) {
      case ADDR_16B:
         return &idmanager_vars.my16bID;
      case ADDR_64B:
         return &idmanager_vars.my64bID;
      case ADDR_PANID:
         return &idmanager_vars.myPANID;
      case ADDR_PREFIX:
         return &idmanager_vars.myPrefix;
      case ADDR_128B:
         //you don't ask for my full address, rather for prefix, then 64b
      default:
         openserial_printError(COMPONENT_IDMANAGER,ERR_WRONG_ADDR_TYPE,
                               (errorparameter_t)type,
                               (errorparameter_t)0);
         return NULL;
   }
}

__monitor error_t idmanager_setMyID(open_addr_t* newID) {
   switch (newID->type) {
      case ADDR_16B:
         memcpy(&idmanager_vars.my16bID,newID,sizeof(open_addr_t));
         break;
      case ADDR_64B:
         memcpy(&idmanager_vars.my64bID,newID,sizeof(open_addr_t));
         break;
      case ADDR_PANID:
         memcpy(&idmanager_vars.myPANID,newID,sizeof(open_addr_t));
         break;
      case ADDR_PREFIX:
         memcpy(&idmanager_vars.myPrefix,newID,sizeof(open_addr_t));
         break;
      case ADDR_128B:
         //don't set 128b, but rather prefix and 64b
      default:
         openserial_printError(COMPONENT_IDMANAGER,ERR_WRONG_ADDR_TYPE,
                               (errorparameter_t)newID->type,
                               (errorparameter_t)1);
         return E_FAIL;
   }
   return E_SUCCESS;
}

__monitor bool idmanager_isMyAddress(open_addr_t* addr) {
   open_addr_t temp_my128bID;
   switch (addr->type) {
      case ADDR_16B:
         return packetfunctions_sameAddress(addr,&idmanager_vars.my16bID);
      case ADDR_64B:
         return packetfunctions_sameAddress(addr,&idmanager_vars.my64bID);
      case ADDR_128B:
         //build temporary my128bID
         temp_my128bID.type = ADDR_128B;
         memcpy(&temp_my128bID.addr_128b[0],&idmanager_vars.myPrefix.prefix,8);
         memcpy(&temp_my128bID.addr_128b[8],&idmanager_vars.my64bID.addr_64b,8);
         return packetfunctions_sameAddress(addr,&temp_my128bID);
      case ADDR_PANID:
         return packetfunctions_sameAddress(addr,&idmanager_vars.myPANID);
      case ADDR_PREFIX:
         return packetfunctions_sameAddress(addr,&idmanager_vars.myPrefix);
      default:
         openserial_printError(COMPONENT_IDMANAGER,ERR_WRONG_ADDR_TYPE,
                               (errorparameter_t)addr->type,
                               (errorparameter_t)2);
         return FALSE;
   }
}

void idmanager_triggerAboutBridge() {
   uint8_t number_bytes_from_input_buffer;
   uint8_t input_buffer[9];
   //get command from OpenSerial (1B command, 8B prefix)
   number_bytes_from_input_buffer = openserial_getInputBuffer(&input_buffer[0],sizeof(input_buffer));
   if (number_bytes_from_input_buffer!=sizeof(input_buffer)) {
      openserial_printError(COMPONENT_IDMANAGER,ERR_INPUTBUFFER_LENGTH,
                            (errorparameter_t)number_bytes_from_input_buffer,
                            (errorparameter_t)0);
      return;
   };
   //handle command
   switch (input_buffer[0]) {
      case 'Y':
         idmanager_setIsBridge(TRUE);
         memcpy(&(idmanager_vars.myPrefix.prefix),&(input_buffer[1]),8);
         break;
      case 'N':
         idmanager_setIsBridge(FALSE);
         break;
      case 'T':
         if (idmanager_getIsBridge()) {
            idmanager_setIsBridge(FALSE);
         } else {
            idmanager_setIsBridge(TRUE);
            memcpy(&(idmanager_vars.myPrefix.prefix),&(input_buffer[1]),8);
         }
         break;
   }
   return;
}
void idmanager_triggerAboutRoot() {
   uint8_t number_bytes_from_input_buffer;
   uint8_t input_buffer;
   //get command from OpenSerial (16B IPv6 destination address, 2B destination port)
   number_bytes_from_input_buffer = openserial_getInputBuffer(&input_buffer,sizeof(input_buffer));
   if (number_bytes_from_input_buffer!=sizeof(input_buffer)) {
      openserial_printError(COMPONENT_IDMANAGER,ERR_INPUTBUFFER_LENGTH,
                            (errorparameter_t)number_bytes_from_input_buffer,
                            (errorparameter_t)0);
      return;
   };
   //handle command
   switch (input_buffer) {
      case 'Y':
         idmanager_setIsDAGroot(TRUE);
         break;
      case 'N':
         idmanager_setIsDAGroot(FALSE);
         break;
      case 'T':
         if (idmanager_getIsDAGroot()) {
            idmanager_setIsDAGroot(FALSE);
         } else {
            idmanager_setIsDAGroot(TRUE);
         }
         break;
   }
   return;
}

bool debugPrint_id() {
   debugIDManagerEntry_t output;
   output.isDAGroot = idmanager_vars.isDAGroot;
   output.isBridge  = idmanager_vars.isBridge;
   output.my16bID   = idmanager_vars.my16bID;
   output.my64bID   = idmanager_vars.my64bID;
   output.myPANID   = idmanager_vars.myPANID;
   output.myPrefix  = idmanager_vars.myPrefix;
   openserial_printStatus(STATUS_ID,(uint8_t*)&output,sizeof(debugIDManagerEntry_t));
   return TRUE;
}

//=========================== private =========================================