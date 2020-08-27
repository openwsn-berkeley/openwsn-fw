#ifndef __RADIO_BF_H
#define __RADIO_BF_H

#include "board.h"

//=========================== define ==========================================

//=========================== typedef =========================================


//=========================== variables =======================================

//=========================== prototypes ======================================

// admin
void     radio_configure_direction_finding(void);
void     radio_get_df_samples(uint32_t* sample_buffer, uint16_t length);

/**
\}
\}
*/

#endif
