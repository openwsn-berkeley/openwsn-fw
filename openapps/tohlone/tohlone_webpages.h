/**
\brief Webpages for Ohlone

\author Ankur Mehta <mehtank@eecs.berkeley.edu>, September 2010
*/

#ifndef __TOHLONE_WEBPAGES_H
#define __TOHLONE_WEBPAGES_H

//=========================== define ==========================================

#define HTTP_LINE(str) {                         \
  if (chunk == ++current_line) {                 \
    memcpy(packet, str, sizeof(str)-1);          \
    len = sizeof(str)-1;                         \
  }                                              \
}

#define HTTP_LINE_REPLACE16(offset, value) {     \
  if (chunk == current_line)                     \
    tohlone_line_replace16(packet+offset, value); \
}

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void     tohlone_webpages_init(void);
uint8_t  tohlone_webpage(uint8_t *getRequest, uint16_t chunk, uint8_t *packet);

#endif
