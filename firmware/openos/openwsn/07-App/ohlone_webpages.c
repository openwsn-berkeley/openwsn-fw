/**
\brief Webpgages for Ohlone

\author Ankur Mehta <mehtank@eecs.berkeley.edu>, September 2010
*/

#include "openwsn.h"
#include "ohlone_webpages.h"

//drivers
#include "gyro.h"
#include "large_range_accel.h"
#include "magnetometer.h"
#include "sensitive_accel_temperature.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

uint16_t ohlone_replace_digit(uint8_t *buffer, uint16_t value, uint16_t place);
void     ohlone_line_replace16(uint8_t *buffer, uint16_t value);
uint8_t  ohlone_insert3sensors(uint8_t *buffer, uint8_t *sensors); 
uint8_t  ohlone_insert4sensors(uint8_t *buffer, uint8_t *sensors); 

//=========================== public ==========================================

void ohlone_webpages_init() {
     if (*(&eui64+3)==0x09) {                      // this is a GINA board (not a basestation)
      gyro_init();
      large_range_accel_init();
      magnetometer_init();
      sensitive_accel_temperature_init();
   }
}

uint8_t ohlone_webpage(uint8_t *getRequest, uint16_t chunk, uint8_t *packet) {
  // TODO : enforce max of TCP_DEFAULT_WINDOW_SIZE
  uint8_t len = 0;
  uint8_t current_line = 0;
  current_line--;
  uint8_t sensordata[10];
  
  HTTP_LINE("HTTP/1.1 200 OK\r\n\r\n");
  
  switch (*(getRequest + 1)) {
  case 'h':
    //         "<-- 10 --><-- 20 --><-- 30 --><-- 40 --><--7-->"
    HTTP_LINE( "<html><head><title>Hi!</title></head>" );
    HTTP_LINE( "<body><h1>:o</h1></body></html>" );
    break;

  case 't':
    //         "<-- 10 --><-- 20 --><-- 30 --><-- 40 --><--7-->"
    HTTP_LINE( "<html><head><title>The temp is:</title></head>" );
    HTTP_LINE( "<body><h1>xxxxx</h1></body></html>" );

    if (chunk == current_line)
      sensitive_accel_temperature_get_measurement(sensordata);
    HTTP_LINE_REPLACE16(10, (sensordata[8] << 8) + sensordata[9]);
    
    break;

  case 'l': 
    large_range_accel_get_measurement(sensordata);
    len += ohlone_insert3sensors(packet + len, sensordata);
    break;

  case 'm': 
    magnetometer_get_measurement(sensordata);
    len += ohlone_insert3sensors(packet + len, sensordata);
    break;

  case 's': 
    if (*(getRequest + 2) == 'm') {
      memcpy(packet + len, ":)", 2);
      len += 2;
    } else {
      sensitive_accel_temperature_get_measurement(sensordata);
      len += ohlone_insert4sensors(packet + len, sensordata);
    }
    break;                         

  case 'g': 
    gyro_get_measurement(sensordata);
    len += ohlone_insert4sensors(packet + len, sensordata);
    break;
    
  case 'f':
    memcpy(packet + len, ":(", 2);
    len += 2;
    break;
    
  default:
    memcpy(packet + len, "=P", 2);
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