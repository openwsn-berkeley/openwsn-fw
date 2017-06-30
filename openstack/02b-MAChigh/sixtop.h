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

//=========================== define ==========================================
// 201 is the first available subIE ID for experimental use: 
// https://tools.ietf.org/html/draft-kivinen-802-15-ie-06#section-7
#define IANA_6TOP_SUBIE_ID          201
// 6P version 
#define IANA_6TOP_6P_VERSION        0x00
#define IANA_6TOP_VESION_MASK       0x0F
// 6p type
#define IANA_6TOP_TYPE_SHIFT        4
#define IANA_6TOP_TYPE_REQUEST      0<<IANA_6TOP_TYPE_SHIFT
#define IANA_6TOP_TYPE_RESPONSE     1<<IANA_6TOP_TYPE_SHIFT
#define IANA_6TOP_TYPE_CONFIRMATION 2<<IANA_6TOP_TYPE_SHIFT
// 6p schedule generation shift
#define IANA_6TOP_GEN_SHIFT         4
// 6P command Id
#define IANA_6TOP_CMD_NONE          0x00
#define IANA_6TOP_CMD_ADD           0x01 // CMD_ADD          | add one or more cells     
#define IANA_6TOP_CMD_DELETE        0x02 // CMD_DELETE       | delete one or more cells  
#define IANA_6TOP_CMD_RELOCATE      0x03 // CMD_RELOCATE     | relocate one or more cells  
#define IANA_6TOP_CMD_COUNT         0x04 // CMD_COUNT        | count scheduled cells     
#define IANA_6TOP_CMD_LIST          0x05 // CMD_LIST         | list the scheduled cells  
#define IANA_6TOP_CMD_CLEAR         0x06 // CMD_CLEAR        | clear all cells
// 6P return code
#define IANA_6TOP_RC_SUCCESS        0x00 // RC_SUCCESS       | operation succeeded
#define IANA_6TOP_RC_ERROR          0x01 // RC_ERROR         | generic error
#define IANA_6TOP_RC_EOL            0x02 // RC_EOL           | end of list
#define IANA_6TOP_RC_RESET          0x03 // RC_RESET         | critical error, reset
#define IANA_6TOP_RC_VER_ERR        0x04 // RC_VER_ERR       | unsupported 6P version   
#define IANA_6TOP_RC_SFID_ERR       0x05 // RC_SFID_ERR      | unsupported SFID         
#define IANA_6TOP_RC_GEN_ERR        0x06 // RC_GEN_ERR       | wrong schedule generation       
#define IANA_6TOP_RC_BUSY           0x07 // RC_BUSY          | busy
#define IANA_6TOP_RC_NORES          0x08 // RC_NORES         | not enough resources      
#define IANA_6TOP_RC_CELLLIST_ERR   0x09 // RC_CELLLIST_ERR  | cellList error      

// SF ID
#define SFID_SF0  0

// Default Ch. ID
#define DEFAULT_CHANNEL_OFFSET 2

typedef enum {
    SIXTOP_CELL_REQUEST       = 0x00,
    SIXTOP_CELL_RESPONSE      = 0x01,
    SIXTOP_CELL_CONFIRMATION  = 0x02,
}sixtop_message_t;

// states of the sixtop-to-sixtop state machine
typedef enum {
    // ready for next event
    SIX_STATE_IDLE                              = 0x00,
    // sending
    SIX_STATE_SENDING_REQUEST                   = 0x01,
    // waiting for SendDone confirmation
    SIX_STATE_WAIT_ADDREQUEST_SENDDONE          = 0x02,   
    SIX_STATE_WAIT_DELETEREQUEST_SENDDONE       = 0x03,
    SIX_STATE_WAIT_RELOCATEREQUEST_SENDDONE     = 0x04,
    SIX_STATE_WAIT_COUNTREQUEST_SENDDONE        = 0x05,
    SIX_STATE_WAIT_LISTREQUEST_SENDDONE         = 0x06,
    SIX_STATE_WAIT_CLEARREQUEST_SENDDONE        = 0x07,
    // waiting for response from the neighbor
    SIX_STATE_WAIT_ADDRESPONSE                  = 0x08, 
    SIX_STATE_WAIT_DELETERESPONSE               = 0x09,
    SIX_STATE_WAIT_RELOCATERESPONSE             = 0x0a,
    SIX_STATE_WAIT_COUNTRESPONSE                = 0x0b,
    SIX_STATE_WAIT_LISTRESPONSE                 = 0x0c,
    SIX_STATE_WAIT_CLEARRESPONSE                = 0x0d,
} six2six_state_t;

// before sixtop protocol is called, sixtop handler must be set
typedef enum {
    SIX_HANDLER_NONE                            = 0x00, // when complete reservation, handler must be set to none
    SIX_HANDLER_MAINTAIN                        = 0x01, // the handler is maintenance process
    SIX_HANDLER_SF0                             = 0x02  // the handler is otf
} six2six_handler_t;

typedef enum {
    METADATA_TYPE_FRAMEID                       = 0
}metadata_t;
//=========================== typedef =========================================

#define SIX2SIX_TIMEOUT_MS 4000
typedef uint8_t                 (*sixtop_sf_getsfid)(void);
typedef uint16_t                (*sixtop_sf_getmetadata)(void);
typedef metadata_t              (*sixtop_sf_translatemetadata)(void);
typedef void (*sixtop_sf_handle_callback)(uint8_t arg);

//=========================== module variables ================================

typedef struct {
   uint16_t                     periodMaintenance;
   bool                         busySendingKA;           // TRUE when busy sending a keep-alive
   bool                         busySendingEB;           // TRUE when busy sending an enhanced beacon
   uint8_t                      dsn;                     // current data sequence number
   uint8_t                      mgtTaskCounter;          // counter to determine what management task to do
   uint8_t              ebCounter;               // counter to determine when to send EB
   opentimers_id_t      ebSendingTimerId;        // EB sending timer id
   opentimers_id_t      maintenanceTimerId;
   opentimers_id_t      timeoutTimerId;          // TimeOut timer id
   uint16_t                     kaPeriod;                // period of sending KA
   uint16_t                     ebPeriod;                // period of sending EB
   six2six_state_t              six2six_state;
   uint8_t                      commandID;
   six2six_handler_t            handler;
   bool                         isResponseEnabled;
   uint8_t                      cellOptions;
   cellInfo_ht                  celllist_toDelete[CELLLIST_MAX_LEN];
   sixtop_sf_getsfid            cb_sf_getsfid;
   sixtop_sf_getmetadata        cb_sf_getMetadata;
   sixtop_sf_translatemetadata  cb_sf_translateMetadata;
   sixtop_sf_handle_callback    cb_sf_handleRCError;
} sixtop_vars_t;

//=========================== prototypes ======================================

// admin
void      sixtop_init(void);
void      sixtop_setKaPeriod(uint16_t kaPeriod);
void      sixtop_setEBPeriod(uint8_t ebPeriod);
bool      sixtop_setHandler(six2six_handler_t handler);
void      sixtop_setSFcallback(
    sixtop_sf_getsfid     cb0,
    sixtop_sf_getmetadata cb1, 
    sixtop_sf_translatemetadata cb2, 
    sixtop_sf_handle_callback cb3
);
// scheduling
void sixtop_request(
    uint8_t      code, 
    open_addr_t* neighbor, 
    uint8_t      numCells, 
    uint8_t      cellOptions, 
    cellInfo_ht* celllist_toBeAdded, 
    cellInfo_ht* celllist_toBeRemoved, 
    uint8_t      sfid,
    uint16_t     listingOffset,
    uint16_t     listingMaxNumCells
);
// from upper layer
owerror_t sixtop_send(OpenQueueEntry_t *msg);
// from lower layer
void      task_sixtopNotifSendDone(void);
void      task_sixtopNotifReceive(void);
// debugging
bool      debugPrint_myDAGrank(void);
bool      debugPrint_kaPeriod(void);
// control
void      sixtop_setIsResponseEnabled(bool isEnabled);

/**
\}
\}
*/

#endif


