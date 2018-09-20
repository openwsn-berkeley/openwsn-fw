/**
 * brief Nordic nRF52840-specific definition of the "radio" bsp module. 
 *
 * Authors: Tamas Harczos (1, tamas.harczos@imms.de) and Adam Sedmak (2, adam.sedmak@gmail.com)
 * Company: (1) Institut fuer Mikroelektronik- und Mechatronik-Systeme gemeinnuetzige GmbH (IMMS GmbH)
 *          (2) Faculty of Electronics and Computing, Zagreb, Croatia
 *    Date: June 2018
*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "sdk/components/boards/boards.h"
#include "sdk/components/drivers_nrf/radio_config/radio_config.h"
#include "sdk/modules/nrfx/drivers/include/nrfx_systick.h"
#include "sdk/modules/nrfx/mdk/nrf52840.h"

#include "sdk/integration/nrfx/legacy/nrf_drv_clock.h"

#include "nrf_delay.h"

#include "app_config.h"
#include "leds.h"
#include "radio.h"
#include "board.h"
#include "board_info.h"
#include "debugpins.h"
#include "sctimer.h"


//=========================== defines =========================================

/* For calculating frequency */
#define FREQUENCY_OFFSET  10
#define FREQUENCY_STEP    5

#define SFD_OCTET                 (0xA7)      ///< start of frame delimiter of IEEE 802.15.4
#define MAX_PACKET_SIZE           (127)       ///< maximal size of radio packet (one more byte at the beginning needed to store the length)
#define CRC_POLYNOMIAL            (0x11021)   ///< polynomial used for CRC calculation in 802.15.4 frames (x^16 + x^12 + x^5 + 1)

#define WAIT_FOR_RADIO_DISABLE    (0)         ///< whether the driver shall wait until the radio is disabled upon calling radio_rfOff()
#define WAIT_FOR_RADIO_ENABLE     (1)         ///< whether the driver shall wait until the radio is enabled upon calling radio_txEnable() or radio_rxEnable()


//=========================== variables =======================================

typedef struct
{
  radio_capture_cbt  startFrame_cb;
  radio_capture_cbt  endFrame_cb;
  radio_state_t      state; 
  uint8_t payload[1+MAX_PACKET_SIZE] __attribute__ ((aligned));
  enum Transciever_state { TS_OFF, TS_TX, TS_RX } transciever_state;
  bool hfc_started;
//  volatile bool event_ready;
} radio_vars_t;

static radio_vars_t radio_vars;


//=========================== prototypes ======================================

static uint32_t swap_bits(uint32_t inp);
static uint32_t bytewise_bitswap(uint32_t inp);

//=========================== public ==========================================


void radio_init(void)
{
#if 0
  // start 16 MHz crystal oscillator
  NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
  NRF_CLOCK->TASKS_HFCLKSTART    = 1;
  while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0) { }
#endif

  // clear internal variables
  memset(&radio_vars, 0, sizeof(radio_vars));

  // set radio configuration parameters
  NRF_RADIO->TXPOWER   = (RADIO_TXPOWER_TXPOWER_Pos4dBm << RADIO_TXPOWER_TXPOWER_Pos);

  uint8_t const frequency= 13;                                          ///< let's start on channel 13
  NRF_RADIO->FREQUENCY = FREQUENCY_STEP*(frequency-FREQUENCY_OFFSET);   ///< IEEE 802.15.4 frequency channel, see http://infocenter.nordicsemi.com/topic/com.nordic.infocenter.nrf52840.ps/radio.html?cp=2_0_0_5_19_11_1#ieee802154_freq

  // set radio mode to IEEE 802.15.4
  NRF_RADIO->MODE      = (RADIO_MODE_MODE_Ieee802154_250Kbit << RADIO_MODE_MODE_Pos);

  // - - - - - - The followings may be unneeded - - - - - ->
  // radio address config
  NRF_RADIO->PREFIX0 =
      ((uint32_t)swap_bits(0xC3) << 24) // Prefix byte of address 3 converted to nRF24L series format
    | ((uint32_t)swap_bits(0xC2) << 16) // Prefix byte of address 2 converted to nRF24L series format
    | ((uint32_t)swap_bits(0xC1) << 8)  // Prefix byte of address 1 converted to nRF24L series format
    | ((uint32_t)swap_bits(0xC0) << 0); // Prefix byte of address 0 converted to nRF24L series format

  NRF_RADIO->PREFIX1 =
      ((uint32_t)swap_bits(0xC7) << 24) // Prefix byte of address 7 converted to nRF24L series format
    | ((uint32_t)swap_bits(0xC6) << 16) // Prefix byte of address 6 converted to nRF24L series format
    | ((uint32_t)swap_bits(0xC4) << 0); // Prefix byte of address 4 converted to nRF24L series format

  NRF_RADIO->BASE0 = bytewise_bitswap(0x01234567UL);  // Base address for prefix 0 converted to nRF24L series format
  NRF_RADIO->BASE1 = bytewise_bitswap(0x89ABCDEFUL);  // Base address for prefix 1-7 converted to nRF24L series format

  NRF_RADIO->TXADDRESS   = 0x00UL;  // Set device address 0 to use when transmitting
  NRF_RADIO->RXADDRESSES = 0x01UL;  // Enable device address 0 to use to select which addresses to receive
  // <- - - - - - - - - - until here - - - - - - - - - - - -

  // set config field length to 8
  NRF_RADIO->PCNF0 &= (~RADIO_PCNF0_LFLEN_Msk);
  NRF_RADIO->PCNF0 |= (8UL << RADIO_PCNF0_LFLEN_Pos);

  // set 32-bit zero preamble
  NRF_RADIO->PCNF0 &= (~RADIO_PCNF0_PLEN_Msk);
  NRF_RADIO->PCNF0 |= ((uint32_t) RADIO_PCNF0_PLEN_32bitZero << RADIO_PCNF0_PLEN_Pos);

#if 0
  // [MADE THE RADIO UNUSABLE IN FIRST TESTS] set fast radio ramp-up mode - shortens ramp-up to 40 about us from about 140 us while increasing required current, see http://infocenter.nordicsemi.com/topic/com.nordic.infocenter.nrf52840.ps/radio.html?cp=2_0_0_5_19_14_7#unique_1302685657
  NRF_RADIO->PCNF0 &= (~RADIO_MODECNF0_RU_Msk);
  NRF_RADIO->PCNF0 |= ((uint32_t) RADIO_MODECNF0_RU_Fast << RADIO_MODECNF0_RU_Pos);
#endif  

  // set max packet size
  NRF_RADIO->PCNF1 &= (~RADIO_PCNF1_MAXLEN_Msk);
  NRF_RADIO->PCNF1 |= ((uint32_t) MAX_PACKET_SIZE << RADIO_PCNF1_MAXLEN_Pos);

  // set start of frame delimiter
  NRF_RADIO->SFD= (SFD_OCTET << RADIO_SFD_SFD_Pos) & RADIO_SFD_SFD_Msk;

  // set CRC to be included
  NRF_RADIO->PCNF0 &= (~RADIO_PCNF0_CRCINC_Msk);
  NRF_RADIO->PCNF0 |= ((uint32_t) RADIO_PCNF0_CRCINC_Include << RADIO_PCNF0_CRCINC_Pos);

  // set CRC length
  NRF_RADIO->CRCCNF &= (~RADIO_CRCCNF_LEN_Msk);
  NRF_RADIO->CRCCNF |= ((uint32_t) LENGTH_CRC << RADIO_CRCCNF_LEN_Pos);

  // configure CRC (CRC calculation as per 802.15.4 standard. Starting at first byte after length field.)
  NRF_RADIO->CRCCNF &= (~RADIO_CRCCNF_SKIPADDR_Msk);
  NRF_RADIO->CRCCNF |= ((uint32_t) RADIO_CRCCNF_SKIPADDR_Ieee802154 << RADIO_CRCCNF_SKIPADDR_Pos);

  // set CRC polynomial
  NRF_RADIO->CRCPOLY = (CRC_POLYNOMIAL << RADIO_CRCPOLY_CRCPOLY_Pos);
  NRF_RADIO->CRCINIT = 0x0UL;

  // set payload pointer
  NRF_RADIO->PACKETPTR = (uint32_t) &radio_vars.payload[0];

  // set up interrupts
  // disable radio interrupt
  NVIC_DisableIRQ(RADIO_IRQn);
  NRF_RADIO->INTENSET = // RADIO_INTENSET_READY_Enabled << RADIO_INTENSET_READY_Pos | 
                        RADIO_INTENSET_ADDRESS_Enabled << RADIO_INTENSET_ADDRESS_Pos |
                        RADIO_INTENSET_END_Enabled << RADIO_INTENSET_END_Pos; 
  NVIC_SetPriority(RADIO_IRQn, NRFX_RADIO_CONFIG_IRQ_PRIORITY);

#if 0
  nrf_drv_clock_hfclk_release();
  radio_vars.hfc_started= false;
#endif
}


void radio_setStartFrameCb(radio_capture_cbt cb)
{
   radio_vars.startFrame_cb  = cb;
}


void radio_setEndFrameCb(radio_capture_cbt cb)
{
   radio_vars.endFrame_cb = cb;
}


void radio_reset(void)
{
  radio_rfOff();
  radio_init();
}


void radio_setFrequency(uint8_t frequency)
{
  NRF_RADIO->FREQUENCY = FREQUENCY_STEP*(frequency-FREQUENCY_OFFSET);  ///< IEEE 802.15.4 frequency channel, see http://infocenter.nordicsemi.com/topic/com.nordic.infocenter.nrf52840.ps/radio.html?cp=2_0_0_5_19_11_1#ieee802154_freq
}


void radio_rfOn(void)
{
 /**
  *  \var hfclk_request_timeout_us
  *
  *  Tests have been made to measure the time the hfclk (HFXO) needs to start up at different temperatures.
  *  It has been shown that there is little variation related to temperature; between +24 and -28 degrees
  *  Celsius the difference was only 10 us in max. and 5 us in median (calculated from 100000 samples), so
  *  that starting the HFXO in cold surrounding will take max. 10 us longer, which is already included in
  *  the number (380 us) below.
  */  
  #define hfclk_request_timeout_us 380


  nrfx_systick_state_t systick_time;


NRFX_CRITICAL_SECTION_ENTER();

  // as a prerequisite for the radio, we start the high frequency clock now, if not yet running, this can take about 350 us on the Dongle
  if (!radio_vars.hfc_started)
  {
    nrfx_systick_get(&systick_time);
    nrf_drv_clock_hfclk_request(NULL);

    while ((!nrf_drv_clock_hfclk_is_running()) && (!nrfx_systick_test(&systick_time, hfclk_request_timeout_us)))
    {
      // nrfx_systick_get(&systick_time2);
      // lastDelay= ((systick_time.time-systick_time2.time) & 0x00FFFFFF)/(SystemCoreClock/1000000);
    }

    if (!nrf_drv_clock_hfclk_is_running())
    {
      // this seems to happen a few times an hour, though the HFCLK *IS* running
      // leds_error_blink();
    }

    radio_vars.hfc_started= true;
  }

NRFX_CRITICAL_SECTION_EXIT();


  NVIC_ClearPendingIRQ(RADIO_IRQn);
  NVIC_EnableIRQ(RADIO_IRQn);

   // RF can only be turned on in TX or RX mode, so the actual
   // code has been moved to txEnable and rxEnable, respectively
}


void radio_rfOff(void)
{
  if (radio_vars.transciever_state == TS_OFF) { return; }


NRFX_CRITICAL_SECTION_ENTER();

  // disable radio shortcut
  NRF_RADIO->SHORTS= 0;

  // disable radio interrupt
  NVIC_DisableIRQ(RADIO_IRQn);

  // release the high frequency clock
  if (radio_vars.hfc_started)
  {
    nrf_drv_clock_hfclk_release();
    radio_vars.hfc_started= false;
  }

  radio_vars.transciever_state = TS_OFF;

NRFX_CRITICAL_SECTION_EXIT();


#if (WAIT_FOR_RADIO_DISABLE == 1)
  NRF_RADIO->EVENTS_DISABLED = 0U;
#endif // WAIT_FOR_RADIO_DISABLE == 1

  // disable radio - should take max. 6 µs, see http://infocenter.nordicsemi.com/topic/com.nordic.infocenter.nrf52840.ps/radio.html?cp=2_0_0_5_19_14_7#unique_1302685657
  NRF_RADIO->TASKS_DISABLE = 1U;

#if (WAIT_FOR_RADIO_DISABLE == 1)
  while (NRF_RADIO->EVENTS_DISABLED == 0U) { }  
#endif // WAIT_FOR_RADIO_DISABLE == 1

  leds_radio_off();
  debugpins_radio_clr();

}


void radio_loadPacket(uint8_t* packet, uint16_t len)
{
  if ((len > 0) && (len <= MAX_PACKET_SIZE))      ///< note: 1st byte should be the payload size (for Nordic), and the two last bytes are used by the MAC layer for CRC
  {
    radio_vars.payload[0]= len;
    memcpy(&radio_vars.payload[1], packet, len);
  }

  // (re)set payload pointer
  NRF_RADIO->PACKETPTR = (uint32_t) &radio_vars.payload[0];
}


void radio_txEnable(void)
{
  // M1-START
  if (radio_vars.transciever_state == TS_TX) { return; }

  radio_rfOn();

  radio_vars.transciever_state = TS_TX;
  // M1-END
}


void radio_txNow(void)
{
  // M2-START
  if (radio_vars.transciever_state != TS_TX) { return; }

  leds_radio_on();

  // shortcut: start transmitting when ready
  NRF_RADIO->SHORTS |= RADIO_SHORTS_READY_START_Enabled << RADIO_SHORTS_READY_START_Pos;

  NRF_RADIO->EVENTS_END = 0U;
  NRF_RADIO->TASKS_TXEN = 1U;

  debugpins_radio_set();
}


void radio_rxEnable(void)
{
  if (radio_vars.transciever_state == TS_RX) { return; }

  radio_rfOn();

  radio_vars.transciever_state = TS_RX;
}


void radio_rxNow(void)
{
  if (radio_vars.transciever_state != TS_RX) { return; }

  leds_radio_on();

  // enable shortcut (as soon as a packet was received, the radio prepares itself for the next one)
  // NRF_RADIO->SHORTS |= RADIO_SHORTS_PHYEND_START_Enabled << RADIO_SHORTS_PHYEND_START_Pos;
  NRF_RADIO->SHORTS |= RADIO_SHORTS_END_START_Enabled << RADIO_SHORTS_END_START_Pos;

  // shortcut: start listening when ready
  NRF_RADIO->SHORTS |= RADIO_SHORTS_READY_START_Enabled << RADIO_SHORTS_READY_START_Pos;

  NRF_RADIO->EVENTS_END = 0U;
  NRF_RADIO->TASKS_RXEN = 1U;

  debugpins_radio_set();
}


void radio_getReceivedFrame(uint8_t* pBufRead,
                            uint8_t* pLenRead,
                            uint8_t  maxBufLen,
                             int8_t* pRssi,
                            uint8_t* pLqi,
                               bool* pCrc)
{
  // check for length parameter; if too long, payload won't fit into memory
  uint8_t len= radio_vars.payload[0];
  if (len == 0) { return; }
  if (len > MAX_PACKET_SIZE) { len= MAX_PACKET_SIZE; }
  if (len > maxBufLen) { len= maxBufLen; }

  // copy payload
  memcpy(pBufRead, &radio_vars.payload[1], len);

  // store other parameters
  *pLenRead= len;
  *pLqi= radio_vars.payload[radio_vars.payload[0]-1];

  // For the RSSI calculation, see 
  //
  // - http://infocenter.nordicsemi.com/topic/com.nordic.infocenter.nrf52840.ps/radio.html?cp=2_0_0_5_19_11_6#ieee802154_rx and
  // - https://www.metageek.com/training/resources/understanding-rssi.html
  //
  // Our RSSI will be in the range -91 dB (worst) to 0 dB (best)
  *pRssi= (*pLqi > 91)?(0):(((int8_t) *pLqi) - 91);

  *pCrc= (NRF_RADIO->CRCSTATUS == 1U);
}


//=========================== private =========================================

static uint32_t swap_bits(uint32_t inp)
{
  uint32_t i;
  uint32_t retval = 0;

  inp = (inp & 0x000000FFUL);

  for (i = 0; i < 8; i++)
  {
    retval |= ((inp >> i) & 0x01) << (7 - i);
  }

  return retval;
}


static uint32_t bytewise_bitswap(uint32_t inp)
{
  return (swap_bits(inp >> 24) << 24)
        | (swap_bits(inp >> 16) << 16)
        | (swap_bits(inp >> 8) << 8)
        | (swap_bits(inp));
}


//=========================== callbacks =======================================

//=========================== interrupt handlers ==============================

void RADIO_IRQHandler(void)
{
  if (NRF_RADIO->EVENTS_ADDRESS)
  {
    NRF_RADIO->EVENTS_ADDRESS = 0;

    // M2-END

    if (radio_vars.startFrame_cb)
    {
      radio_vars.startFrame_cb(sctimer_readCounter());
    }
  }

  if (NRF_RADIO->EVENTS_END)
  {
    NRF_RADIO->EVENTS_END = 0;

    if (radio_vars.endFrame_cb)
    {
      radio_vars.endFrame_cb(sctimer_readCounter());
    }
  }
}
