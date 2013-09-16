/**
\brief This is a standalone test program for retrieving the unique identifier
      from the DS2411 chip on the TelosB.

The datasheet of the chip at http://pdfserv.maxim-ic.com/en/ds/DS2411.pdf.

Run the program, put a breakpoint a the last line of main(), and when you get
there, watch variable eui. I contains the 64-bits read from the DS2411, i.e.
- [1B] CRC8
- [6B] unique 48-bit identifier
- [1B] always 0x01
 
\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, March 2012.
*/

#include "msp430f1611.h"
#include "stdint.h"
#include "string.h"

//=========================== defines =========================================

#define PIN_1WIRE 0x10

//=========================== enums ===========================================

enum  {
   OW_DLY_A = 6,
   OW_DLY_B = 64,
   OW_DLY_C = 60,
   OW_DLY_D = 10,
   OW_DLY_E = 9,
   OW_DLY_F = 55,
   OW_DLY_G = 0,
   OW_DLY_H = 480,
   OW_DLY_I = 90,
   OW_DLY_J = 220,
};

//=========================== prototypes ======================================

// chip
void    owchip_geteui(uint8_t* eui);
// 1Wire
uint8_t ow_reset();
void    ow_write_byte(uint8_t byte);
uint8_t ow_read_byte();
void    ow_write_bit(int is_one);
uint8_t ow_read_bit();
void    ow_write_bit_one();
void    ow_write_bit_zero();
// CRC
uint8_t crc8_byte(uint8_t crc, uint8_t byte);
uint8_t crc8_bytes(uint8_t crc, uint8_t* bytes, uint8_t len);
// timer
void    delay_us(uint16_t delay);
// pin
void    owpin_init();
void    owpin_output_low();
void    owpin_output_high();
void    owpin_prepare_read();
uint8_t owpin_read();

//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int main(void) {
   uint8_t eui[8];
   
   WDTCTL     =  WDTPW + WDTHOLD;                // disable watchdog timer
   
   DCOCTL     =  DCO0 | DCO1 | DCO2;             // MCLK at 8MHz
   BCSCTL1    =  RSEL0 | RSEL1 | RSEL2;          // MCLK at 8MHz
   
   owchip_geteui(eui);
   
   __bis_SR_register(GIE+LPM4_bits);             // sleep
}

//=========================== private =========================================

//===== chip

void owchip_geteui(uint8_t* eui) {          // >= 6000us
   uint8_t  id[8];
   int      retry;
   int      crc;
   uint8_t* byte;
   uint16_t oldTactl;
   
   retry = 5;
   memset(eui,0,8);
   
   // store current value of TACTL
   oldTactl   = TACTL;
   
   // start timer in continuous mode at 1MHz
   TACTL      = TASSEL_2 | ID_2 | MC_2;
   
   owpin_init();
   while (retry-- > 0) {
      crc = 0;
      
      if(ow_reset()) {
         ow_write_byte(0x33); //read rom
         for(byte=id+7; byte>=id; byte--) {
            crc = crc8_byte( crc, *byte=ow_read_byte() );
         }
         if(crc==0) {
            // CRC valid
            memcpy(eui,id,8);
         }
      }
   }
   
   // restore value of TACTL
   TACTL = oldTactl;
}

//===== 1Wire

// admin

uint8_t ow_reset() {                    // >= 960us 
   int present;
   owpin_output_low();
   delay_us(OW_DLY_H);             // t_RSTL
   owpin_prepare_read();
   delay_us(OW_DLY_I);             // t_MSP
   present = owpin_read();
   delay_us(OW_DLY_J);             // t_REC
   return (present==0);
}

// byte-level access

void ow_write_byte(uint8_t byte) {   // >= 560us
   uint8_t bit;
   for(bit=0x01;bit!=0;bit<<=1) {
      ow_write_bit(byte & bit);
   }
}

uint8_t ow_read_byte() {             // >= 560us
   uint8_t byte = 0;
   uint8_t bit;
   for( bit=0x01; bit!=0; bit<<=1 ) {
      if(ow_read_bit()) {
         byte |= bit;
      }
   }
   return byte;
}

// bit-level access

void ow_write_bit(int is_one) {      // >= 70us
   if(is_one) {
      ow_write_bit_one();
   } else {
      ow_write_bit_zero();
   }
}

uint8_t ow_read_bit() {                 // >= 70us
   int bit;
   owpin_output_low();
   delay_us(OW_DLY_A);             // t_RL
   owpin_prepare_read();
   delay_us(OW_DLY_E);             // near-max t_MSR
   bit = owpin_read();
   delay_us(OW_DLY_F);             // t_REC
   return bit;
}

void ow_write_bit_one() {            // >= 70us
   owpin_output_low();
   delay_us(OW_DLY_A);             // t_W1L
   owpin_output_high();
   delay_us(OW_DLY_B);             // t_SLOT - t_W1L
}

void ow_write_bit_zero() {           // >= 70us
   owpin_output_low();
   delay_us(OW_DLY_C);             // t_W0L
   owpin_output_high();
   delay_us(OW_DLY_D);             // t_SLOT - t_W0L
}

//===== CRC

uint8_t crc8_byte(uint8_t crc, uint8_t byte) {
   int i;
   crc ^= byte;
   for( i=0; i<8; i++ )
   {
      if( crc & 1 )
         crc = (crc >> 1) ^ 0x8c;
      else
         crc >>= 1;
   }
   return crc;
}

uint8_t crc8_bytes(uint8_t crc, uint8_t* bytes, uint8_t len) {
   uint8_t* end = bytes+len;
   while( bytes != end )
      crc = crc8_byte( crc, *bytes++ );
   return crc;
}

//===== timer

void delay_us(uint16_t delay) {
   uint16_t startTime;
   startTime = TAR;
   while (TAR<startTime+delay);
}

//===== pin

void    owpin_init() {
   P2DIR &= ~PIN_1WIRE; // set as input
   P2OUT &= ~PIN_1WIRE; // pull low
}

void    owpin_output_low() {
   P2DIR |=  PIN_1WIRE; // set as output
}

void    owpin_output_high() {
   P2DIR &= ~PIN_1WIRE; // set as input
}

void    owpin_prepare_read() {
   P2DIR &= ~PIN_1WIRE; // set as input
}

uint8_t owpin_read() {
   return (P2IN & PIN_1WIRE);
}