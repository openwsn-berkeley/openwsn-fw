/**
\brief nRF52840-specific definition of the "i2c" bsp module.

\author Tengfei Chang <tengfei.chang@gmail.com>, Nov 2021.
*/


#include "nrf52840.h"
#include "nrf52840_bitfields.h"
#include "opendefs.h"
#include "i2c.h"

//=========================== define ==========================================

#define NRF_GPIO_PIN_MAP(port, pin) (((port) << 5) | ((pin) & 0x1F))

#define DOF10_SCL_PIN   NRF_GPIO_PIN_MAP(1,0)   // SCL signal pin P1.00
#define DOF10_SDA_PIN   NRF_GPIO_PIN_MAP(0,24)  // SDA signal pin P0.24
#define DOF10_FREQ      0x06400000              // frequency 6400000->400kbps

#define TWIM_INTENSET_STOPPED_POS    1
#define TWIM_INTENSET_ERROR_POS      9
#define TWIM_INTENSET_SUSPENDED_POS  18
#define TWIM_INTENSET_RXSTARTED_POS  19
#define TWIM_INTENSET_TXSTARTED_POS  20
#define TWIM_INTENSET_LASTRX_POS     23
#define TWIM_INTENSET_LASTTX_POS     24

#define TWI_INT_ENABLED       0

//=========================== variables =======================================

typedef struct {
    uint8_t   i2c_addr;

} i2c_vars_t; 

//=========================== prototypes ======================================

void nrf_gpio_cfg_input(uint32_t pin_number);

//=========================== public ==========================================

void i2c_init(void) {

    nrf_gpio_cfg_input(DOF10_SCL_PIN);
    nrf_gpio_cfg_input(DOF10_SDA_PIN);

    // make sure TWIM0 is disabled before configuring the GPIO pins
    NRF_TWIM0->ENABLE = (TWIM_ENABLE_ENABLE_Disabled << TWIM_ENABLE_ENABLE_Pos);

    NRF_TWIM0->PSEL.SCL   = DOF10_SCL_PIN;
    NRF_TWIM0->PSEL.SDA   = DOF10_SDA_PIN;

    NRF_TWIM0->FREQUENCY  = DOF10_FREQ;

    NRF_TWIM0->SHORTS       = \
        TWIM_SHORTS_LASTTX_STOP_Msk | TWIM_SHORTS_LASTRX_STOP_Msk;

#if TWI_INT_ENABLED == 1

    NRF_TWIM0->INTENSET =
           (((uint32_t)0)   << TWIM_INTENSET_STOPPED_POS)
         | (((uint32_t)0)   << TWIM_INTENSET_ERROR_POS)
         | (((uint32_t)0)   << TWIM_INTENSET_SUSPENDED_POS)
         | (((uint32_t)0)   << TWIM_INTENSET_RXSTARTED_POS)
         | (((uint32_t)0)   << TWIM_INTENSET_TXSTARTED_POS)
         | (((uint32_t)0)   << TWIM_INTENSET_LASTRX_POS)
         | (((uint32_t)0)   << TWIM_INTENSET_LASTTX_POS);

    // set priority and enable interrupt in NVIC
    NVIC->IP[((uint32_t)SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0_IRQn)] = 
        (uint8_t)(
            (
                I2C_PRIORITY << (8 - __NVIC_PRIO_BITS)
            ) & (uint32_t)0xff
        );
    NVIC->ISER[((uint32_t)SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0_IRQn)>>5] = 
       ((uint32_t)1) << ( ((uint32_t)SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0_IRQn) & 0x1f);

#endif

    NRF_TWIM0->ENABLE = (TWIM_ENABLE_ENABLE_Enabled << TWIM_ENABLE_ENABLE_Pos);
}

void i2c_set_addr(uint8_t address) {

    NRF_TWIM0->ADDRESS    = address;
}


void i2c_read_bytes(uint8_t address, uint8_t* buffer, uint32_t length) {

    uint8_t tx_buffer[1];

    // ---- write first

    // clear events
    NRF_TWIM0->EVENTS_LASTTX    = 0;
    NRF_TWIM0->EVENTS_STOPPED   = 0;
    
    // set tx buffer
    NRF_TWIM0->TXD.PTR      = (uint32_t)(&tx_buffer[0]);
    NRF_TWIM0->TXD.MAXCNT   = 1;
    tx_buffer[0]            = address;

    // start to write
    NRF_TWIM0->TASKS_STARTTX  = 1;
    while( NRF_TWIM0->EVENTS_LASTTX==0);
    while( NRF_TWIM0->EVENTS_STOPPED==0);
    NRF_TWIM0->EVENTS_LASTTX  = 0;
    NRF_TWIM0->EVENTS_STOPPED = 0;

    // ---- reading data

    // clear events 
    NRF_TWIM0->EVENTS_LASTRX  = 0;
    NRF_TWIM0->EVENTS_STOPPED = 0;
    
    // set rx buffer
    NRF_TWIM0->RXD.PTR        = (uint32_t)(buffer);
    NRF_TWIM0->RXD.MAXCNT     = length;

    // start to read
    NRF_TWIM0->TASKS_STARTRX  = 1;
    while( NRF_TWIM0->EVENTS_LASTRX==0);
    while( NRF_TWIM0->EVENTS_STOPPED==0);
    NRF_TWIM0->EVENTS_LASTRX  = 0;
    NRF_TWIM0->EVENTS_STOPPED = 0;
}

void i2c_write_bytes(uint8_t address, uint8_t* buffer, uint32_t length) {
    
    uint8_t tx_buffer[1+length];

    // clear events
    NRF_TWIM0->EVENTS_LASTTX    = 0;
    NRF_TWIM0->EVENTS_STOPPED   = 0;
    
    // set tx buffer
    NRF_TWIM0->TXD.PTR      = (uint32_t)(&tx_buffer[0]);
    NRF_TWIM0->TXD.MAXCNT   = 1+length;
    tx_buffer[0]            = address;
    memcpy(&tx_buffer[1], buffer, length);

    // start to write
    NRF_TWIM0->TASKS_STARTTX = 1;
    while( NRF_TWIM0->EVENTS_LASTTX==0);
    while( NRF_TWIM0->EVENTS_STOPPED==0);
    NRF_TWIM0->EVENTS_LASTTX = 0;
    NRF_TWIM0->EVENTS_STOPPED = 0;
}

//=========================== private =========================================

void nrf_gpio_cfg_input(uint32_t pin_number) {

    NRF_GPIO_Type* NRF_Px_port;
    uint32_t       nrf_pin_number;

    if (pin_number < 32) {

        NRF_Px_port     = NRF_P0;
        nrf_pin_number  = pin_number;
    } else {

        NRF_Px_port = NRF_P1;
        nrf_pin_number  = pin_number & 0x1f;
    }
    
    NRF_Px_port->PIN_CNF[nrf_pin_number]  = \
            ((uint32_t)GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos)
        | ((uint32_t)GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos)
        | ((uint32_t)GPIO_PIN_CNF_PULL_Pullup << GPIO_PIN_CNF_PULL_Pos)
        | ((uint32_t)GPIO_PIN_CNF_DRIVE_S0D1 << GPIO_PIN_CNF_DRIVE_Pos)
        | ((uint32_t)GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);
}