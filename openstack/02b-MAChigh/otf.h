#ifndef __OTF_H
#define __OTF_H

/**
\addtogroup MAChigh
\{
\addtogroup otf
\{
*/

#include "opendefs.h"

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== module variables ================================

//=========================== prototypes ======================================

// admin
void      otf_init(void);
// notification from sixtop
void      otf_notif_addedCell(void);
void      otf_notif_removedCell(void);

//statistics update
owerror_t otf_stat_tx(
                      OpenQueueEntry_t*      msg,
                      uint32_t*              flow_label,
                      uint8_t                fw_SendOrfw_Rcv
                      );



/**
\}
\}
*/

#endif
