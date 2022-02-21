/**
\brief nRF5340-specific standalone leds project.

\author Tengfei Chang <tengfei.chang@gmail.com>, August 2020.
*/

#include "nrf5340_application.h"
#include "nrf5340_application_bitfields.h"

//=========================== defines =========================================

#define LED_PORT          0
#define LED_ERROR         28 // p0.28
#define LED_RADIO         29 // p0.29
#define LED_SYNC          30 // p0.30
#define LED_DEBUG         31 // p0.31

//=========================== variables =======================================

//=========================== prototypes ======================================

void clocks_start(void);
void nrf_gpio_cfg_output(uint8_t port_number, uint32_t pin_number);

//=========================== main ============================================

int main(void) {

    volatile uint32_t output_status;
    uint32_t delay;
    uint8_t  i;

    clocks_start();

    // init gpio

    nrf_gpio_cfg_output(LED_PORT, LED_ERROR);
    nrf_gpio_cfg_output(LED_PORT, LED_RADIO);
    nrf_gpio_cfg_output(LED_PORT, LED_SYNC);
    nrf_gpio_cfg_output(LED_PORT, LED_DEBUG);


    // toggle gpio

    i = 0;
    while (i<10) {

        output_status = ((uint32_t)(1 << LED_ERROR)) & (NRF_P0_S->OUT);

        if (output_status==0){
            // it is on , turn off led
            NRF_P0_S->OUTSET =  1 << LED_ERROR;
        } else {
            // it is off, turn on led
            NRF_P0_S->OUTCLR =  1 << LED_ERROR;
        }

        for (delay=0;delay<0xffff;delay++);
        i++;
    }

    // release GPIO: P0.19-31

    for (i=19;i<32;i++) {
        NRF_P0_S->PIN_CNF[i] |= 0x10000000;
    }

    for (i=4;i<16;i++) {
        NRF_P1_S->PIN_CNF[i] |= 0x10000000;
    }

    // release networking core

    if (NRF_RESET_S->NETWORK.FORCEOFF == 0x1) {
        NRF_RESET_S->NETWORK.FORCEOFF = 0x0;
    }

    while(1);

 }

//=========================== public ==========================================



//=========================== private =========================================

void clocks_start( void ){

    // Start HFCLK and wait for it to start.
    NRF_CLOCK_S->EVENTS_HFCLKSTARTED = 0;
    NRF_CLOCK_S->TASKS_HFCLKSTART = 1;
    while (NRF_CLOCK_S->EVENTS_HFCLKSTARTED == 0);

    while (NRF_CLOCK_S->EVENTS_HFCLKSTARTED == 0);
}

void nrf_gpio_cfg_output(uint8_t port_number, uint32_t pin_number){
 
    switch (port_number) {
    case 0:
        NRF_P0_S->PIN_CNF[pin_number] =   \
               ((uint32_t)GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos)
             | ((uint32_t)GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos)
             | ((uint32_t)GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos)
             | ((uint32_t)GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos)
             | ((uint32_t)GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);
        break; 
    case 1:
        NRF_P1_S->PIN_CNF[pin_number] =   \
               ((uint32_t)GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos)
             | ((uint32_t)GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos)
             | ((uint32_t)GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos)
             | ((uint32_t)GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos)
             | ((uint32_t)GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);
        break;
    default:
        // invalid port number
        break;
    }
}