/*
 * File:        tower.h
 * Purpose:     Kinetis tower CPU card definitions
 *
 * Notes:
 */

#ifndef __TOWER_H__
#define __TOWER_H__

#include "mcg.h"
#include "board.h"

/********************************************************************/


 /*
   * Input Clock Info
   */
  #define CLK0_FREQ_HZ        8000000
  #define CLK0_TYPE           CRYSTAL

  /*
   * PLL Configuration Info
   */
//  #define NO_PLL_INIT

/* The expected PLL output frequency is:
 * PLL out = (((CLKIN/PRDIV) x VDIV))
 * where the CLKIN can be either CLK0_FREQ_HZ or CLK1_FREQ_HZ.
 * 
 * For more info on PLL initialization refer to the mcg driver files.
 */

  #define PLL0_PRDIV      4
  #define PLL0_VDIV       36

  /*
   * System Bus Clock Info
   */
  #define REF_CLK             XTAL8
  #define CORE_CLK_MHZ        PLL72      /* system/core frequency in MHz */

  /* Serial Port Info */
  #define TERM_PORT           UART1_BASE_PTR	/* OSJTAG serial port or TWR-SER COM port */
  #define TERMINAL_BAUD       115200
  #undef  HW_FLOW_CONTROL


#endif /* __TOWER_H__ */
