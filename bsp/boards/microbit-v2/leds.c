/**
 * Description: microbit-specific definition of the "leds" bsp module.
 */

 #include "stdbool.h"
#include "nrf52833.h"
#include "board_info.h"
#include "leds.h"


//=========================== defines =========================================

#define ROW1_DISCONNECT()    (NRF_P0->PIN_CNF[21]     = 0x00000002 )          // P0.21
#define ROW2_DISCONNECT()    (NRF_P0->PIN_CNF[22]     = 0x00000002 )          // P0.22
#define ROW3_DISCONNECT()    (NRF_P0->PIN_CNF[15]     = 0x00000002 )          // P0.15
#define ROW4_DISCONNECT()    (NRF_P0->PIN_CNF[24]     = 0x00000002 )          // P0.24
#define ROW5_DISCONNECT()    (NRF_P0->PIN_CNF[19]     = 0x00000002 )          // P0.19

#define ROW1_OUTPUT()        (NRF_P0->PIN_CNF[21]     = 0x00000003 )          // P0.21
#define ROW2_OUTPUT()        (NRF_P0->PIN_CNF[22]     = 0x00000003 )          // P0.22
#define ROW3_OUTPUT()        (NRF_P0->PIN_CNF[15]     = 0x00000003 )          // P0.15
#define ROW4_OUTPUT()        (NRF_P0->PIN_CNF[24]     = 0x00000003 )          // P0.24
#define ROW5_OUTPUT()        (NRF_P0->PIN_CNF[19]     = 0x00000003 )          // P0.19

#define ROW1_HIGH()          (NRF_P0->OUTSET          = (0x00000001 << 21) )   // P0.21
#define ROW2_HIGH()          (NRF_P0->OUTSET          = (0x00000001 << 22) )   // P0.22
#define ROW3_HIGH()          (NRF_P0->OUTSET          = (0x00000001 << 15) )   // P0.15
#define ROW4_HIGH()          (NRF_P0->OUTSET          = (0x00000001 << 24) )   // P0.24
#define ROW5_HIGH()          (NRF_P0->OUTSET          = (0x00000001 << 19) )   // P0.19

#define COL1_DISCONNECT()    (NRF_P0->PIN_CNF[28]     = 0x00000002 )           // P0.28
#define COL2_DISCONNECT()    (NRF_P0->PIN_CNF[11]     = 0x00000002 )           // P0.11
#define COL3_DISCONNECT()    (NRF_P0->PIN_CNF[31]     = 0x00000002 )           // P0.31
#define COL4_DISCONNECT()    (NRF_P1->PIN_CNF[05]     = 0x00000002 )           // P1.05
#define COL5_DISCONNECT()    (NRF_P0->PIN_CNF[30]     = 0x00000002 )           // P0.30

#define COL1_OUTPUT()        (NRF_P0->PIN_CNF[28]     = 0x00000003 )           // P0.28
#define COL2_OUTPUT()        (NRF_P0->PIN_CNF[11]     = 0x00000003 )           // P0.11
#define COL3_OUTPUT()        (NRF_P0->PIN_CNF[31]     = 0x00000003 )           // P0.31
#define COL4_OUTPUT()        (NRF_P1->PIN_CNF[05]     = 0x00000003 )           // P1.05
#define COL5_OUTPUT()        (NRF_P0->PIN_CNF[30]     = 0x00000003 )           // P0.30

#define COL1_LOW()           (NRF_P0->OUTCLR          = (0x00000001 << 28) )   // P0.28
#define COL2_LOW()           (NRF_P0->OUTCLR          = (0x00000001 << 11) )   // P0.11
#define COL3_LOW()           (NRF_P0->OUTCLR          = (0x00000001 << 31) )   // P0.31
#define COL4_LOW()           (NRF_P1->OUTCLR          = (0x00000001 << 05) )   // P1.05
#define COL5_LOW()           (NRF_P0->OUTCLR          = (0x00000001 << 30) )   // P0.30

#define COL1_HIGH()           (NRF_P0->OUTSET          = (0x00000001 << 28) )   // P0.28
#define COL2_HIGH()           (NRF_P0->OUTSET          = (0x00000001 << 11) )   // P0.11
#define COL3_HIGH()           (NRF_P0->OUTSET          = (0x00000001 << 31) )   // P0.31
#define COL4_HIGH()           (NRF_P1->OUTSET          = (0x00000001 << 05) )   // P1.05
#define COL5_HIGH()           (NRF_P0->OUTSET          = (0x00000001 << 30) )   // P0.30

typedef enum {
    LED11,LED12,LED13,LED14,LED15,
    LED21,LED22,LED23,LED24,LED25,
    LED31,LED32,LED33,LED34,LED35,
    LED41,LED42,LED43,LED44,LED45,
    LED51,LED52,LED53,LED54,LED55
} led_id_t;


void leds_all_off(void) {
   COL1_HIGH();
   COL2_HIGH();
   COL3_HIGH();
   COL4_HIGH();
   COL5_HIGH();
}


//=========================== prototypes ======================================

void leds_on(led_id_t led_id);
void leds_off(led_id_t led_id);

//=========================== public ==========================================

void leds_init() {

}

//==== error led

void leds_error_off(void) {
  leds_off(LED11);
}

void leds_error_on(void) {
  leds_on(LED11);
}

void leds_error_toggle(void) {
    if (leds_error_isOn()==0) {
        leds_error_on();
    } else {
        leds_error_off();
    }
}

uint8_t leds_error_isOn(void) {
  uint32_t row = NRF_P0->OUT & (1<<21);
  uint32_t col = NRF_P0->OUT & (1<<28);
    if (row  && !col) {
        return 1;
    } else {
        return 0;
    }
}

//==== sync led

void leds_sync_off(void) {
  leds_off(LED12);
}

void leds_sync_on(void) {
  leds_on(LED12);
}

void leds_sync_toggle(void) {
    if (leds_sync_isOn()==0) {
        leds_sync_on();
    } else {
        leds_sync_off();
    }
}

uint8_t leds_sync_isOn(void) {
  uint32_t row = NRF_P0->OUT & (1<<21);
  uint32_t col = NRF_P0->OUT & (1<<11);
    if (row  && !col) {
        return 1;
    } else {
        return 0;
    }
}

//==== radio led

void leds_radio_off(void) {
  leds_off(LED13);
}

void leds_radio_on(void) {
  leds_on(LED13);
}

void leds_radio_toggle(void) {
    if (leds_radio_isOn()==0) {
        leds_radio_on();
    } else {
        leds_radio_off();
    }
}

uint8_t leds_radio_isOn(void) {
  uint32_t row = NRF_P0->OUT & (1<<21);
  uint32_t col = NRF_P0->OUT & (1<<31);
    if (row  && !col) {
        return 1;
    } else {
        return 0;
    }
}

//==== debug led

void leds_debug_off(void) {
  leds_off(LED14);
}

void leds_debug_on(void) {
  leds_on(LED14);
}

void leds_debug_toggle(void) {
    if (leds_debug_isOn()==0) {
        leds_debug_on();
    } else {
        leds_debug_off();
    }
}

uint8_t leds_debug_isOn(void) {
  uint32_t row = NRF_P0->OUT & (1<<21);
  uint32_t col = NRF_P1->OUT & (1<<05);
    if (row  && !col) {
        return 1;
    } else {
        return 0;
    }
}

//==== all leds

void leds_all_on(void) {
    leds_radio_on();
    leds_sync_on();
    leds_debug_on();
    leds_error_on();
}

void leds_all_toggle(void) {
    leds_radio_toggle();
    leds_sync_toggle();
    leds_debug_toggle();
    leds_error_toggle();
}

void leds_error_blink(void) {
    
    uint8_t i;
    uint32_t j;

    // turn all LEDs off
    leds_all_off();

    // blink error LED for ~10s
    for (i = 0; i < 100; i++) {
        leds_error_toggle();
        for(j=0;j<0x1ffff;j++);
    }
}

void leds_circular_shift(void) {
  leds_increment();
}

void leds_increment(void) {
    
    if (leds_error_isOn()) {
      leds_error_off();
      leds_sync_on();
    } else if(leds_sync_isOn()){
      leds_sync_off();
      leds_radio_on();
    } else if (leds_radio_isOn()) {
      leds_radio_off();
      leds_debug_on(); 
    } else if (leds_debug_isOn()){
      leds_debug_off();
    } else {
      leds_error_on();
    }
}

//=========================== private =========================================

void leds_on(led_id_t led_id) {

    switch(led_id) {

        case LED11: ROW1_OUTPUT(); ROW1_HIGH(); COL1_OUTPUT(); COL1_LOW(); break;
        case LED12: ROW1_OUTPUT(); ROW1_HIGH(); COL2_OUTPUT(); COL2_LOW(); break;
        case LED13: ROW1_OUTPUT(); ROW1_HIGH(); COL3_OUTPUT(); COL3_LOW(); break;
        case LED14: ROW1_OUTPUT(); ROW1_HIGH(); COL4_OUTPUT(); COL4_LOW(); break;
        case LED15: ROW1_OUTPUT(); ROW1_HIGH(); COL5_OUTPUT(); COL5_LOW(); break;

        case LED21: ROW2_OUTPUT(); ROW2_HIGH(); COL1_OUTPUT(); COL1_LOW(); break;
        case LED22: ROW2_OUTPUT(); ROW2_HIGH(); COL2_OUTPUT(); COL2_LOW(); break;
        case LED23: ROW2_OUTPUT(); ROW2_HIGH(); COL3_OUTPUT(); COL3_LOW(); break;
        case LED24: ROW2_OUTPUT(); ROW2_HIGH(); COL4_OUTPUT(); COL4_LOW(); break;
        case LED25: ROW2_OUTPUT(); ROW2_HIGH(); COL5_OUTPUT(); COL5_LOW(); break;

        case LED31: ROW3_OUTPUT(); ROW3_HIGH(); COL1_OUTPUT(); COL1_LOW(); break;
        case LED32: ROW3_OUTPUT(); ROW3_HIGH(); COL2_OUTPUT(); COL2_LOW(); break;
        case LED33: ROW3_OUTPUT(); ROW3_HIGH(); COL3_OUTPUT(); COL3_LOW(); break;
        case LED34: ROW3_OUTPUT(); ROW3_HIGH(); COL4_OUTPUT(); COL4_LOW(); break;
        case LED35: ROW3_OUTPUT(); ROW3_HIGH(); COL5_OUTPUT(); COL5_LOW(); break;

        case LED41: ROW4_OUTPUT(); ROW4_HIGH(); COL1_OUTPUT(); COL1_LOW(); break;
        case LED42: ROW4_OUTPUT(); ROW4_HIGH(); COL2_OUTPUT(); COL2_LOW(); break;
        case LED43: ROW4_OUTPUT(); ROW4_HIGH(); COL3_OUTPUT(); COL3_LOW(); break;
        case LED44: ROW4_OUTPUT(); ROW4_HIGH(); COL4_OUTPUT(); COL4_LOW(); break;
        case LED45: ROW4_OUTPUT(); ROW4_HIGH(); COL5_OUTPUT(); COL5_LOW(); break;

        case LED51: ROW5_OUTPUT(); ROW5_HIGH(); COL1_OUTPUT(); COL1_LOW(); break;
        case LED52: ROW5_OUTPUT(); ROW5_HIGH(); COL2_OUTPUT(); COL2_LOW(); break;
        case LED53: ROW5_OUTPUT(); ROW5_HIGH(); COL3_OUTPUT(); COL3_LOW(); break;
        case LED54: ROW5_OUTPUT(); ROW5_HIGH(); COL4_OUTPUT(); COL4_LOW(); break;
        case LED55: ROW5_OUTPUT(); ROW5_HIGH(); COL5_OUTPUT(); COL5_LOW(); break;
    }
}

void leds_off(led_id_t led_id) {

    switch(led_id) {

        case LED11:  COL1_HIGH(); break;
        case LED12:  COL2_HIGH(); break;
        case LED13:  COL3_HIGH(); break;
        case LED14:  COL4_HIGH(); break;
        case LED15:  COL5_HIGH(); break;

        case LED21:  COL1_HIGH(); break;
        case LED22:  COL2_HIGH(); break;
        case LED23:  COL3_HIGH(); break;
        case LED24:  COL4_HIGH(); break;
        case LED25:  COL5_HIGH(); break;

        case LED31:  COL1_HIGH(); break;
        case LED32:  COL2_HIGH(); break;
        case LED33:  COL3_HIGH(); break;
        case LED34:  COL4_HIGH(); break;
        case LED35:  COL5_HIGH(); break;

        case LED41:  COL1_HIGH(); break;
        case LED42:  COL2_HIGH(); break;
        case LED43:  COL3_HIGH(); break;
        case LED44:  COL4_HIGH(); break;
        case LED45:  COL5_HIGH(); break;

        case LED51:  COL1_HIGH(); break;
        case LED52:  COL2_HIGH(); break;
        case LED53:  COL3_HIGH(); break;
        case LED54:  COL4_HIGH(); break;
        case LED55:  COL5_HIGH(); break;
    }
}
