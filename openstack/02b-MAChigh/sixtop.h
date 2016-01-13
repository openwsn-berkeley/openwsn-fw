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
   SIX_HANDLER_OTF                     = 0x02  // the handler is otf
} six2six_handler_t;

//=========================== typedef =========================================

#define SIXTOP_NBCELLS_INREQ     3     //nb cells in the 6top IE (request / reply)

#define SIX2SIX_TIMEOUT_MS 4000
#define SIXTOP_MINIMAL_EBPERIOD 5 // minist period of sending EB

//TODO: fix a correct timeout
//#define SIX2SIX_TIMEOUT_MS ((uint32_t)(1 + MAXBE / NUMSHAREDTXRX) * TXRETRIES * SUPERFRAME_LENGTH * TsSlotDuration * PORT_TICS_PER_MS / 1000)


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
bool      sixtop_isIdle(void);
void      sixtop_addCells(open_addr_t* neighbor, uint16_t numCells, track_t track);
void      sixtop_removeCell(open_addr_t*  neighbor);
void      sixtop_removeCellByInfo(open_addr_t*  neighbor,cellInfo_ht* cellInfo);
track_t   sixtop_getTrackCellsByState(uint8_t slotframeID, uint8_t numOfLink, cellInfo_ht* cellList, open_addr_t* previousHop);
// maintaining
void      sixtop_maintaining(uint16_t slotOffset,open_addr_t* neighbor);
// from upper layer
owerror_t sixtop_send(OpenQueueEntry_t *msg);
// from lower layer
void      task_sixtopNotifSendDone(void);
void      task_sixtopNotifReceive(void);
// debugging
bool      debugPrint_myDAGrank(void);
bool      debugPrint_kaPeriod(void);
//track helpers for the best effort track
bool      sixtop_is_trackequal(track_t track1, track_t track2);
bool      sixtop_is_trackbesteffort(track_t track);
track_t   sixtop_get_trackbesteffort(void);



/**
\}
\}
*/

#endif
