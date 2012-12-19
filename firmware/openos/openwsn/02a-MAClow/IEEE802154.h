#ifndef __IEEE802154_H
#define __IEEE802154_H

/**
\addtogroup helpers
\{
\addtogroup IEEE802154
\{
*/

#include "openwsn.h"

//=========================== define ==========================================
#define INTIAL_KEY_ID           0x07

enum IEEE802154_fcf_enums {
   IEEE154_FCF_FRAME_TYPE              = 0,
   IEEE154_FCF_SECURITY_ENABLED        = 3,
   IEEE154_FCF_FRAME_PENDING           = 4,
   IEEE154_FCF_ACK_REQ                 = 5,
   IEEE154_FCF_INTRAPAN                = 6,
   IEEE154_FCF_IELISTPRESENT           = 1,
   IEEE154_FCF_DEST_ADDR_MODE          = 2,
   IEEE154_FCF_SRC_ADDR_MODE           = 6,
};

enum IEEE802154_fcf_type_enums {
   IEEE154_TYPE_BEACON                 = 0,
   IEEE154_TYPE_DATA                   = 1,
   IEEE154_TYPE_ACK                    = 2,
   IEEE154_TYPE_CMD                    = 3,
   IEEE154_TYPE_UNDEFINED              = 5,
};

enum IEEE802154_fcf_sec_enums {
   IEEE154_SEC_NO_SECURITY             = 0,
   IEEE154_SEC_YES_SECURITY            = 1,
};

enum IEEE802154_fcf_pending_enums {
   IEEE154_PENDING_NO_FRAMEPENDING     = 0,
   IEEE154_PENDING_YES_FRAMEPENDING    = 1,
};

enum IEEE802154_fcf_ack_enums {
   IEEE154_ACK_NO_ACK_REQ              = 0,
   IEEE154_ACK_YES_ACK_REQ             = 1,
};

enum IEEE802154_fcf_panid_enums {
   IEEE154_PANID_UNCOMPRESSED          = 0,
   IEEE154_PANID_COMPRESSED            = 1,
};

enum IEEE802154_fcf_addr_mode_enums {
   IEEE154_ADDR_NONE                   = 0,
   IEEE154_ADDR_SHORT                  = 2,
   IEEE154_ADDR_EXT                    = 3,
};

enum IEEE802154_aux_enums {
   IEEE154_AUX_SECURITY_LEVEL                      = 0,
   IEEE154_AUX_KEY_ID_MODE                         = 3,
   IEEE154_AUX_FRAME_COUNTER_SUPPRESSION           = 5,
   IEEE154_AUX_FRAME_COUNTER_SIZE                  = 6,
};

enum IEEE802154_aux_security_level_enums {
   IEEE154_SECURITY_LEVEL_32MIC                    = 1,
};

enum IEEE802154_aux_key_id_mode_enums {
   IEEE154_KEY_FROM_INDEX                          = 1,
};

enum IEEE802154_aux_frame_counter_suppression_enums {
    IEEE154_COUNTER_SUPPRESSION_YES                = 1,
};

enum IEEE802154_aux_frame_counter_size_enums {
    IEEE154_COUNTER_SIZE_5B                        =1,
};


//=========================== typedef =========================================

typedef struct {
   bool        valid;
   uint8_t     headerLength;    //including the length field
   uint8_t     frameType;
   bool        securityEnabled;
   bool        framePending;
   bool        ackRequested;
   bool        panIDCompression;
   bool        IEListPresent;   //did have IE field?
   uint8_t     dsn;
   open_addr_t panid;
   open_addr_t dest;
   open_addr_t src;
} ieee802154_header_iht; //iht for "internal header type"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== prototypes ======================================

void ieee802154_prependHeader  (OpenQueueEntry_t*      msg,
                                uint8_t                frameType,
                                bool                   securityEnabled,
                                bool                   IEListPresent,
                                uint8_t                sequenceNumber,
                                open_addr_t*           nextHop);
void ieee802154_retrieveHeader (OpenQueueEntry_t*      msg,
                                ieee802154_header_iht* ieee802514_header);

/**
\}
\}
*/

#endif