#ifndef __RADIO_DF_H
#define __RADIO_DF_H

#include "board.h"

/**
    Note: when using inline configuration, the S1 field is used for the 
    direction finding configuration. Hence in PCNF0 register we need set
    the RADIO_PCNF0_S1LEN field to 8 (bits). And the packet length indicated
    in LFLEN doesn't include the length of S1. That means if bytes stream
    received is [0x20, 0x02, 0x03, 0xdd, 0xff, crc1, crc2, crc3], with the 
    following configuration:
    NRF_RADIO_NS->PCNF0 = 
        (((1UL) << RADIO_PCNF0_S0LEN_Pos) & RADIO_PCNF0_S0LEN_Msk) | 
        (((8UL) << RADIO_PCNF0_S1LEN_Pos) & RADIO_PCNF0_S1LEN_Msk) |
        (((8UL) << RADIO_PCNF0_LFLEN_Pos) & RADIO_PCNF0_LFLEN_Msk);

    0x20 is the S0 field
    0x02 is the lflen field
    0x03 is the S1 field
    [0xdd, 0xff] is the packet payload, which is 2 bytes indicated by lflen.
*/

//=========================== define ==========================================

// set this value according to the direction finding configurations
// e.g. if 
#define SAMPLE_MAXCNT       (0xA0)

//=========================== typedef =========================================


//=========================== variables =======================================

//=========================== prototypes ======================================

// admin
void     radio_configure_direction_finding_antenna_switch(void);
void     radio_configure_direction_finding_manual(void);
void     radio_configure_direction_finding_inline(void);
uint16_t radio_get_df_samples(uint32_t* sample_buffer, uint16_t length);
void     radio_get_crc(uint8_t* crc24);

/**
\}
\}
*/

#endif
