#ifndef __IEFIELD_H
#define __IEFIELD_H

#include "processIE.h"
#include "openwsn.h"

//=========================== define ==========================================
// IE header
enum IE_enums {
   IE_LENGTH                    = 5,
   IE_GROUPID                   = 1,
   IE_TYPE                      = 0,
};

//subIE header
enum SUBIE_enums {
   SUBIE_TYPE                   = 0,
   SUBIE_SUBID                  = 1,
   SUBIE_SHORT_LENGTH           = 8,
   SUBIE_LONG_LENGTH            = 5,

};

enum SUBIE_SUBID_enums {
   SUBIE_SYNC                   = 0x1a,
   SUBIE_FRAME_AND_LINK         = 0x1b,
   SUBIE_TIMESLOT               = 0x1c,
   SUBIE_CHANNEL_HOPPING        = 0x09,
   SUBIE_LINKTYPE               = 0x40,
   SUBIE_RES_COMMAND            = 0x41,
   SUBIE_RES_BANDWIDTH          = 0x42,
   SUBIE_RES_GENERAL_SCHEDULE   = 0x43,
};

enum SUBIE_type_enums {
   SUBIE_TYPE_SHORT             = 0,
   SUBIE_TYPE_LONG              = 1,
};

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== prototypes ======================================

void IEFiled_prependIE  (OpenQueueEntry_t*      msg);
void IEFiled_retrieveIE (OpenQueueEntry_t*      msg);

/**
\}
\}
*/

#endif