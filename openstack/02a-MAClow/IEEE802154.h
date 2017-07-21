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
   IEEE154_FCF_DSN_SUPPRESSION         = 0,
};

enum IEEE802154_fcf_frameversion_enums {
   IEEE154_FRAMEVERSION_2003           = 0, //ieee154-2003
   IEEE154_FRAMEVERSION_2006           = 1, //ieee154-2006
   IEEE154_FRAMEVERSION_2012           = 2, //ieee154e-2012
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

enum IEEE802154_fcf_dsn_enums {
   IEEE154_DSN_SUPPRESSION_NO          = 0,
   IEEE154_DSN_SUPPRESSION_YES         = 1,
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

enum IEEE802154_ash_scf_enums { // Security Control Field
   IEEE154_ASH_SCF_SECURITY_LEVEL         = 0,
   IEEE154_ASH_SCF_KEY_IDENTIFIER_MODE    = 3,
   IEEE154_ASH_SCF_FRAME_CNT_MODE         = 5,
   IEEE154_ASH_SCF_FRAME_CNT_SIZE         = 6,
};

enum IEEE802154_ash_slf_enums { // Security Level Field
   IEEE154_ASH_SLF_TYPE_NOSEC             = 0,
   IEEE154_ASH_SLF_TYPE_MIC_32            = 1,
   IEEE154_ASH_SLF_TYPE_MIC_64            = 2,
   IEEE154_ASH_SLF_TYPE_MIC_128           = 3,
   IEEE154_ASH_SLF_TYPE_ENC               = 4,
   IEEE154_ASH_SLF_TYPE_ENC_MIC_32        = 5,
   IEEE154_ASH_SLF_TYPE_ENC_MIC_64        = 6,
   IEEE154_ASH_SLF_TYPE_ENC_MIC_128       = 7,
};

enum IEEE802154_ash_keyIdMode_enums { // Key Identifier Mode Field
   IEEE154_ASH_KEYIDMODE_IMPLICIT         = 0,
   IEEE154_ASH_KEYIDMODE_DEFAULTKEYSOURCE = 1,
   IEEE154_ASH_KEYIDMODE_EXPLICIT_16      = 2,
   IEEE154_ASH_KEYIDMODE_EXPLICIT_64      = 3,
};

enum IEEE802154_ash_FrameCounterSuppression_enums { // Frame Counter Suppression Field
	IEEE154_ASH_FRAMECOUNTER_PRESENT    = 0,
	IEEE154_ASH_FRAMECOUNTER_SUPPRESSED = 1,
};

enum IEEE802154_ash_FrameCounterSize_enums { // Frame Counter Size Field
	IEEE154_ASH_FRAMECOUNTER_COUNTER    = 0,
	IEEE154_ASH_FRAMECOUNTER_ASN        = 1,
};


#define TERMINATIONIE_LEN                   2 ///< length of a termination IE

// length(b0~b6):0   ID(b7~b14):0x7E   type(b15): 0
#define HEADER_TERMINATION_1_IE             0x3F00 ///< payload IE is present

// length(b0~b6):0   ID(b7~b14):0x7F   type(b15): 0
#define HEADER_TERMINATION_2_IE             0x3F80 ///< payload IE is NOT present

// length(b0~b10):0   ID(b11~b14):0x0F   type(b15): 1
#define PAYLOAD_TERMINATION_IE              0xF800 ///< MAC payload follows

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
   bool        dsn_suppressed;
   open_addr_t panid;
   open_addr_t dest;
   open_addr_t src;
   int16_t     timeCorrection;
} ieee802154_header_iht; //iht for "internal header type"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== prototypes ======================================

void ieee802154_prependHeader(OpenQueueEntry_t* msg,
                              uint8_t           frameType,
                              bool              payloadIEPresent,
                              uint8_t           sequenceNumber,
                              open_addr_t*      nextHop);

void ieee802154_retrieveHeader (OpenQueueEntry_t*      msg,
                                ieee802154_header_iht* ieee802514_header);

/**
\}
\}
*/

#endif
