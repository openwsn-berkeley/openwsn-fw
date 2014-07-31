/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:11:17.289365.
*/
#include "openwsn_obj.h"
#include "idmanager_obj.h"
#include "eui64_obj.h"
#include "packetfunctions_obj.h"
#include "openserial_obj.h"
#include "neighbors_obj.h"

//=========================== variables =======================================

// declaration of global variable _idmanager_vars_ removed during objectification.

//=========================== prototypes ======================================

//=========================== public ==========================================

void idmanager_init(OpenMote* self) {
   (self->idmanager_vars).isDAGroot            = FALSE;
   (self->idmanager_vars).isBridge             = FALSE;
   (self->idmanager_vars).myPANID.type         = ADDR_PANID;
   (self->idmanager_vars).myPANID.panid[0]     = 0xca;
   (self->idmanager_vars).myPANID.panid[1]     = 0xfe;

   (self->idmanager_vars).myPrefix.type        = ADDR_PREFIX;
   memset(&(self->idmanager_vars).myPrefix.prefix[0], 0x00, sizeof((self->idmanager_vars).myPrefix.prefix));
   (self->idmanager_vars).my64bID.type         = ADDR_64B;

 eui64_get(self, (self->idmanager_vars).my64bID.addr_64b);
 packetfunctions_mac64bToMac16b(self, &(self->idmanager_vars).my64bID,&(self->idmanager_vars).my16bID);
}

bool idmanager_getIsDAGroot(OpenMote* self) {
   bool res;
   INTERRUPT_DECLARATION();
   
   DISABLE_INTERRUPTS();
   res=(self->idmanager_vars).isDAGroot;
   ENABLE_INTERRUPTS();
   return res;
}

void idmanager_setIsDAGroot(OpenMote* self, bool newRole) {
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   (self->idmanager_vars).isDAGroot = newRole;
 neighbors_updateMyDAGrankAndNeighborPreference(self);
   ENABLE_INTERRUPTS();
}

bool idmanager_getIsBridge(OpenMote* self) {
   bool res;
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   res=(self->idmanager_vars).isBridge;
   ENABLE_INTERRUPTS();
   return res;
}

void idmanager_setIsBridge(OpenMote* self, bool newRole) {
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   (self->idmanager_vars).isBridge = newRole;
   ENABLE_INTERRUPTS();

}

open_addr_t* idmanager_getMyID(OpenMote* self, uint8_t type) {
   open_addr_t* res;
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   switch (type) {
     case ADDR_16B:
        res= &(self->idmanager_vars).my16bID;
        break;
     case ADDR_64B:
        res= &(self->idmanager_vars).my64bID;
        break;
     case ADDR_PANID:
        res= &(self->idmanager_vars).myPANID;
        break;
     case ADDR_PREFIX:
        res= &(self->idmanager_vars).myPrefix;
        break;
     case ADDR_128B:
        // you don't ask for my full address, rather for prefix, then 64b
     default:
// openserial_printCritical(self, COMPONENT_IDMANAGER,ERR_WRONG_ADDR_TYPE,
//              (errorparameter_t)type,
//              (errorparameter_t)0);
        res= NULL;
        break;
   }
   ENABLE_INTERRUPTS();
   return res;
}

owerror_t idmanager_setMyID(OpenMote* self, open_addr_t* newID) {
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();
   switch (newID->type) {
     case ADDR_16B:
        memcpy(&(self->idmanager_vars).my16bID,newID,sizeof(open_addr_t));
        break;
     case ADDR_64B:
        memcpy(&(self->idmanager_vars).my64bID,newID,sizeof(open_addr_t));
        break;
     case ADDR_PANID:
        memcpy(&(self->idmanager_vars).myPANID,newID,sizeof(open_addr_t));
        break;
     case ADDR_PREFIX:
        memcpy(&(self->idmanager_vars).myPrefix,newID,sizeof(open_addr_t));
        break;
     case ADDR_128B:
        //don't set 128b, but rather prefix and 64b
     default:
// openserial_printCritical(self, COMPONENT_IDMANAGER,ERR_WRONG_ADDR_TYPE,
//              (errorparameter_t)newID->type,
//              (errorparameter_t)1);
        ENABLE_INTERRUPTS();
        return E_FAIL;
   }
   ENABLE_INTERRUPTS();
   return E_SUCCESS;
}

bool idmanager_isMyAddress(OpenMote* self, open_addr_t* addr) {
   open_addr_t temp_my128bID;
   bool res;
   INTERRUPT_DECLARATION();
   DISABLE_INTERRUPTS();

   switch (addr->type) {
     case ADDR_16B:
        res= packetfunctions_sameAddress(self, addr,&(self->idmanager_vars).my16bID);
        ENABLE_INTERRUPTS();
        return res;
     case ADDR_64B:
        res= packetfunctions_sameAddress(self, addr,&(self->idmanager_vars).my64bID);
        ENABLE_INTERRUPTS();
        return res;
     case ADDR_128B:
        //build temporary my128bID
        temp_my128bID.type = ADDR_128B;
        memcpy(&temp_my128bID.addr_128b[0],&(self->idmanager_vars).myPrefix.prefix,8);
        memcpy(&temp_my128bID.addr_128b[8],&(self->idmanager_vars).my64bID.addr_64b,8);

        res= packetfunctions_sameAddress(self, addr,&temp_my128bID);
        ENABLE_INTERRUPTS();
        return res;
     case ADDR_PANID:
        res= packetfunctions_sameAddress(self, addr,&(self->idmanager_vars).myPANID);
        ENABLE_INTERRUPTS();
        return res;
     case ADDR_PREFIX:
        res= packetfunctions_sameAddress(self, addr,&(self->idmanager_vars).myPrefix);
        ENABLE_INTERRUPTS();
        return res;
     default:
// openserial_printCritical(self, COMPONENT_IDMANAGER,ERR_WRONG_ADDR_TYPE,
//              (errorparameter_t)addr->type,
//              (errorparameter_t)2);
        ENABLE_INTERRUPTS();
        return FALSE;
   }
}

void idmanager_triggerAboutRoot(OpenMote* self) {
   uint8_t number_bytes_from_input_buffer;
   uint8_t input_buffer;
   // get command from OpenSerial
   number_bytes_from_input_buffer = openserial_getInputBuffer(self, &input_buffer,sizeof(input_buffer));
   if (number_bytes_from_input_buffer!=sizeof(input_buffer)) {
// openserial_printError(self, COMPONENT_IDMANAGER,ERR_INPUTBUFFER_LENGTH,
//            (errorparameter_t)number_bytes_from_input_buffer,
//            (errorparameter_t)0);
      return;
   };
   // handle command
   switch (input_buffer) {
     case ACTION_YES:
 idmanager_setIsDAGroot(self, TRUE);
        break;
     case ACTION_NO:
 idmanager_setIsDAGroot(self, FALSE);
        break;
     case ACTION_TOGGLE:
        if ( idmanager_getIsDAGroot(self)) {
 idmanager_setIsDAGroot(self, FALSE);
        } else {
 idmanager_setIsDAGroot(self, TRUE);
        }
        break;
   }
   return;
}

void idmanager_triggerAboutBridge(OpenMote* self) {
   uint8_t number_bytes_from_input_buffer;
   uint8_t input_buffer;
   //get command from OpenSerial
   number_bytes_from_input_buffer = openserial_getInputBuffer(self, &input_buffer,sizeof(input_buffer));
   if (number_bytes_from_input_buffer!=sizeof(input_buffer)) {
// openserial_printError(self, COMPONENT_IDMANAGER,ERR_INPUTBUFFER_LENGTH,
//            (errorparameter_t)number_bytes_from_input_buffer,
//            (errorparameter_t)1);
      return;
   };
   //handle command
   switch (input_buffer) {
     case ACTION_YES:
 idmanager_setIsBridge(self, TRUE);
        break;
     case ACTION_NO:
 idmanager_setIsBridge(self, FALSE);
        break;
     case ACTION_TOGGLE:
        if ( idmanager_getIsBridge(self)) {
 idmanager_setIsBridge(self, FALSE);
        } else {
 idmanager_setIsBridge(self, TRUE);
        }
        break;
   }
   return;
}

/**
\brief Trigger this module to print status information, over serial.

debugPrint_* functions are used by the openserial module to continuously print
status information about several modules in the OpenWSN stack.

\returns TRUE if this function printed something, FALSE otherwise.
*/
bool debugPrint_id(OpenMote* self) {
   debugIDManagerEntry_t output;
   output.isDAGroot = (self->idmanager_vars).isDAGroot;
   output.isBridge  = (self->idmanager_vars).isBridge;
   output.my16bID   = (self->idmanager_vars).my16bID;
   output.my64bID   = (self->idmanager_vars).my64bID;
   output.myPANID   = (self->idmanager_vars).myPANID;
   output.myPrefix  = (self->idmanager_vars).myPrefix;
   // openserial_printStatus(self, STATUS_ID,(uint8_t*)&output,sizeof(debugIDManagerEntry_t));
   return TRUE;
}


//=========================== private =========================================
