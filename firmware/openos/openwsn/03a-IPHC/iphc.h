#ifndef __IPHC_H
#define __IPHC_H

/**
\addtogroup LoWPAN
\{
\addtogroup IPHC
\{
*/

//=========================== define ==========================================

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
   uint8_t     next_header;
   uint8_t     hop_limit;
   open_addr_t src;
   open_addr_t dest;
   uint8_t     header_length; //used to toss the header
} ipv6_header_iht; //iht for "internal header type"

//=========================== variables =======================================

//=========================== prototypes ======================================

void    iphc_init();
error_t iphc_sendFromForwarding(OpenQueueEntry_t *msg);
error_t iphc_sendFromBridge(OpenQueueEntry_t *msg);
void    iphc_sendDone(OpenQueueEntry_t* msg, error_t error);
void    iphc_receive(OpenQueueEntry_t* msg);

/**
\}
\}
*/

#endif
