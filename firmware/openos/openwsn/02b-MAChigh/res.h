#ifndef __RES_H
#define __RES_H

/**
\addtogroup MAChigh
\{
\addtogroup RES
\{
*/

#include "opentimers.h"
#include "openwsn.h"

//=========================== define ==========================================

enum uResCommandID_num{
  RESERCATIONLINKREQ            = 0x00,
  RESERCATIONLINKRESPONSE       = 0x01,
  RESERVATIONREMOVELINKREQUEST  = 0x02,
  RESERVATIONSCHEDULERREQUEST   = 0x03,
  RESERVATIONSCHEDULERESPONSE   = 0x04,
};

// the different states of the reservation state machine
typedef enum {
    S_IDLE                              = 0x00,   // ready for next event
    // send,receive and sendDone state of resLinkRequest
    S_RESLINKREQUEST_SEND               = 0x01,   // generating resLinkRequest command packet
    S_WAIT_RESLINKREQUEST_SENDDONE      = 0x02,   // waiting for SendDone confirmation
    S_RESLINKREQUEST_RECEIVE            = 0x03,   // 
    // wait respone command
    S_WAIT_FORRESPONSE                  = 0x04,   // waiting for response from the neighbor
    // send,receive and sendDone state of resLinkRespone
    S_RESLINKRESPONSE_SEND               = 0x05,   // generating resLinkRespone command packet       
    S_WAIT_RESLINKRESPONSE_SENDDONE      = 0x06,   // waiting for SendDone confirmation
    S_RESLINKRESPONSE_RECEIVE            = 0x07,
    // send,receive and sendDone state of removeLinkRequest
    S_REMOVELINKREQUEST_SEND            = 0x08,   // generating resLinkRespone command packet  
    S_WAIT_REMOVELINKREQUEST_SENDDONE   = 0x09,    // waiting for SendDone confirmation
    S_REMOVELINKREQUEST_RECEIVE         = 0x0a
} reservation_state_t;

//=========================== typedef =========================================

#define RESPERIOD 10000
#define NO_UPPER_LAYER_CALLING_RESERVATION // this is used for debugging reservation

//=========================== module variables ================================

typedef struct {
   uint16_t        periodMaintenance;
   bool            busySendingKa;        // TRUE when busy sending a keep-alive
   bool            busySendingAdv;       // TRUE when busy sending an advertisement
   uint8_t         dsn;                  // current data sequence number
   uint8_t         MacMgtTaskCounter;    // counter to determine what management task to do
   opentimer_id_t  timerId;
   uint16_t        kaPeriod;             // period of sending KA
} res_vars_t;

typedef struct {
  bool                 busySending;     // TRUE when busy sending an reservation command
  opentimer_id_t       timerId;           // this timer is used to simulate uplayer's requirement for new links
  uint16_t             periodReservation;
  bool                 addORremove;     // when FALSE, add one link, when TRUE, remove one link.
  open_addr_t          ResNeighborAddr;
  reservation_state_t  State;
  uint8_t              commandID;
  uint8_t              button_event; //when requestOrRemoveLink%3 is 0 or 1, call uResLinkRequest; when the value is 2, call uResRemoveLink.
} reservation_vars_t;

//=========================== prototypes ======================================

void    res_init(void);
bool    debugPrint_myDAGrank(void);
// from upper layer
owerror_t res_send(OpenQueueEntry_t *msg);
// from lower layer
void    task_resNotifSendDone(void);
void    task_resNotifReceive(void);
void    res_setKaPeriod(uint16_t kaPeriod);

void    res_notifRetrieveIEDone(OpenQueueEntry_t *msg);

//admin
void             reservation_init();
//public
//by neighbors to add a link
void             reservation_addLinkToNode(open_addr_t* addressToWrite);

void             reservation_notifyReceiveuResLinkRequest(OpenQueueEntry_t* msg);
void             reservation_notifyReceiveuResLinkResponse(OpenQueueEntry_t* msg);
void             reservation_notifyReceiveRemoveLinkRequest(OpenQueueEntry_t* msg);
void             reservation_notifyReceiveScheduleRequest(OpenQueueEntry_t* msg);
void             reservation_notifyReceiveScheduleResponse(OpenQueueEntry_t* msg);

void             reservation_linkResponse(open_addr_t* tempNeighbor);
// call by res
void             reservation_sendDone(OpenQueueEntry_t* msg, owerror_t error);
void             reservation_notifyReceiveuResCommand(OpenQueueEntry_t* msg);
// call by up layer
void             reservation_linkRequest(open_addr_t*  reservationNeighAddr,uint16_t bandwidth);
void             reservation_removeLinkRequest(open_addr_t*  reservationNeighAddr);
//"reservation" pretends it is upplayer and sending a data
void             reservation_pretendSendData();
void             reservation_pretendReceiveData(OpenQueueEntry_t* msg);
// events
void             isr_reservation_button();

/**
\}
\}
*/

#endif
