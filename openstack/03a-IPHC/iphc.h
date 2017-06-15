#ifndef __IPHC_H
#define __IPHC_H

/**
\addtogroup LoWPAN
\{
\addtogroup IPHC
\{
*/

#include "opendefs.h"

//=========================== define ==========================================

#define IPHC_DEFAULT_HOP_LIMIT    65
#define IPv6HOP_HDR_LEN           2  // tengfei: should be 2
#define MAXNUM_RH3                3

enum IPHC_enums {
   IPHC_DISPATCH             = 5,
   IPHC_TF                   = 3,
   IPHC_NH                   = 2,
   IPHC_HLIM                 = 0,
   IPHC_CID                  = 7,
   IPHC_SAC                  = 6,
   IPHC_SAM                  = 4,
   IPHC_M                    = 3,
   IPHC_DAC                  = 2,
   IPHC_DAM                  = 0,
};

enum IPHC_DISPATCH_enums {
   IPHC_DISPATCH_IPHC        = 3,
};

enum IPHC_TF_enums {
   IPHC_TF_4B                = 0,
   IPHC_TF_3B                = 1,
   IPHC_TF_1B                = 2,
   IPHC_TF_ELIDED            = 3,
};

enum IPHC_NH_enums {    
   IPHC_NH_INLINE            = 0,
   IPHC_NH_COMPRESSED        = 1,
};

enum NHC_enums {
   // IPv6 Extension Header Encoding starts with b1110 xxxx
   NHC_IPv6EXT_MASK          = 0xf0,          // b1111 0000
   NHC_IPv6EXT_ID            = 0xe0,          // b1110 0000
   // UDP Header Encoding            starts with b1111 0xxx
   NHC_UDP_MASK              = 0xf8,          // b1111 1000
   NHC_UDP_ID                = 0xf0,          // b1111 0000
   // EID Encoding
   NHC_EID_MASK              = 0x0e,
   NHC_EID_HOP_VAL           = 0x00,
   NHC_EID_ROUTING_VAL       = 0x01,
   NHC_EID_IPv6_VAL          = 0x07,
   // next header is iphc
   NHC_IPHC_ID               = 0xe7,
};

enum NHC_NH_enums {
   NHC_NH_MASK               = 0x01,
   NHC_NH_INLINE             = 0x00,
   NHC_NH_COMPRESSED         = 0x01,
};

enum NHC_UDP_enums {
   NHC_UDP_C_MASK            = 0x40,
   NHC_UDP_PORTS_MASK        = 0x03,
};

enum NHC_UDP_PORTS_enums {
   NHC_UDP_PORTS_INLINE      = 0,
   NHC_UDP_PORTS_16S_8D      = 1,
   NHC_UDP_PORTS_8S_16D      = 2,
   NHC_UDP_PORTS_4S_4D       = 3,
};

enum IPHC_HLIM_enums {
   IPHC_HLIM_INLINE          = 0,
   IPHC_HLIM_1               = 1,
   IPHC_HLIM_64              = 2,
   IPHC_HLIM_255             = 3,
};

enum IPHC_CID_enums {
   IPHC_CID_NO               = 0,
   IPHC_CID_YES              = 1,
};

enum IPHC_SAC_enums {
   IPHC_SAC_STATELESS        = 0,
   IPHC_SAC_STATEFUL         = 1,
};

enum IPHC_SAM_enums {
   IPHC_SAM_128B             = 0,
   IPHC_SAM_64B              = 1,
   IPHC_SAM_16B              = 2,
   IPHC_SAM_ELIDED           = 3,
};

enum IPHC_M_enums {
   IPHC_M_NO                 = 0,
   IPHC_M_YES                = 1,
};

enum IPHC_DAC_enums {
   IPHC_DAC_STATELESS        = 0,
   IPHC_DAC_STATEFUL         = 1,
};

enum IPHC_DAM_enums {
   IPHC_DAM_128B             = 0,
   IPHC_DAM_64B              = 1,
   IPHC_DAM_16B              = 2,
   IPHC_DAM_ELIDED           = 3,
};

enum IPHC_OUTER_INNER_enums {
    IPHC_OUTER               = 0,
    IPHC_INNER               = 1,
};

enum FORMAT_6LORH_enums {
    FORMAT_6LORH_MASK        = 0xE0,
    CRITICAL_6LORH           = 0x80,
    ELECTIVE_6LoRH           = 0xa0,        
};

enum IPINIP_LEN_6LORH_enums {
    IPINIP_LEN_6LORH_MASK    = 0x1F, 
    IPINIP_TYPE_6LORH        = 0x06,
};

enum TYPE_6LORH_enums{
    RH3_6LOTH_TYPE_0         = 0x00, 
    RH3_6LOTH_TYPE_1         = 0x01,
    RH3_6LOTH_TYPE_2         = 0x02,
    RH3_6LOTH_TYPE_3         = 0x03,
    RH3_6LOTH_TYPE_4         = 0x04,
    RPI_6LOTH_TYPE           = 0x05,
    IPECAP_6LOTH_TYPE        = 0x06,
};

enum SIZE_6LORH_RH3_enums{
    RH3_6LOTH_SIZE_MASK      = 0x1F,
};

enum PAGE_DISPATCH_enums{
    PAGE_DISPATCH_NO_1       = 0xF1,
    PAGE_DISPATCH_TAG        = 0xF0,
    PAGE_DISPATCH_NUM        = 0x0F,
};

//=========================== typedef =========================================

typedef struct {
   uint8_t     traffic_class;
   uint32_t    flow_label;
   bool        next_header_compressed;
   uint8_t     next_header;
   uint8_t*    routing_header[MAXNUM_RH3];
   uint8_t*    hopByhop_option;
   uint8_t     hop_limit;
   uint8_t	   rhe_length;
   open_addr_t src;
   open_addr_t dest;
   uint8_t     header_length;          ///< Counter for internal use
} ipv6_header_iht; // iht for "internal header type"

/**
\brief IPv6 hop-by-hop option.

The Hop-by-Hop Options header is used to carry optional information that must
be examined by every node along a packet's delivery path.
The Hop-by-Hop Options header is identified by a Next Header value of 0 in the
IPv6 header.

Per http://tools.ietf.org/html/rfc6282#section-4.2, the first 7 bits serve as
an identifier for the IPv6 Extension Header immediately following the
LOWPAN_NHC octet. The remaining bit indicates whether or not the following
header utilizes LOWPAN_NHC encoding.

The Length field contained in a compressed IPv6 Extension Header indicates the
number of octets that pertain to the (compressed) extension header following
the Length field. Note that this changes the Length field definition in
[RFC2460] from indicating the header size in 8-octet units, not including the
first 8 octets.  Changing the Length field to be in units of octets removes
wasteful internal fragmentation.
*/
typedef struct {
   uint8_t    headerlen;               ///< Counter for internal use
   bool       next_header_compressed;
   uint8_t    lowpan_nhc; 
   uint8_t    nextHeader;
   uint8_t    HdrExtLen;
} ipv6_hopbyhop_iht;

/**
\brief RPL Option header type.

Described in http://tools.ietf.org/html/rfc6553#section-3
*/
BEGIN_PACK
typedef struct {
   uint8_t    optionType;
   uint8_t    flags;         ///< 000ORFIK
   uint8_t    rplInstanceID; ///< instanceid
   uint16_t   senderRank;    ///< RPL rank of the sender of the packet
} rpl_option_ht;
END_PACK

//=========================== variables =======================================

//=========================== prototypes ======================================

void          iphc_init(void);
owerror_t     iphc_sendFromForwarding(
   OpenQueueEntry_t*    msg, 
   ipv6_header_iht*     ipv6_outer_header, 
   ipv6_header_iht*     ipv6_inner_header, 
   rpl_option_ht*       rpl_option, 
   uint32_t*            flow_label,
   uint8_t*             rh3_copy,
   uint8_t              rh3_length,
   uint8_t              fw_SendOrfw_Rcv
);
owerror_t     iphc_sendFromBridge(OpenQueueEntry_t *msg);
void          iphc_sendDone(OpenQueueEntry_t *msg, owerror_t error);
void          iphc_receive(OpenQueueEntry_t *msg);
// called by forwarding when IPHC inner header required
owerror_t iphc_prependIPv6Header(
   OpenQueueEntry_t*    msg,
   uint8_t              tf,
   uint32_t             value_flowLabel,
   uint8_t              nh,
   uint8_t              value_nextHeader,
   uint8_t              hlim,
   uint8_t              value_hopLimit,
   bool                 cid,
   bool                 sac,
   uint8_t              sam,
   bool                 m,
   bool                 dac,
   uint8_t              dam,
   open_addr_t*         value_dest,
   open_addr_t*         value_src,
   uint8_t              fw_SendOrfw_Rcv
);
uint8_t iphc_retrieveIPv6HopByHopHeader(
   OpenQueueEntry_t*    msg,
   rpl_option_ht*       rpl_option
);

/**
\}
\}
*/

#endif
