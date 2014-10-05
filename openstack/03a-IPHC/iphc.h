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
#define IPv6HOP_HDR_LEN           3

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
};

enum NHC_IPv6HOP_enums {
   NHC_IPv6HOP_MASK          = 0x0e,
   NHC_IPv6HOP_VAL           = 0x0e,
   NHC_HOP_NH_MASK           = 0x01,
};


enum NHC_UDP_enums {
   NHC_UDP_C_MASK            = 0x40,
   NHC_UDP_PORTS_MASK        = 0x03,
};

enum NHC_UDP_PORTS_enums {
   NHC_UDP_PORTS_INLINE      = 0,
   NHC_UDP_PORTS_16S_8D      = 1,
   NHC_UDP_PORTS_8S_8D       = 2,
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

//=========================== typedef =========================================

typedef struct {
   uint8_t     traffic_class;
   uint32_t    flow_label;
   bool        next_header_compressed;
   uint8_t     next_header;
   uint8_t     hop_limit;
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
   uint8_t    optionType;    ///< RPL_HOPBYHOP_HEADER_OPTION_TYPE
   uint8_t    optionLen;     ///< 8-bit field indicating the length of the option, in octets, excluding the Option Type and Opt Data Len fields.
   uint8_t    flags;         ///< ORF00000
   uint8_t    rplInstanceID; ///< instanceid
   uint16_t   senderRank;    ///< RPL rank of the sender of the packet
} rpl_option_ht;
END_PACK

//=========================== variables =======================================

//=========================== prototypes ======================================

void          iphc_init(void);
owerror_t     iphc_sendFromForwarding(
   OpenQueueEntry_t*    msg, 
   ipv6_header_iht*     ipv6_header, 
   rpl_option_ht*       rpl_option, 
   uint32_t*            flow_label,
   uint8_t              fw_SendOrfw_Rcv
);
owerror_t     iphc_sendFromBridge(OpenQueueEntry_t *msg);
void          iphc_sendDone(OpenQueueEntry_t *msg, owerror_t error);
void          iphc_receive(OpenQueueEntry_t *msg);

/**
\}
\}
*/

#endif
