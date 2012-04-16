/**
 \brief K20-specific board information bsp module.

 This module simply defines some strings describing the board, which CoAP uses
 to return the board's description.

 \author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, February 2012.
 */

#ifndef _BOARD_INFO_H
#define _BOARD_INFO_H

#include "derivative.h"

#define PORT_TIMER_WIDTH                    uint16_t
#define PORT_TICS_PER_MS                    33


#define CAPTURE_TIME()   
#define DISABLE_INTERRUPTS() 

#define ENABLE_INTERRUPTS() 



#define SCHEDULER_WAKEUP()                  //do nothing
#define SCHEDULER_ENABLE_INTERRUPT()        //do nothing.


//===== IEEE802154E timing

// time-slot related
#define PORT_TsSlotDuration                 491   // counter counts one extra count, see datasheet
// execution speed related
#define PORT_maxTxDataPrepare               66    // 2014us (measured 746us)
#define PORT_maxRxAckPrepare                10    //  305us (measured  83us)
#define PORT_maxRxDataPrepare               33    // 1007us (measured  84us)
#define PORT_maxTxAckPrepare                10    //  305us (measured 219us)
// radio speed related
#define PORT_delayTx                        7     //  214us (measured 219us)
#define PORT_delayRx                        0     //    0us (can not measure)
// radio watchdog


//=========================== variables =======================================

static const uint8_t rreg_uriquery[] = "h=ucb";
static const uint8_t infoBoardname[] = "k20";
static const uint8_t infouCName[]    = "Freescale k20DX72";
static const uint8_t infoRadioName[] = "AT86RF231";

//=========================== defines =========================================



// SLP_TR 

#define PORT_PIN_RADIO_SLP_TR_CNTL_HIGH()  
#define PORT_PIN_RADIO_SLP_TR_CNTL_LOW()   

//  radio RSTn 
#define PORT_PIN_RADIO_RESET_HIGH()       
#define PORT_PIN_RADIO_RESET_LOW()        

#endif /* _BOARD_INFO_H */
