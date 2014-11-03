/**
\brief WSN430v14-specific definition of the "eui64" bsp module.

This reads a 64-bit identifier from the DS2411 chip 
(http://datasheets.maximintegrated.com/en/ds/DS2411.pdf), which is formatted
as follows:
- [1B] CRC code
- [6B] serial number
- [1B] family code

This module will create an EUI64 as follows:
- extract the identifier from the DS2411, and only continue if the 8-bit CRC
matches
- create a EUI as follows:
    . [3B] OpenWSN OUI hex(14-15-92)
    . [5B] 5 last bytes from 6-byte serial number

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, March 2012.
*/

#include "msp430f1611.h"
#include "string.h"
#include "eui64.h"

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

//=========================== variables =======================================

//=========================== prototypes ======================================

// 1Wire
uint8_t ow_reset(void);
void    ow_write_byte(uint8_t byte);
uint8_t ow_read_byte(void);
void    ow_write_bit(int is_one);
uint8_t ow_read_bit(void);
void    ow_write_bit_one(void);
void    ow_write_bit_zero(void);
// CRC
uint8_t crc8_byte(uint8_t crc, uint8_t byte);
uint8_t crc8_bytes(uint8_t crc, uint8_t* bytes, uint8_t len);
// timer
void    delay_us(uint16_t delay);
// pin
void    owpin_init(void);
void    owpin_output_low(void);
void    owpin_output_high(void);
void    owpin_prepare_read(void);
uint8_t owpin_read(void);

//=========================== public ==========================================

void eui64_get(uint8_t* addressToWrite) {    // >= 6000us
   uint8_t  id[8];
   int      retry;
   int      crc;
   uint8_t* byte;
   uint16_t oldTactl;
   
   // reset EUI64 to default value
   memset(addressToWrite,0x00,8);
   
   // store current value of TACTL
   oldTactl   = TACTL;
   
   // start timer in continuous mode at 1MHz
   TACTL      = TASSEL_2 | ID_0 | MC_2;
   
   // initializer 1-Wire pin
   owpin_init();
   
   retry = 10;
   while (retry-- > 0) {
      crc = 0;
      
      if(ow_reset()) {
         ow_write_byte(0x33); //read rom
         for(byte=id+7; byte>=id; byte--) {
            crc = crc8_byte( crc, *byte=ow_read_byte() );
         }
         if(crc==0) {
            // CRC valid
            
            // create EUI64
            addressToWrite[0] = 0x14;   // OpenWSN OUI
            addressToWrite[1] = 0x15;
            addressToWrite[2] = 0x92;
            addressToWrite[3] = id[2]; // last 5 bytes of 6B board ID
            addressToWrite[4] = id[3];
            addressToWrite[5] = id[4];
            addressToWrite[6] = id[5];
            addressToWrite[7] = id[6];
            break;
         }
      }
   }
   
   // restore value of TACTL
   TACTL = oldTactl;
}

//=========================== private =========================================

//===== 1Wire

// admin

uint8_t ow_reset() {              // >= 960us 
   int present;
   owpin_output_low();
   delay_us(OW_DLY_H);            // t_RSTL
   owpin_prepare_read();
   delay_us(OW_DLY_I);            // t_MSP
   present = owpin_read();
   delay_us(OW_DLY_J);            // t_REC
   return (present==0);
}

// byte-level access

void ow_write_byte(uint8_t byte) {// >= 560us
   uint8_t bit;
   for(bit=0x01;bit!=0;bit<<=1) {
      ow_write_bit(byte & bit);
   }
}

uint8_t ow_read_byte() {          // >= 560us
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

void ow_write_bit(int is_one) {   // >= 70us
   if(is_one) {
      ow_write_bit_one();
   } else {
      ow_write_bit_zero();
   }
}

uint8_t ow_read_bit() {           // >= 70us
   int bit;
   owpin_output_low();
   delay_us(OW_DLY_A);            // t_RL
   owpin_prepare_read();
   delay_us(OW_DLY_E);            // near-max t_MSR
   bit = owpin_read();
   delay_us(OW_DLY_F);            // t_REC
   return bit;
}

void ow_write_bit_one() {         // >= 70us
   owpin_output_low();
   delay_us(OW_DLY_A);            // t_W1L
   owpin_output_high();
   delay_us(OW_DLY_B);            // t_SLOT - t_W1L
}

void ow_write_bit_zero() {        // >= 70us
   owpin_output_low();
   delay_us(OW_DLY_C);            // t_W0L
   owpin_output_high();
   delay_us(OW_DLY_D);            // t_SLOT - t_W0L
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
   P2DIR &= ~PIN_1WIRE;           // set as input
   P2OUT &= ~PIN_1WIRE;           // pull low
}

void    owpin_output_low() {
   P2DIR |=  PIN_1WIRE;           // set as output
}

void    owpin_output_high() {
   P2DIR &= ~PIN_1WIRE;           // set as input
}

void    owpin_prepare_read() {
   P2DIR &= ~PIN_1WIRE;           // set as input
}

uint8_t owpin_read() {
   return (P2IN & PIN_1WIRE);
}
