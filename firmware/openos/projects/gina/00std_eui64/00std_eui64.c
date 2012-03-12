/**
\brief This is a standalone program to program the EUI-64 address of a the GINA and
basestation GINA boards.

Set the variables 'board_type', 'board_version' and 'board_id', compile and run the
program.

It will store the corresponding EUI64 into the information memory space of
the MSP430, which does not get erased, ever. The available memory ranges are:
  - 0x10ee-0x10f5 [8B] to store the EUI64
  - 0x10c4-0x10d9 [22B] reserved for future use.

The EUI-64 naming convention is: 14-15-92-xx yy-yy-zz-zz 
where:
- xx is the type of board:
  . 0x09 for GINA ('9' looks like a 'g')
  . 0x0b for basestation GINA
- yy-yy is the version
  . 0x022b for a GINA 2.2b
  . 0x022c for a GINA 2.2c
  . 0x0301 for a GINA basestation MSP1 ('3' looks like a rotated 'm')
- zz-zz is the unique identifier of the board, in hex, big endian (the one 
  printed on the sticker).
 
\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, August 2010
*/

/* 
   =!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!
   CAREFUL! if the board is reset in the second half of this code, the MSP is dead.
   Therefore, before running, make a screen capture of the INFO memory contents. If
   you don't know *exactly* what you are doing, ask watteyne@eecs.berkeley.edu.
   =!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!=!
*/

#include "msp430x26x.h"
#include "stdint.h"

//===== start editing according to the label printed on the board =============
#define LABEL_00E1
//===== stop editing ==========================================================

enum {
   BOARD_TYPE_GINA           = 0x09,
   BOARD_TYPE_BASETATION     = 0x0b,
};

enum {
   BOARD_VERSION_22a         = 0x022a,
   BOARD_VERSION_22b         = 0x022b,
   BOARD_VERSION_22c         = 0x022c,
   BOARD_VERSION_MSP1        = 0x0301,
};

#include "00std_eui64.h"

#ifdef BOARD_CALIBRATION

__no_init volatile uint8_t segment_D @ 0x1000;
__no_init volatile uint8_t segment_C @ 0x1040;
__no_init volatile uint8_t segment_B @ 0x1080;
__no_init volatile uint8_t segment_A @ 0x10c0;

int main(void)
{
   uint8_t i;
   uint8_t default_config[64] = BOARD_CALIBRATION;

   WDTCTL  = WDTPW + WDTHOLD;                    // disable watchdog timer

   if (CALBC1_1MHZ==0xFF || CALDCO_1MHZ==0xFF) { // calibration constants erased
      // set bits to mess with flash
      FCTL2 = FWKEY + FSSEL0 + FN1;              // MCLK/3 for Flash Timing Generator
      FCTL3 = FWKEY + LOCKA;                     // clear LOCK and LOCKA bits
      FCTL1 = FWKEY + ERASE;                     // set ERASE bit
      // erase segment_A
      *(&segment_A+0) = 0;                       // dummy write to somewhere in segmend_A to erase
      FCTL1 = FWKEY + WRT;                       // set WRT bit for write operation
      // copy default_config to segment_A
      for (i=0;i<64;i++) {
         *(&segment_A+i) = default_config[i];
      }
      // clear bits to mess with flash
      FCTL1 = FWKEY;                             // clear WRT bit
      FCTL3 = FWKEY + LOCKA + LOCK;              // set LOCK and LOCKA bits
      __bis_SR_register(LPM4_bits);              // sleep
   }

   BCSCTL1 = CALBC1_1MHZ;                        // MCLK at 1MHz
   DCOCTL  = CALDCO_1MHZ;

   // ============ prepare  the contents for segment_A in segment D ===========

   // set bits to mess with flash
   FCTL2 = FWKEY + FSSEL0 + FN1;                 // MCLK/3 for Flash Timing Generator
   FCTL3 = FWKEY;                                // clear LOCK bit
   FCTL1 = FWKEY + ERASE;                        // set ERASE bit
   // erase segment_D
   *(&segment_D+0) = 0;                          // dummy write to somewhere in segmend_D to erase
   FCTL1 = FWKEY + WRT;                          // set WRT bit for write operation
   // copy first half of segment_A to segment_D
   for (i=0;i<46;i++) {
      *(&segment_D+i) = *(&segment_A+i);
   }
   // write EUI-64 space in segment_D
   *(&segment_D+46) = 0x14;
   *(&segment_D+47) = 0x15;
   *(&segment_D+48) = 0x92;
   *(&segment_D+49) = BOARD_TYPE;
   *(&segment_D+50) = (uint8_t)((BOARD_VERSION>>8)&0x00FF);
   *(&segment_D+51) = (uint8_t)((BOARD_VERSION   )&0x00FF);
   *(&segment_D+52) = (uint8_t)((BOARD_NUMBER>>8)&0x00FF);
   *(&segment_D+53) = (uint8_t)((BOARD_NUMBER   )&0x00FF);
   // copy second half of segment_A to segment_D
   for (i=54;i<64;i++) {
      *(&segment_D+i) = *(&segment_A+i);
   }
   // clear bits to mess with flash
   FCTL1 = FWKEY;                                // clear WRT bit
   FCTL3 = FWKEY + LOCK;                         // set LOCK bit

   //======================== write segment_A =================================

   // set bits to mess with flash
   FCTL2 = FWKEY + FSSEL0 + FN1;                 // MCLK/3 for Flash Timing Generator
   FCTL3 = FWKEY + LOCKA;                        // clear LOCK and LOCKA bits
   FCTL1 = FWKEY + ERASE;                        // set ERASE bit
   // erase segment_A
   *(&segment_A+0) = 0;                          // dummy write to somewhere in segmend_A to erase
   FCTL1 = FWKEY + WRT;                          // set WRT bit for write operation
   // copy segment_D to segment_A
   for (i=0;i<64;i++) {
      *(&segment_A+i) = *(&segment_D+i);
   }
   // clear bits to mess with flash
   FCTL1 = FWKEY;                                // clear WRT bit
   FCTL3 = FWKEY + LOCKA + LOCK;                 // set LOCK and LOCKA bits

   __bis_SR_register(LPM4_bits);                 // sleep
}

#endif

