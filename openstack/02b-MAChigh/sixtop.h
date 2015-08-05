#ifndef __SIXTOP_H
#define __SIXTOP_H

/**
\addtogroup MAChigh
\{
\addtogroup sixtop
\{
*/

#include "opentimers.h"
#include "opendefs.h"
#include "processIE.h"
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

// before sixtop protocol is called, sixtop handler must be set
typedef enum {
   SIX_HANDLER_NONE                    = 0x00, // when complete reservation, handler must be set to none
   SIX_HANDLER_MAINTAIN                = 0x01, // the handler is maintenance process
   SIX_HANDLER_OTF                     = 0x02, // the handler is otf
   SIX_HANDLER_PID                     = 0x03  // the handler is PID scheduling algorithm 
} six2six_handler_t;

//=========================== typedef =========================================

#define SIX2SIX_TIMEOUT_MS 10000
#define SIXTOP_MINIMAL_EBPERIOD 5 // minist period of sending EB
//#define SIXTOP_DEBUGINFO

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
   uint16_t             ebPeriod;                // period of sending EB
   six2six_state_t      six2six_state;
   uint8_t              commandID;
   six2six_handler_t    handler;
} sixtop_vars_t;

//=========================== prototypes ======================================

// admin
void      sixtop_init(void);
void      sixtop_setKaPeriod(uint16_t kaPeriod);
void      sixtop_setEBPeriod(uint8_t ebPeriod);
void      sixtop_setHandler(six2six_handler_t handler);
// scheduling
void      sixtop_addCells(open_addr_t* neighbor, uint16_t numCells);
void      sixtop_removeCell(open_addr_t*  neighbor);
void      sixtop_removeCellByInfo(open_addr_t*  neighbor,cellInfo_ht* cellInfo);
// maintaining
void      sixtop_maintaining(uint16_t slotOffset,open_addr_t* neighbor);
// from upper layer
owerror_t sixtop_send(OpenQueueEntry_t *msg);
// from lower layer
void      task_sixtopNotifSendDone(void);
void      task_sixtopNotifReceive(void);
// interface with pid
void      sixtop_notifyNewSlotframe(void);
void      sixtop_checkSchedule();
// debugging
bool      debugPrint_myDAGrank(void);
bool      debugPrint_kaPeriod(void);

/**
\}
\}
*/

#endif
