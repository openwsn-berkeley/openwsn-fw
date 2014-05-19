#ifndef __SECURITY_H__
#define __SECURITY_H__

/**
\addtogroup BSP
\{
\addtogroup security
\{

\brief Cross-platform declaration "security" bsp module.

\author Marcelo Barros <marcelobarrosalmeida@gmail.com>, May 2014.
*/


 
//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

void security_init();
uint8_t security_decrypt(OpenQueueEntry_t* msg, ieee802154_sec_hdr_t sec_hdr);

/**
\}
\}
*/

#endif /* __SECURITY_H__ */

