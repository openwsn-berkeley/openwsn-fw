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
#include "IEEE802154E.h"


//=========================== define ==========================================
// 6P version 
#define IANA_6TOP_6P_VERSION   0x01 
// 6P command Id
#define IANA_6TOP_CMD_NONE     0x00
#define IANA_6TOP_CMD_ADD      0x01 // CMD_ADD      | add one or more cells     
#define IANA_6TOP_CMD_DELETE   0x02 // CMD_DELETE   | delete one or more cells  
#define IANA_6TOP_CMD_COUNT    0x03 // CMD_COUNT    | count scheduled cells     
#define IANA_6TOP_CMD_LIST     0x04 // CMD_LIST     | list the scheduled cells  
#define IANA_6TOP_CMD_CLEAR    0x05 // CMD_CLEAR    | clear all cells
// 6P return code
#define IANA_6TOP_RC_SUCCESS   0x06 // RC_SUCCESS  | operation succeeded      
#define IANA_6TOP_RC_VER_ERR   0x07 // RC_VER_ERR  | unsupported 6P version   
#define IANA_6TOP_RC_SFID_ERR  0x08 // RC_SFID_ERR | unsupported SFID         
#define IANA_6TOP_RC_BUSY      0x09 // RC_BUSY     | handling previous request
#define IANA_6TOP_RC_RESET     0x0a // RC_RESET    | abort 6P transaction     
#define IANA_6TOP_RC_ERR       0x0b // RC_ERR      | operation failed         

// SF ID
#define SFID_SF0  0

// Default Ch. ID
#define DEFAULT_CHANNEL_OFFSET 2

enum sixtop_CommandID_num{
    SIXTOP_SOFT_CELL_REQ                = 0x00,
    SIXTOP_SOFT_CELL_RESPONSE           = 0x01,
    SIXTOP_REMOVE_SOFT_CELL_REQUEST     = 0x02,
};

// states of the sixtop-to-sixtop state machine
typedef enum {

/*
    SIX_IDLE                            = 0x00,   // ready for next event
   // ADD: source
   SIX_SENDING_ADDREQUEST              = 0x01,   // generating LinkRequest packet
   SIX_WAIT_ADDREQUEST_SENDDONE        = 0x02,   // waiting for SendDone confirmation
   SIX_WAIT_ADDRESPONSE                = 0x03,   // waiting for response from the neighbor
   SIX_ADDRESPONSE_RECEIVED            = 0x04,   // I received the link response request command
   // ADD: destinations
   SIX_ADDREQUEST_RECEIVED             = 0x05,   // I received the link request command
   SIX_SENDING_ADDRESPONSE             = 0x06,   // generating resLinkRespone command packet
//   SIX_WAIT_ADDRESPONSE_SENDDONE       = 0x07,   // waiting for SendDone confirmation
   // REMOVE: source
   SIX_SENDING_REMOVEREQUEST           = 0x08,   // generating resLinkRespone command packet
   SIX_WAIT_REMOVEREQUEST_SENDDONE     = 0x09,   // waiting for SendDone confirmation
   // REMOVE: destinations
   SIX_REMOVEREQUEST_RECEIVED          = 0x0a    // I received the remove link request command
*/

    // ready for next event
    SIX_IDLE                            = 0x00,
    // sending
    SIX_SENDING_REQUEST                 = 0x01,
    // waiting for SendDone confirmation
    SIX_WAIT_ADDREQUEST_SENDDONE        = 0x02,   
    SIX_WAIT_DELETEREQUEST_SENDDONE     = 0x03,
    SIX_WAIT_COUNTREQUEST_SENDDONE      = 0x04,
    SIX_WAIT_LISTREQUEST_SENDDONE       = 0x05,
    SIX_WAIT_CLEARREQUEST_SENDDONE      = 0x06,
    // waiting for response from the neighbor
    SIX_WAIT_ADDRESPONSE                = 0x07, 
    SIX_WAIT_DELETERESPONSE             = 0x08,
    SIX_WAIT_COUNTRESPONSE              = 0x09,
    SIX_WAIT_LISTRESPONSE               = 0x0a,
    SIX_WAIT_CLEARRESPONSE              = 0x0b,
   
    // response senddone
    SIX_REQUEST_RECEIVED                = 0x0c,
    SIX_WAIT_RESPONSE_SENDDONE          = 0x0d
} six2six_state_t;

// before sixtop protocol is called, sixtop handler must be set
typedef enum {
    SIX_HANDLER_NONE                    = 0x00, // when complete reservation, handler must be set to none
    SIX_HANDLER_MAINTAIN                = 0x01, // the handler is maintenance process
    SIX_HANDLER_SF0                     = 0x02, //the handler is otf
    SIX_HANDLER_SFLOC                   = 0x03  //SF-loc
} six2six_handler_t;

//=========================== typedef =========================================

#define SIXTOP_NBCELLS_INREQ     1     //nb cells in the 6top IE (request / reply)

#define SIXTOP_MINIMAL_EBPERIOD 5 // minist period of sending EB

//TODO: fix a correct timeout
#define SIX2SIX_TIMEOUT_MS ((uint32_t)10 * (1 + MAXBE / SCHEDULE_MINIMAL_6TISCH_ACTIVE_CELLS) * TXRETRIES * SLOTFRAME_LENGTH * TsSlotDuration * PORT_TICS_PER_MS / 1000)
#define SIX2SIX_LINKREP_TIMEOUT_MS  (SIX2SIX_TIMEOUT_MS / 2)


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
   bool                 isResponseEnabled;
} sixtop_vars_t;

//=========================== prototypes ======================================

// admin
void      sixtop_init(void);
void      sixtop_setKaPeriod(uint16_t kaPeriod);
void      sixtop_setEBPeriod(uint8_t ebPeriod);
void      sixtop_setHandler(six2six_handler_t handler);
// scheduling
bool      sixtop_isIdle(void);
//void      sixtop_addCells(open_addr_t* neighbor, uint16_t numCells, track_t track);
//void      sixtop_removeCell(open_addr_t*  neighbor);
//void      sixtop_removeCellByInfo(open_addr_t*  neighbor,cellInfo_ht* cellInfo);
track_t   sixtop_getTrackCellsByState(uint8_t slotframeID, uint8_t numOfLink, cellInfo_ht* cellList, open_addr_t* previousHop);
void      sixtop_request(uint8_t code, open_addr_t* neighbor, uint8_t numCells, track_t track);
void      sixtop_addORremoveCellByInfo(uint8_t code,open_addr_t*  neighbor,cellInfo_ht* cellInfo);
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
track_t   sixtop_get_trackcommon(void);

// control
void      sixtop_setIsResponseEnabled(bool isEnabled);


/**
\}
\}
*/

#endif
