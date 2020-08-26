/**
\brief nRF5340-specific standalone leds project.

\author Tengfei Chang <tengfei.chang@gmail.com>, August 2020.
*/

#include "nrf5340_network.h"
#include "nrf5340_network_bitfields.h"

//=========================== defines =========================================

#define NRF_GPIO_PIN_MAP(port, pin) (((port) << 5) | ((pin) & 0x1F))

#define LED_ERROR          NRF_GPIO_PIN_MAP(0,28) // p0.28
#define LED_RADIO          NRF_GPIO_PIN_MAP(0,29) // p0.29
#define LED_SYNC           NRF_GPIO_PIN_MAP(0,30) // p0.30
#define LED_DEBUG          NRF_GPIO_PIN_MAP(0,31) // p0.31

//=========================== variables =======================================

//=========================== prototypes ======================================

void clocks_start(void);
void nrf_gpio_cfg_output(uint32_t pin_number);

//=========================== main ============================================

int main(void) {

    volatile uint32_t output_status;
    uint32_t delay;

    clocks_start();

    // init gpio

    nrf_gpio_cfg_output(LED_ERROR);
    nrf_gpio_cfg_output(LED_RADIO);
    nrf_gpio_cfg_output(LED_SYNC);
    nrf_gpio_cfg_output(LED_DEBUG);


    // toggle gpio

    while (1) {

        output_status = ((uint32_t)(1 << LED_ERROR)) & (NRF_P0_NS->OUT);

        if (output_status==0){
            // it is on , turn off led
            NRF_P0_NS->OUTSET =  1 << LED_ERROR;
        } else {
            // it is off, turn on led
            NRF_P0_NS->OUTCLR =  1 << LED_ERROR;
        }

        for (delay=0;delay<0xfffff;delay++);
    }

 }

//=========================== public ==========================================



//=========================== private =========================================

void clocks_start( void ){

    // Start HFCLK and wait for it to start.
    NRF_CLOCK_NS->EVENTS_HFCLKSTARTED = 0;
    NRF_CLOCK_NS->TASKS_HFCLKSTART = 1;
    while (NRF_CLOCK_NS->EVENTS_HFCLKSTARTED == 0);

    while (NRF_CLOCK_NS->EVENTS_HFCLKSTARTED == 0);
}


void nrf_gpio_cfg_output(uint32_t pin_number){
 

    NRF_P0_NS->PIN_CNF[pin_number] =   \
           ((uint32_t)GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos)
         | ((uint32_t)GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos)
         | ((uint32_t)GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos)
         | ((uint32_t)GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos)
         | ((uint32_t)GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);

}