#ifndef __PROCESSIE_H
#define __PROCESSIE_H

#include "opendefs.h"

//=========================== define ==========================================

// maximum of cells in a Schedule IE
#define SCHEDULEIEMAXNUMCELLS 3

// subIE shift
#define MLME_IE_SUBID_SHIFT            8

// subIEs identifier
#define MLME_IE_SUBID_SYNC             0x1A
#define MLME_IE_SUBID_SLOTFRAME_LINK   0x1B
#define MLME_IE_SUBID_TIMESLOT         0x1c
#define MLME_IE_SUBID_CHANNELHOPPING   0x09
#define MLME_IE_SUBID_LINKTYPE         0x40
#define MLME_IE_SUBID_OPCODE           0x41
#define MLME_IE_SUBID_BANDWIDTH        0x42
#define MLME_IE_SUBID_TRACKID          0x43
#define MLME_IE_SUBID_SCHEDULE         0x44

// 0xc9 = 201 is the first available subIE ID for experimental use: 
// https://tools.ietf.org/html/draft-kivinen-802-15-ie-06#section-7
#define IANA_6TOP_SUBIE_ID             0xC9
#define SIXTOP_IE_GROUPID              0x05

// ========================== typedef =========================================

BEGIN_PACK

typedef struct {
   uint16_t        tsNum;
   uint16_t        choffset;
   uint8_t         linkoptions;
} cellInfo_ht;


/**
\brief Header of header IEs.
*/
typedef struct{
   uint16_t length_elementid_type; 
} header_IE_ht; 

/**
\brief Header of payload IEs.
*/
typedef struct{
   uint16_t length_groupid_type;
} payload_IE_ht;

//======= header IEs

/**
\brief TSCH ACK/NACK TimeCorrection IE

IEEE802.15.4e-2012, Section 5.2.4.11, p. 88.
*/
typedef struct {
   int16_t        timesync_info;
} timecorrection_IE_ht;

//======= payload IEs

/**
\brief MLME IE common header

IEEE802.15.4e-2012, Section 5.2.4.5, p. 82.
*/
typedef struct{
   uint16_t        length_subID_type;
} mlme_IE_ht;

/**
\brief TSCH Synchronization IE

http://tools.ietf.org/html/draft-wang-6tisch-6top-sublayer-01#section-4.1.1.1
*/
typedef struct {
   uint8_t         asn[5];
   uint8_t         join_priority;
} sync_IE_ht;

/**
\brief TSCH Slotframe and Link IE

http://tools.ietf.org/html/draft-wang-6tisch-6top-sublayer-01#section-4.1.1.2
*/
typedef struct {
   uint8_t         slotframehandle;
   uint16_t        slotframesize;
   uint8_t         numlinks;
} slotframeLink_IE_ht;

/**
\brief 6top Opcode IE

http://tools.ietf.org/html/draft-wang-6tisch-6top-sublayer-01#section-4.1.1.5
*/
typedef struct {
   uint8_t         opcode;
} opcode_IE_ht;

/**
\brief 6top Bandwidth IE

http://tools.ietf.org/html/draft-wang-6tisch-6top-sublayer-01#section-4.1.1.6
*/
typedef struct{
   uint8_t         slotframeID;
   uint8_t         numOfLinks;
} bandwidth_IE_ht;

/**
\brief 6top Generic Schedule IE

http://tools.ietf.org/html/draft-wang-6tisch-6top-sublayer-01#section-4.1.1.8
*/
typedef struct{
   uint8_t         type;
   uint8_t         length;
   uint8_t         frameID;
   uint8_t         numberOfcells;
   bool            flag;
   cellInfo_ht     cellList[SCHEDULEIEMAXNUMCELLS];
} schedule_IE_ht;

END_PACK

//=========================== variables =======================================

//=========================== prototypes ======================================

void             processIE_prependMLMEIE(
   OpenQueueEntry_t*    pkt,
   uint8_t              len
);
void             processIE_prepend_sixtopIE(
   OpenQueueEntry_t*    pkt, 
   uint8_t              len
);
//===== prepend IEs

uint8_t          processIE_prependSyncIE(
    OpenQueueEntry_t*    pkt
);
uint8_t          processIE_prependSlotframeLinkIE(
    OpenQueueEntry_t*    pkt
);
uint8_t          processIE_prependTSCHTimeslotIE(
    OpenQueueEntry_t*    pkt
);
uint8_t          processIE_prependChannelHoppingIE(
    OpenQueueEntry_t*    pkt
);
uint8_t          processIE_prepend_sixSubIEHeader(
    OpenQueueEntry_t*    pkt,
    uint8_t len
);
uint8_t           processIE_prepend_sixGeneralMessage(
    OpenQueueEntry_t*    pkt,
    uint8_t code
);
uint8_t           processIE_prepend_sixSubID(
    OpenQueueEntry_t*    pkt
);
uint8_t           processIE_prepend_sixCelllist(
    OpenQueueEntry_t*    pkt,
    cellInfo_ht*         cellList
);

//===== retrieve IEs

void             processIE_retrieveSlotframeLinkIE(
   OpenQueueEntry_t*    pkt,
   uint8_t * ptr
); 
void            processIE_retrieve_sixCelllist(
    OpenQueueEntry_t*   pkt,
    uint8_t             ptr,
    uint8_t             length,
    cellInfo_ht*        cellList
);

#endif
