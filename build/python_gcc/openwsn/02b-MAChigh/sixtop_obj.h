/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:02:04.557171.
*/
#ifndef __SIXTOP_H
#define __SIXTOP_H

/**
\addtogroup MAChigh
\{
\addtogroup sixtop
\{
*/

#include "opentimers_obj.h"
#include "openwsn_obj.h"

//=========================== define ==========================================

enum sixtop_CommandID_num{
   SIXTOP_SOFT_CELL_REQ                = 0x00,
   SIXTOP_SOFT_CELL_RESPONSE           = 0x01,
   SIXTOP_REMOVE_SOFT_CELL_REQUEST     = 0x02,
};

// states of the sixtop-to-sixtop state machine
typedef enum {
   SIX_IDLE                            = 0x00,   // ready for next event
   // ADD: source
   SIX_SENDING_ADDREQUEST              = 0x01,   // generating LinkRequest packet
   SIX_WAIT_ADDREQUEST_SENDDONE        = 0x02,   // waiting for SendDone confirmation
   SIX_WAIT_ADDRESPONSE                = 0x03,   // waiting for response from the neighbor
   SIX_ADDRESPONSE_RECEIVED            = 0x04,   // I received the link response request command
   // ADD: destinations
   SIX_ADDREQUEST_RECEIVED             = 0x05,   // I received the link request command
   SIX_SENDING_ADDRESPONSE             = 0x06,   // generating resLinkRespone command packet
   SIX_WAIT_ADDRESPONSE_SENDDONE       = 0x07,   // waiting for SendDone confirmation
   // REMOVE: source
   SIX_SENDING_REMOVEREQUEST           = 0x08,   // generating resLinkRespone command packet
   SIX_WAIT_REMOVEREQUEST_SENDDONE     = 0x09,   // waiting for SendDone confirmation
   // REMOVE: destinations
   SIX_REMOVEREQUEST_RECEIVED          = 0x0a    // I received the remove link request command
} six2six_state_t;

//=========================== typedef =========================================

#define SIX2SIX_TIMEOUT_MS 2000

//=========================== module variables ================================

typedef struct {
   uint16_t             periodMaintenance;
   bool                 busySendingKA;           // TRUE when busy sending a keep-alive
   bool                 busySendingEB;           // TRUE when busy sending an enhanced beacon
   uint8_t              dsn;                     // current data sequence number
   uint8_t              mgtTaskCounter;          // counter to determine what management task to do
   opentimer_id_t       maintenanceTimerId;
   opentimer_id_t       timeoutTimerId;          // TimeOut timer id
   uint16_t             kaPeriod;                // period of sending KA
   six2six_state_t      six2six_state;
   uint8_t              commandID;
} sixtop_vars_t;

#include "openwsnmodule_obj.h"
typedef struct OpenMote OpenMote;

//=========================== prototypes ======================================

// admin
void sixtop_init(OpenMote* self);
void sixtop_setKaPeriod(OpenMote* self, uint16_t kaPeriod);
// scheduling
void sixtop_addCells(OpenMote* self, open_addr_t* neighbor, uint16_t numCells);
void sixtop_removeCell(OpenMote* self, open_addr_t*  neighbor);
// from upper layer
owerror_t sixtop_send(OpenMote* self, OpenQueueEntry_t *msg);
// from lower layer
void task_sixtopNotifSendDone(OpenMote* self);
void task_sixtopNotifReceive(OpenMote* self);
// debugging
bool debugPrint_myDAGrank(OpenMote* self);

/**
\}
\}
*/

#endif
