#ifndef __6TOP_H
#define __6TOP_H

/**
\addtogroup MAChigh
\{
\addtogroup 6TOP
\{
*/

#include "opentimers.h"
#include "openwsn.h"

//=========================== define ==========================================

enum sixtop_CommandID_num{
  SIXTOP_SOFT_CELL_REQ            = 0x00,
  SIXTOP_SOFT_CELL_RESPONSE       = 0x01,
  SIXTOP_REMOVE_SOFT_CELL_REQUEST = 0x02,
  SIXTOP_HARD_CELL_REQ            = 0x03,
  SIXTOP_REMOVE_HARD_CELL         = 0x04,
};

// the different states of the reservation state machine
typedef enum {
    S_IDLE                              = 0x00,   // ready for next event
    // send,receive and sendDone state of resLinkRequest
    S_SIXTOP_LINKREQUEST_SEND           = 0x01,   // generating resLinkRequest command packet
    S_WAIT_SIXTOP_LINKREQUEST_SENDDONE  = 0x02,   // waiting for SendDone confirmation
    S_SIXTOP_LINKREQUEST_RECEIVE        = 0x03,   // 
    // wait respone command
    S_WAIT_FORRESPONSE                  = 0x04,   // waiting for response from the neighbor
    // send,receive and sendDone state of resLinkRespone
    S_SIXTOP_LINKRESPONSE_SEND          = 0x05,   // generating resLinkRespone command packet       
    S_WAIT_SIXTOP_LINKRESPONSE_SENDDONE = 0x06,   // waiting for SendDone confirmation
    S_SIXTOP_LINKRESPONSE_RECEIVE       = 0x07,
    // send,receive and sendDone state of removeLinkRequest
    S_REMOVELINKREQUEST_SEND            = 0x08,   // generating resLinkRespone command packet  
    S_WAIT_REMOVELINKREQUEST_SENDDONE   = 0x09,    // waiting for SendDone confirmation
    S_REMOVELINKREQUEST_RECEIVE         = 0x0a
} sixtop_state_t;

//=========================== typedef =========================================

#define SIXTOP_PERIOD 2000
#define NO_UPPER_LAYER_CALLING_SIXTOP // this is used for debugging reservation

//=========================== module variables ================================

typedef struct {
   uint16_t             periodMaintenance;
   bool                 busySendingKa;        // TRUE when busy sending a keep-alive
   bool                 busySendingAdv;       // TRUE when busy sending an advertisement
   uint8_t              dsn;                  // current data sequence number
   uint8_t              MacMgtTaskCounter;    // counter to determine what management task to do
   opentimer_id_t       timerId;
   opentimer_id_t       TOtimerId;            // TimeOut timer id
   uint16_t             kaPeriod;             // period of sending KA
   sixtop_state_t       State;
   uint8_t              commandID;
   uint8_t              button_event; //when requestOrRemoveLink%3 is 0 or 1, call uResLinkRequest; when the value is 2, call uResRemoveLink.
} sixtop_vars_t;

//=========================== prototypes ======================================

void    sixtop_init(void);
bool    debugPrint_myDAGrank(void);
// from upper layer
owerror_t sixtop_send(OpenQueueEntry_t *msg);
void    sixtop_linkRequest(open_addr_t* sixtopNeighAddr,uint16_t bandwidth);
void    sixtop_removeLinkRequest(open_addr_t*  sixtopNeighAddr);

// from lower layer
void    task_sixtopNotifSendDone(void);
void    sixtop_sendDone(OpenQueueEntry_t* msg, owerror_t error);
void    task_sixtopNotifReceive(void);

void    sixtop_setKaPeriod(uint16_t kaPeriod);

// events
void    isr_sixtop_button();

/**
\}
\}
*/

#endif
