#ifndef __PROCESSIE_H
#define __PROCESSIE_H

#define         MAXLINKNEIGHBORS    10

#include "openwsn.h"

//=========================== define =========================================
// maximum of cells can be scheduled or removed
#define MAXSCHEDULEDCELLS   3

// subIE's ID and shift
#define SIXTOP_MLME_SYNC_IE_SUBID                        0x1A
#define SIXTOP_MLME_SYNC_IE_SUBID_SHIFT                  1
#define SIXTOP_MLME_SLOTFRAME_LINK_IE_SUBID              0x1B
#define SIXTOP_MLME_SLOTFRAME_LINK_IE_SUBID_SHIFT        1
#define SIXTOP_MLME_TIMESLOT_IE_SUBID                    0x1c
#define SIXTOP_MLME_TIMESLOT_IE_SUBID_SHIFT              1
#define SIXTOP_MLME_CHANNELHOPPING_IE_SUBID              0x09
#define SIXTOP_MLME_CHANNELHOPPING_IE_SUBID_SHIFT        1
#define SIXTOP_MLME_LINKTYPE_IE_SUBID                    0x40
#define SIXTOP_MLME_LINKTYPE_IE_SUBID_SHIFT              1
#define SIXTOP_MLME_RES_OPCODE_IE_SUBID                  0x41
#define SIXTOP_MLME_RES_OPCODE_IE_SUBID_SHIFT            1
#define SIXTOP_MLME_RES_BANDWIDTH_IE_SUBID               0x42
#define SIXTOP_MLME_RES_BANDWIDTH_IE_SUBID_SHIFT         1
#define SIXTOP_MLME_RES_TRACKID_IE_SUBID                 0x43
#define SIXTOP_MLME_RES_TRACKID_IE_SUBID_SHIFT           1
#define SIXTOP_MLME_RES_GENERAL_SCHEDULE_IE_SUBID        0x44
#define SIXTOP_MLME_RES_GENERAL_SCHEDULE_IE_SUBID_SHIFT  1


// should those IE related define place here? (There are existed in IEEE802154e)
//======================= ieee802154e =====================
/*
// group id of IE for MLME
#define IEEE802154E_PAYLOAD_DESC_GROUP_ID_MLME   (0x01 << 1) //includes shift 1

// type of IE
#define IEEE802154E_DESC_TYPE_SHORT              0x00
#define IEEE802154E_DESC_TYPE_LONG               0x01

// identity of header IE and payloadIE
#define IEEE802154E_DESC_TYPE_HEADER_IE          0x00
#define IEEE802154E_DESC_TYPE_PAYLOAD_IE         0x01

//length field on PAYLOAD/HEADER DESC
#define IEEE802154E_DESC_LEN_HEADER_IE_MASK      0xFE00
#define IEEE802154E_DESC_LEN_PAYLOAD_IE_MASK     0xFFE0

// length shift on PAYLOAD/HEADER DESC
#define IEEE802154E_DESC_LEN_PAYLOAD_IE_SHIFT    5
#define IEEE802154E_DESC_LEN_HEADER_IE_SHIFT     9

//groupID/elementID field on PAYLOAD/HEADER DESC
#define IEEE802154E_DESC_ELEMENTID_HEADER_IE_MASK     0x01FE
#define IEEE802154E_DESC_GROUPID_PAYLOAD_IE_MASK      0x001E

//groupID/elementID shift on PAYLOAD/HEADER DESC
#define IEEE802154E_DESC_ELEMENTID_HEADER_IE_SHIFT    1
#define IEEE802154E_DESC_GROUPID_PAYLOAD_IE_SHIFT     1

// IE ids
#define IEEE802154E_MLME_IE_GROUPID                   0x01
#define IEEE802154E_ACK_NACK_TIMECORRECTION_ELEMENTID 0x1E

// length/subID field on MLME SubIE LONG 
#define IEEE802154E_DESC_LEN_LONG_MLME_IE_MASK        0xFFE0
#define IEEE802154E_DESC_SUBID_LONG_MLME_IE_MASK      0x001E

// length/subID feild on MLME SubIE SHORT page 82
#define IEEE802154E_DESC_LEN_SHORT_MLME_IE_MASK       0xFF00
#define IEEE802154E_DESC_SUBID_SHORT_MLME_IE_MASK     0x00FE

// length shift for short type subIE and long type subIE
#define IEEE802154E_DESC_LEN_SHORT_MLME_IE_SHIFT      8
#define IEEE802154E_DESC_LEN_LONG_MLME_IE_SHIFT       5
// subID shift for short type subIE and long type subIE
#define IEEE802154E_DESC_SUBID_LONG_MLME_IE_SHIFT     1
#define IEEE802154E_DESC_SUBID_SHORT_MLME_IE_SHIFT    1
*/
// ============================== typedef ================================
typedef struct {
    uint8_t asn[5];
    uint8_t join_priority;
}synch_IE_t;

//the Slotframe and Link IE
typedef struct {
    uint8_t slotframehandle;
    uint16_t slotframesize;
    uint8_t numlinks;
}sixtop_slotframelink_subIE_t;

typedef struct {
    uint8_t opcode;
}sixtop_opcode_subIE_t;

typedef	struct{
    uint8_t slotframeID;
    uint8_t numOfLinks;
}sixtop_bandwidth_subIE_t;

typedef struct {
    uint16_t tsNum;
    uint16_t choffset;
    uint8_t linkoptions;
}sixtop_cellInfo_subIE_t;

typedef	struct{
    //TLV structure
    uint8_t type;
    uint8_t length;
    uint8_t frameID;
    uint8_t numberOfcells;
    bool flag;
    sixtop_cellInfo_subIE_t celllist[MAXSCHEDULEDCELLS];
}sixtop_generalschedule_subIE_t;

//=========================== variables =======================================

//=========================== prototypes ======================================

void             process_prependMLMEIEHeader(
   OpenQueueEntry_t* pkt,
   uint8_t len
);
//prepend subIE
uint8_t          processIE_prependSyncIE(
   OpenQueueEntry_t* pkt
);
uint8_t          processIE_prependFrameLinkIE(
   OpenQueueEntry_t* pkt
);
uint8_t          processIE_prependTimeslotIE(
   OpenQueueEntry_t* pkt
);
uint8_t          processIE_prependChannelHoppingIE(
   OpenQueueEntry_t* pkt
);
uint8_t          processIE_prependSixtopLinkTypeIE(
   OpenQueueEntry_t* pkt
);
uint8_t          processIE_prependSixtopOpcodeIE(
   OpenQueueEntry_t* pkt,
   uint8_t uResCommandID
);
uint8_t          processIE_prependSixtopBandwidthIE(
   OpenQueueEntry_t* pkt,
   uint8_t numOfLinks, 
   uint8_t slotframeID
);
uint8_t          processIE_prependSixtopGeneralSheduleIE(
   OpenQueueEntry_t* pkt,
   uint8_t type,
   uint8_t frameID,
   uint8_t flag,
   sixtop_cellInfo_subIE_t* celllist
);
//retrieve subIE
void             processIE_retrieveSyncIEcontent(
   OpenQueueEntry_t* pkt,
   uint8_t * ptr
); 
void             processIE_retrieveSlotframeLinkIE(
   OpenQueueEntry_t* pkt,
   uint8_t * ptr
); 
void             processIE_retrieveChannelHoppingIE(
   OpenQueueEntry_t* pkt,
   uint8_t * ptr
); 
void             processIE_retrieveTimeslotIE(
   OpenQueueEntry_t* pkt,
   uint8_t * ptr
); 
void             processIE_retrieveSixtopLinkTypeIE(
   OpenQueueEntry_t* pkt,
   uint8_t * ptr
); 
void             processIE_retrieveSixtopOpcodeIE(
   OpenQueueEntry_t* pkt,
   uint8_t * ptr,
   sixtop_opcode_subIE_t* opcodeIE
); 
void             processIE_retrieveSixtopBandwidthIE(
   OpenQueueEntry_t* pkt,
   uint8_t * ptr,
   sixtop_bandwidth_subIE_t* bandwidthIE
); 
void             processIE_retrieveSixtopGeneralSheduleIE(
   OpenQueueEntry_t* pkt,
   uint8_t * ptr,
   sixtop_generalschedule_subIE_t* schedule_ie
);

#endif
