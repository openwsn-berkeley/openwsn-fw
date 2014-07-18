/**
DO NOT EDIT DIRECTLY!!

This file was 'objectified' by SCons as a pre-processing
step for the building a Python extension module.

This was done on 2014-07-18 12:11:03.919176.
*/
/**
\brief Webpgages for Ohlone

\author Ankur Mehta <mehtank@eecs.berkeley.edu>, September 2010
*/

#include "openwsn_obj.h"
#include "ohlone_webpages_obj.h"

/*
#include "gyro.h"
#include "large_range_accel.h"
#include "magnetometer.h"
#include "sensitive_accel_temperature.h"
*/

//=========================== variables =======================================

//=========================== prototypes ======================================

uint16_t ohlone_replace_digit(uint8_t *buffer, uint16_t value, uint16_t place);
void     ohlone_line_replace16(uint8_t *buffer, uint16_t value);
uint8_t  ohlone_insert3sensors(uint8_t *buffer, uint8_t *sensors); 
uint8_t  ohlone_insert4sensors(uint8_t *buffer, uint8_t *sensors); 

//=========================== public ==========================================

void ohlone_webpages_init(void) {
   /*
   if (*(&eui64+3)==0x09) {                      // this is a GINA board (not a basestation)
      gyro_init();
      large_range_accel_init();
      magnetometer_init();
      sensitive_accel_temperature_init();
   }
   */
}

uint8_t ohlone_webpage(uint8_t *getRequest, uint16_t chunk, uint8_t *packet) {
  // TODO : enforce max of TCP_DEFAULT_WINDOW_SIZE
  uint8_t len = 0;
  uint8_t current_line = 0;
  current_line--;
  
  HTTP_LINE("HTTP/1.1 200 OK\r\n\r\n");
  
  switch (*(getRequest + 1)) {
    
     default:
        memcpy(packet + len, ":)", 2);
       len += 2;
     }
  
  return len;
}

//=========================== private =========================================

uint16_t ohlone_replace_digit(uint8_t *buffer, uint16_t value, uint16_t place) {
  uint8_t digit = '0';
  while (value > place) {
    value -= place;
    digit++;
  }
  *buffer = digit;
  return value;
}

void ohlone_line_replace16(uint8_t *buffer, uint16_t value) {
  value = ohlone_replace_digit(buffer++, value, 10000);
  value = ohlone_replace_digit(buffer++, value, 1000);
  value = ohlone_replace_digit(buffer++, value, 100);
  value = ohlone_replace_digit(buffer++, value, 10);
  value = ohlone_replace_digit(buffer++, value, 1);
}

uint8_t ohlone_insert3sensors(uint8_t *buffer, uint8_t *sensordata) {
  uint8_t len = 0;
  ohlone_line_replace16(buffer + len, (sensordata[0] << 8) + sensordata[1]);
  len += 5; buffer[len++] = ',';    
  ohlone_line_replace16(buffer + len, (sensordata[2] << 8) + sensordata[3]);
  len += 5; buffer[len++] = ',';    
  ohlone_line_replace16(buffer + len, (sensordata[4] << 8) + sensordata[5]);
  len += 5;
  return len;
}

uint8_t ohlone_insert4sensors(uint8_t *buffer, uint8_t *sensordata) {
  uint8_t len = ohlone_insert3sensors(buffer, sensordata);
  buffer[len++] = ',';
  ohlone_line_replace16(buffer + len, (sensordata[6] << 8) + sensordata[7]);
  len += 5; 
  return len;
}
