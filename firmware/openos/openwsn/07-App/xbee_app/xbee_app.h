#ifndef __XBEE_APP_H
#define __XBEE_APP_H

/**
\addtogroup App
\{
\addtogroup xbeeApp
\{
*/

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void xbeeapp_init();
void xbeeapp_trigger();
void xbeeapp_sendDone(OpenQueueEntry_t* msg, owerror_t error);
void xbeeapp_receive(OpenQueueEntry_t* msg);
bool xbeeapp_debugPrint();

void at_command_set(uint8_t atcmd[2], uint8_t * data, uint16_t len);
void at_command_get(uint8_t atcmd[2]);
/**
\}
\}
*/

#endif
