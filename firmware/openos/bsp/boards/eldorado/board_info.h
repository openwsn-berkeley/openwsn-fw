/*
 *
 *  Created on: Feb 27, 2012
 *      Author: Vitor Mangueira, Branko Kerkez <bkerkez@berkeley.edu>
 */

#ifndef BOARD_INFO_H_
#define BOARD_INFO_H_

#include "PE_Types.h"
#include "string.h"
//=========================== defines =========================================

#define PORT_RADIOTIMER_WIDTH               uint16_t

//===== adaptive_sync accuracy

#define SYNC_ACCURACY                       1 // ticks

//=========================== variables =======================================

static const uint8_t rreg_uriquery[] = "h=usp";
static const uint8_t infoBoardname[] = "Eldorado";
static const uint8_t infouCName[]    = "HCS08";
static const uint8_t infoRadioName[] = "MC13192";

//=========================== prototypes ======================================

//=========================== public ==========================================

//=========================== private =========================================
#endif /* BOARD_INFO_H_ */
