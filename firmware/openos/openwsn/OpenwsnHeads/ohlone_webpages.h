/**
\brief Webpages for Ohlone

\author Ankur Mehta <mehtank@eecs.berkeley.edu>, September 2010
*/

#ifndef __OHLONEWEBPAGES_H
#define __OHLONEWEBPAGES_H

//=========================== define ==========================================

#define HTTP_LINE(str) {                         \
  if (chunk == ++current_line) {                 \
    memcpy(packet, str, sizeof(str)-1);          \
    len = sizeof(str)-1;                         \
  }                                              \
}

#define HTTP_LINE_REPLACE16(offset, value) {     \
  if (chunk == current_line)                     \
    ohlone_line_replace16(packet+offset, value); \
}

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void     ohlone_webpages_init();
uint8_t  ohlone_webpage(uint8_t *getRequest, uint16_t chunk, uint8_t *packet);

#endif