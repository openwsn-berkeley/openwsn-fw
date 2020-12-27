/**
\brief nRF52840-specific definition of the "temperature" bsp module.

\author Tengfei Chang <tc@monolet.com>, July 2020.
*/

#include "nrf52833.h"
#include "temp.h"

//=========================== defines =========================================

#define MASK_SIGN           (0x00000200UL)
#define MASK_SIGN_EXTENSION (0xFFFFFC00UL)

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

int32_t read_temperature(void) {
    
    int32_t temp;
    
    // Start the temperature measurement
    NRF_TEMP->TASKS_START = 1;

    // wait until a temperature measurement is ready
    while (NRF_TEMP->EVENTS_DATARDY == 0);
    
    // clear the state
    NRF_TEMP->EVENTS_DATARDY = 0;

    // read temperature
    if ((NRF_TEMP->TEMP & MASK_SIGN) != 0) {
        temp = NRF_TEMP->TEMP | MASK_SIGN_EXTENSION;
    } else {
        temp = NRF_TEMP->TEMP;
    }
    
    // Stop the temperature measurement
    NRF_TEMP->TASKS_STOP = 1;
    
    return temp;
}

//=========================== private =========================================
