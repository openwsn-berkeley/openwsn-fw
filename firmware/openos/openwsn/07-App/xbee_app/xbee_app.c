#include "openwsn.h"
#include "udpinject.h"
#include "openudp.h"
#include "openqueue.h"
#include "openserial.h"
#include "openserial_xbee.h"
#include "packetfunctions.h"
#include "idmanager.h"
#include "IEEE802154E.h"

//=========================== variables =======================================

uint8_t last_frame_id;

//=========================== prototypes ======================================

//=========================== public ==========================================

void xbeeapp_init() {
  last_frame_id = 0;
}

void xbeeapp_send(uint8_t frame_id, uint8_t * dest_address, uint8_t * data, uint16_t data_len) {
  OpenQueueEntry_t* pkt;
  open_addr_t * tempaddr;
  uint8_t number_bytes_from_input_buffer;
  uint8_t input_buffer[18];
  //get command from OpenSerial (16B IPv6 destination address, 2B destination port)
  number_bytes_from_input_buffer = openserial_getInputBuffer(&(input_buffer[0]),sizeof(input_buffer));
  if (number_bytes_from_input_buffer!=sizeof(input_buffer)) {
    openserial_printError(COMPONENT_XBEEAPP,ERR_INPUTBUFFER_LENGTH,
                          (errorparameter_t)number_bytes_from_input_buffer,
                          (errorparameter_t)0);
    return;
  };
  //prepare packet
  pkt = openqueue_getFreePacketBuffer(COMPONENT_XBEEAPP);
  if (pkt==NULL) {
    openserial_printError(COMPONENT_XBEEAPP,ERR_NO_FREE_PACKET_BUFFER,
                          (errorparameter_t)0,
                          (errorparameter_t)0);
    return;
  }
  pkt->creator                     = COMPONENT_XBEEAPP;
  pkt->owner                       = COMPONENT_XBEEAPP;
  pkt->l4_protocol                 = IANA_UDP;
  pkt->l4_sourcePortORicmpv6Type   = WKP_UDP_XBEE;
  pkt->l4_destination_port         = WKP_UDP_XBEE;
  pkt->l3_destinationAdd.type      = ADDR_128B;
  tempaddr = idmanager_getMyID(ADDR_PREFIX);
  memcpy(&(pkt->l3_destinationAdd.prefix),&(tempaddr->prefix),8);
  memcpy(&(pkt->l3_destinationAdd.addr_64b),dest_address,8);
  packetfunctions_reserveHeaderSize(pkt,data_len);
  memcpy(&(pkt->payload),data,data_len);
  //send packet
  if ((openudp_send(pkt))==E_FAIL) {
    openqueue_freePacketBuffer(pkt);
  } else {
    last_frame_id = frame_id;
  }
}

void xbeeapp_sendDone(OpenQueueEntry_t* msg, owerror_t error) {
  msg->owner = COMPONENT_XBEEAPP;
  if (msg->creator!=COMPONENT_XBEEAPP) {
    openserial_printError(COMPONENT_XBEEAPP,ERR_UNEXPECTED_SENDDONE,
                          (errorparameter_t)0,
                          (errorparameter_t)0);
  }
  openqueue_freePacketBuffer(msg);
  
  // send TX success to xbee
  if (last_frame_id) {
    if ( error == E_SUCCESS)
      xbee_print_TX_status(last_frame_id ,XBEE_TX_SUCCESS);
    else
      xbee_print_TX_status(last_frame_id ,XBEE_TX_FAIL);
  }
}

void xbeeapp_receive(OpenQueueEntry_t* msg) {
  if ( msg->l3_sourceAdd.type == ADDR_128B || msg->l3_sourceAdd.type == ADDR_64B ) {
    xbee_print_RX_packet64((msg->l3_sourceAdd.addr_64b), msg->l1_rssi, 0, msg->payload, msg->length);
  }
  openqueue_freePacketBuffer(msg);
}

bool xbeeapp_debugPrint() {
  return FALSE;
}

/* Commands supported:
R/W:
CE (coordinator enable) : 1 = coordinator, 0 = end device
SH (high 64b address)
SL (low 64b address)

R only:
AI (association indication) 
VR (firmware version)
*/

void at_command_set(uint8_t frame_id, uint8_t atcmd[2], uint8_t * data, uint16_t len) {
  open_addr_t * addr;
  uint8_t c1 = atcmd[0], c2 = atcmd[1];
  if ( c1 == 'C' && c2 == 'E' ) {
    if ( data[0] ) { // enable coordinator
      idmanager_setIsDAGroot(TRUE);
    } else {
      idmanager_setIsDAGroot(FALSE);
    }
  } else if ( c1 == 'S' && c2 == 'H' ) {
    if ( len != 4 ) goto errparam;
    addr = idmanager_getMyID(ADDR_64B);
    memcpy(addr->addr_64b,data,4);
    idmanager_setMyID(addr);
  } else if ( c1 == 'S' && c2 == 'L' ) {
    if ( len != 4 ) goto errparam;
    addr = idmanager_getMyID(ADDR_64B);
    memcpy(&(addr->addr_64b[4]),data,4);
    idmanager_setMyID(addr);    
  } else {
    if (frame_id)
      xbee_print_AT_response(frame_id, atcmd, XBEE_AT_INVALID_COMMAND, data, 0);
    return;
  }
  
  if ( frame_id)
    xbee_print_AT_response(frame_id, atcmd, XBEE_AT_OK, data, 0);
  return;
  
errparam:
  if ( frame_id)
    xbee_print_AT_response(frame_id, atcmd, XBEE_AT_INVALID_PARAMETER, data, 0);
  return;
}

void at_command_get(uint8_t frame_id, uint8_t atcmd[2]){
  uint8_t * dataptr;
  uint8_t datalen;
  uint8_t buf[16];
  dataptr = buf; datalen = 0;
  uint8_t c1 = atcmd[0], c2 = atcmd[1];
  if ( c1 == 'C' && c2 == 'E' ) {
    buf[0] == idmanager_getIsDAGroot();
    datalen = 1;
  } else if ( c1 == 'S' && c2 == 'H') {
    dataptr = idmanager_getMyID(ADDR_64B)->addr_64b;
    datalen = 4;
  } else if ( c1 == 'S' && c2 == 'H') {
    dataptr = &(idmanager_getMyID(ADDR_64B)->addr_64b[4]);
    datalen = 4;
  } else if ( c1 == 'A' && c2 == 'I') {
    if (  ieee154e_isSynch() ) {
      buf[0] = 0;
    } else {
      buf[0] = 1;
    }
    datalen = 1;
  } else if ( c1 == 'V' && c2 == 'R') {
      buf[0] = OPENWSN_VERSION_MAJOR | 0xD;
      buf[1] = OPENWSN_VERSION_MINOR;
      datalen = 2;
  } else {
    if (frame_id)
      xbee_print_AT_response(frame_id, atcmd, XBEE_AT_INVALID_COMMAND, dataptr, 0);
    return;    
  }
  if ( frame_id)
    xbee_print_AT_response(frame_id, atcmd, XBEE_AT_INVALID_COMMAND,dataptr,datalen);
}

//=========================== private =========================================