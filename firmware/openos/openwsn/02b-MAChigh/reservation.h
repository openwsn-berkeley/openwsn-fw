#ifndef __RESERVATION_H
#define __RESERVATION_H

#include "openwsn.h"

//=========================== typedef =========================================
enum uResCommandID_num{
  RESERCATIONLINKREQ            = 0x00,
  RESERCATIONLINKRESPONSE       = 0x01,
  RESERVATIONREMOVELINKREQUEST  = 0x02,
  RESERVATIONSCHEDULERREQUEST   = 0x03,
  RESERVATIONSCHEDULERESPONSE   = 0x04,
};

typedef struct {
  uint8_t numOfLinks;
  uint8_t slotframeID;
} bandwidth_vars_t;

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
//=========================== variables =======================================

//=========================== prototypes ======================================
//admin
void             reservation_init();
//public
uint8_t          reservation_getuResCommandID();
bandwidth_vars_t reservation_getuResBandwidth();

void             reservation_setuResCommandID(uint8_t commandID);
void             reservation_setuResBandwidth(uint8_t numOfLinks, uint8_t slotframeID);

void             reservation_notifyReceiveuResLinkRequest(OpenQueueEntry_t* msg);
void             reservation_notifyReceiveuResLinkResponse(OpenQueueEntry_t* msg);
void             reservation_notifyReceiveRemoveLinkRequest(OpenQueueEntry_t* msg);
void             reservation_notifyReceiveScheduleRequest(OpenQueueEntry_t* msg);
void             reservation_notifyReceiveScheduleResponse(OpenQueueEntry_t* msg);

void             reservation_linkResponse(open_addr_t* tempNeighbor);
// call by res
void             reservation_sendDone(OpenQueueEntry_t* msg, error_t error);
void             reservation_notifyReceiveuResCommand(OpenQueueEntry_t* msg);
// call by up layer
void             reservation_linkRequest(open_addr_t*  reservationNeighAddr,uint16_t bandwidth);
void             reservation_removeLinkRequest(open_addr_t*  reservationNeighAddr);
//"reservation" pretends it is upplayer and sending a data
void             reservation_pretendSendData();
void             reservation_pretendReceiveData(OpenQueueEntry_t* msg);
// events
void             isr_reservation_button();
#endif