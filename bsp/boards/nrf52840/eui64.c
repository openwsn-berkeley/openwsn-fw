/**
 * brief nrf52840-specific definition of the "eui64" bsp module.
 *
 * Authors: Tamas Harczos (1, tamas.harczos@imms.de) and Adam Sedmak (2, adam.sedmak@gmail.com)
 * Company: (1) Institut fuer Mikroelektronik- und Mechatronik-Systeme gemeinnuetzige GmbH (IMMS GmbH)
 *          (2) Faculty of Electronics and Computing, Zagreb, Croatia
 * Date:   May 2018
*/

#include "sdk/modules/nrfx/mdk/nrf52840.h"

#include "eui64.h"

#include "string.h"
#include <stdbool.h>

//=========================== defines =========================================

//=========================== variables =======================================

static uint32_t m_deviceID[2]= {0};   ///< this will be filled with the HW-unique serial number
static bool m_deviceIDRead= false;    ///< will become true once the device ID has been read from the chip

//=========================== prototypes ======================================

//=========================== public ==========================================

void eui64_get(uint8_t* addressToWrite)
{
  if (!m_deviceIDRead)
  {
    // get ID from Nordic chip
    m_deviceID[0]= NRF_FICR->DEVICEID[0];
    m_deviceID[1]= NRF_FICR->DEVICEID[1];
    m_deviceIDRead= true;
  }

  if (addressToWrite)
  {
    memcpy(addressToWrite, m_deviceID, 2*sizeof(uint32_t));
  }
}

//=========================== private =========================================