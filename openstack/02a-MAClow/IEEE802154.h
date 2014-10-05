#ifndef __IEEE802154_H
#define __IEEE802154_H

/**
\addtogroup MAClow
\{
\addtogroup IEEE802154
\{
*/

#include "opendefs.h"

//=========================== define ==========================================

enum IEEE802154_fcf_enums {
   IEEE154_FCF_FRAME_TYPE              = 0,
   IEEE154_FCF_SECURITY_ENABLED        = 3,
   IEEE154_FCF_FRAME_PENDING           = 4,
   IEEE154_FCF_ACK_REQ                 = 5,
   IEEE154_FCF_INTRAPAN                = 6,
   IEEE154_FCF_IELIST_PRESENT          = 1,
   IEEE154_FCF_DEST_ADDR_MODE          = 2,
   IEEE154_FCF_FRAME_VERSION           = 4,
   IEEE154_FCF_SRC_ADDR_MODE           = 6,
};


enum IEEE802154_fcf_frameversion_enums {
   IEEE154_FRAMEVERSION_2003           = 0, //ieee154-2003
   IEEE154_FRAMEVERSION_2006           = 1, //ieee154-2006
   IEEE154_FRAMEVERSION                = 2, //ieee154
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

enum IEEE802154_fcf_ielist_enums {
   IEEE154_IELIST_NO                   = 0,
   IEEE154_IELIST_YES                  = 1,
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

//=========================== typedef =========================================

typedef struct {
   bool        valid;
   uint8_t     headerLength;    //including the length field
   uint8_t     frameType;
   bool        securityEnabled;
   bool        framePending;
   bool        ackRequested;
   bool        panIDCompression;
   bool        ieListPresent;
   uint8_t     frameVersion;
   uint8_t     dsn;
   open_addr_t panid;
   open_addr_t dest;
   open_addr_t src;
} ieee802154_header_iht; //iht for "internal header type"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== prototypes ======================================

void ieee802154_prependHeader(OpenQueueEntry_t* msg,
                              uint8_t           frameType,
                              uint8_t           ielistpresent,
                              uint8_t           frameversion,
                              bool              securityEnabled,
                              uint8_t           sequenceNumber,
                              open_addr_t*      nextHop);

void ieee802154_retrieveHeader (OpenQueueEntry_t*      msg,
                                ieee802154_header_iht* ieee802514_header);

/**
\}
\}
*/

#endif