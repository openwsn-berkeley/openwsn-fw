#ifndef __OPENSERIAL_XBEE_H
#define __OPENSERIAL_XBEE_H

// includes "extra functions" which are implemented by openserial_xbee.c

enum {
  XBEE_STATUS_RESET = 0x00,
  XBEE_STATUS_ASSOCIATED = 0x02,
  XBEE_STATUS_DISASSOCIATED = 0x03,
  XBEE_STATUS_COORDINATOR_STARTED = 0x06
};

void xbee_print_modem_status(uint8_t status_byte);

enum {
  XBEE_AT_OK = 0x00,
  XBEE_AT_ERROR = 0x01,
  XBEE_AT_INVALID_COMMAND = 0x02,
  XBEE_AT_INVALID_PARAMETER = 0x03  
};

void xbee_print_AT_response(uint8_t frame_id, uint8_t atcmd[2], uint8_t status, uint8_t * data, uint16_t datalen);

enum{
  XBEE_TX_SUCCESS = 0x00,
  XBEE_TX_FAIL = 0x01
};
void xbee_print_TX_status(uint8_t frame_id, uint8_t status_byte);

void xbee_print_RX_packet64( uint8_t * addr, int8_t rssi, uint8_t options, uint8_t * data, uint16_t datalen);
void xbee_print_RX_packet16( uint8_t * addr, int8_t rssi, uint8_t options, uint8_t * data, uint16_t datalen);






#endif