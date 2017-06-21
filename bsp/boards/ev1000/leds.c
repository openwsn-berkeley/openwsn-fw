/**
\brief ev1000 definition of the "leds" bsp module.

\author Chang Tengfei <tengfei.chang@gmail.com>,  July 2012.
\author Jean-Michel Rubillon <jmrubillon@theiet.org>, April 2017
*/

#include "stm32f10x_lib.h"
#include "leds.h"

//=========================== defines =========================================
#define LED_ERROR GPIO_Pin_7
#define LED_DEBUG GPIO_Pin_9
#define LED_SYNC  GPIO_Pin_8
#define LED_RADIO GPIO_Pin_6
#define LED_ALL (LED_RADIO | LED_DEBUG | LED_ERROR | LED_SYNC)
//=========================== variables =======================================

//=========================== prototypes ======================================

void Delay(void);

//=========================== public ==========================================

void leds_init() {
    
    // Enable GPIOC clock
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC , ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = LED_ALL;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

// red LED6
void leds_error_on() {

    GPIO_SetBits(GPIOC, LED_ERROR);
}

void leds_error_off() {

    GPIO_ResetBits(GPIOC, LED_ERROR);
}

void leds_error_toggle() {

	if( GPIO_ReadOutputDataBit(GPIOC, LED_ERROR) ){
		leds_error_off();
	} else {
		leds_error_on();
	}
}

uint8_t leds_error_isOn(){

    uint8_t bitstatus = 0x00;
    if(GPIO_ReadOutputDataBit(GPIOC, LED_ERROR)) {
        bitstatus = 0x01;
    } else {
        bitstatus = 0x00;
    }
    return bitstatus;
}

void leds_error_blink(){

    for(int i=0;i<16;i++) {
        leds_error_toggle();
        Delay();
    }
}

// Yellow LED5
void leds_radio_on() {
    
    GPIO_SetBits(GPIOC, LED_RADIO);
}

void leds_radio_off() {
    
    GPIO_ResetBits(GPIOC, LED_RADIO);
}

void leds_radio_toggle() {
    
	if( GPIO_ReadOutputDataBit(GPIOC, LED_RADIO) ){
		leds_radio_off();
	} else {
		leds_radio_on();
	}
}

uint8_t leds_radio_isOn() {
    
    uint8_t bitstatus = 0x00;
    if(GPIO_ReadOutputDataBit(GPIOC, LED_RADIO)) {
        bitstatus = 0x01;
    } else {
        bitstatus = 0x00;
    }
    return bitstatus;
}

// Yellow LED7
void leds_sync_on() {
    
    GPIO_SetBits(GPIOC, LED_SYNC);
}

void leds_sync_off() {
    
    GPIO_ResetBits(GPIOC, LED_SYNC);
}

void leds_sync_toggle() {
    
	if( GPIO_ReadOutputDataBit(GPIOC, LED_SYNC) ){
		leds_sync_off();
	} else {
		leds_sync_on();
	}
}

uint8_t leds_sync_isOn() {
    
    uint8_t bitstatus = 0x00;
    if(GPIO_ReadOutputDataBit(GPIOC, LED_SYNC)) {
        bitstatus = 0x01;
    } else {
        bitstatus = 0x00;
    }
    return bitstatus;
}

// red LED8
void leds_debug_on() {
    
    GPIO_SetBits(GPIOC, LED_DEBUG);
}

void leds_debug_off(){
    
    GPIO_ResetBits(GPIOC, LED_DEBUG);
}

void leds_debug_toggle(){
    
	if( GPIO_ReadOutputDataBit(GPIOC, LED_DEBUG) ){
		leds_debug_off();
	} else {
		leds_debug_on();
	}
}

uint8_t leds_debug_isOn(){
    
    uint8_t bitstatus = 0x00;
    if(GPIO_ReadOutputDataBit(GPIOC, LED_DEBUG)) {
        bitstatus = 0x01;
    } else {
        bitstatus = 0x00;
    }
    return bitstatus;
}

void leds_all_on() {
    
    GPIO_SetBits(GPIOC, LED_ALL);
}

void leds_all_off() {
    GPIO_ResetBits(GPIOC, LED_ALL);
}
void leds_all_toggle() {
	
    uint16_t state = GPIO_ReadOutputData(GPIOC);
    state ^= LED_ALL;
	GPIO_Write(GPIOC, state);
}

void leds_circular_shift() {

	volatile uint16_t leds_state = 0;
	leds_state = GPIO_ReadOutputData(GPIOC);
	leds_state <<= 1;
	leds_state &= LED_ALL;
	GPIO_Write(GPIOC, leds_state);
    Delay();
}

void leds_increment() {

	volatile uint16_t leds_state = 0;
	leds_state = GPIO_ReadOutputData(GPIOC);
	leds_state += GPIO_Pin_6;
	leds_state &= LED_ALL;
	GPIO_Write(GPIOC, leds_state);
	Delay();
}

//=========================== private =========================================

void Delay(void){
    
    unsigned long ik;
    for(ik=0;ik<0x7fff8;ik++) ;
}