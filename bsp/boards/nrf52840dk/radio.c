/**
 * Author: Xavier Vilajosana (xvilajosana@eecs.berkeley.edu)
 *         Pere Tuset (peretuset@openmote.com)
 * Date:   July 2013
 * Description: CC2538-specific definition of the "radio" bsp module.
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "sdk/components/boards/pca10056.h"
#include "sdk/components/boards/boards.h"
#include "sdk/components/drivers_nrf/radio_config/radio_config.h"
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

#define WAIT_FOR_RADIO_DISABLE    (1)         ///< whether the driver shall wait until the radio is disabled upon calling radio_rfOff()
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
  bool event_ready;
  bool event_address;
  bool event_end;
} radio_vars_t;

static radio_vars_t radio_vars; // = {0};


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
  NRF_RADIO->TXPOWER   = (RADIO_TXPOWER_TXPOWER_0dBm << RADIO_TXPOWER_TXPOWER_Pos);

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
  // set fast radio ramp-up mode - shortens ramp-up to 40 µs from 140 µs while increasing required current, see http://infocenter.nordicsemi.com/topic/com.nordic.infocenter.nrf52840.ps/radio.html?cp=2_0_0_5_19_14_7#unique_1302685657
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
  // as a prerequisite for the radio, we start the high frequency clock now, if not yet running
  if (!radio_vars.hfc_started)
  {
    nrf_drv_clock_hfclk_request(NULL);
    while (!nrf_drv_clock_hfclk_is_running()) { };
  }

  // set up interrupts
  NRF_RADIO->INTENSET = RADIO_INTENSET_READY_Enabled << RADIO_INTENSET_READY_Pos | 
                        RADIO_INTENSET_ADDRESS_Enabled << RADIO_INTENSET_ADDRESS_Pos |
                        RADIO_INTENSET_END_Enabled << RADIO_INTENSET_END_Pos; 
    
	NVIC_SetPriority(RADIO_IRQn, 1);
	NVIC_ClearPendingIRQ(RADIO_IRQn);
	NVIC_EnableIRQ(RADIO_IRQn);    

   // RF can only be turned on in TX or RX mode, so the actual
   // code has been moved to txEnable and rxEnable, respectively
}


void radio_rfOff(void)
{
  // disable radio shortcut
  NRF_RADIO->SHORTS= 0;

  // disable radio interrupt
	NVIC_DisableIRQ(RADIO_IRQn);  

  // release the high frequency clock
  if (radio_vars.hfc_started)
  {
    nrf_drv_clock_hfclk_release();
  }

  if (radio_vars.transciever_state == TS_OFF) { return; }

#if (WAIT_FOR_RADIO_DISABLE == 1)
  NRF_RADIO->EVENTS_DISABLED = 0U;
#endif // WAIT_FOR_RADIO_DISABLE == 1

  // disable radio - should take max. 6 µs, see http://infocenter.nordicsemi.com/topic/com.nordic.infocenter.nrf52840.ps/radio.html?cp=2_0_0_5_19_14_7#unique_1302685657
  NRF_RADIO->TASKS_DISABLE = 1U;

#if (WAIT_FOR_RADIO_DISABLE == 1)
  while (NRF_RADIO->EVENTS_DISABLED == 0U) { }  
#endif // WAIT_FOR_RADIO_DISABLE == 1

  radio_vars.transciever_state = TS_OFF;
}


void radio_loadPacket(uint8_t* packet, uint16_t len)
{
#if 0  
  if ((uint32_t) radio_vars.payload % 4)
  {
    leds_debug_toggle();
  }
#endif  

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
  if (radio_vars.transciever_state == TS_TX) { return; }

  radio_rfOn();

#if (WAIT_FOR_RADIO_ENABLE == 1)
  NRF_RADIO->EVENTS_READY = 0U;
#endif // WAIT_FOR_RADIO_ENABLE == 1

  radio_vars.event_ready= false;
  NRF_RADIO->TASKS_TXEN = 1U;

#if (WAIT_FOR_RADIO_ENABLE == 1)
  while (!radio_vars.event_ready) { }
#endif // WAIT_FOR_RADIO_ENABLE == 1

  radio_vars.transciever_state = TS_TX;
}


void radio_txNow(void)
{
  if (radio_vars.transciever_state != TS_TX) { return; }

#if 0
  if (radio_vars.startFrame_cb)
  {
    radio_vars.startFrame_cb(sctimer_readCounter());
  }
#endif  

  radio_vars.event_end= false;
  NRF_RADIO->EVENTS_END  = 0U;
  NRF_RADIO->TASKS_START = 1U;

#if 0
  while (!radio_vars.event_end) { }
  radio_vars.endFrame_cb(sctimer_readCounter());
#endif  
}


void radio_rxEnable(void)
{
  if (radio_vars.transciever_state == TS_RX) { return; }

  radio_rfOn();

  // enable shortcut (as soon as a packet was received, the radio prepares itself for the next one)
  // NRF_RADIO->SHORTS |= RADIO_SHORTS_PHYEND_START_Enabled << RADIO_SHORTS_PHYEND_START_Pos;
  NRF_RADIO->SHORTS |= RADIO_SHORTS_END_START_Enabled << RADIO_SHORTS_END_START_Pos;

#if (WAIT_FOR_RADIO_ENABLE == 1)
  NRF_RADIO->EVENTS_READY = 0U;
#endif // WAIT_FOR_RADIO_ENABLE == 1

  radio_vars.event_ready= false;
  NRF_RADIO->TASKS_RXEN = 1U;

#if (WAIT_FOR_RADIO_ENABLE == 1)
  while (!radio_vars.event_ready) { }
#endif // WAIT_FOR_RADIO_ENABLE == 1

  radio_vars.transciever_state = TS_RX;
}


void radio_rxNow(void)
{
  if (radio_vars.transciever_state != TS_RX) { return; }

  // start listening
  radio_vars.event_end= false;
  NRF_RADIO->EVENTS_END = 0U;
  NRF_RADIO->TASKS_START = 1U;

  // wait for end of packet
  // while (!radio_vars.event_end) { }
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
  *pRssi= (int8_t) *pLqi;
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

kick_scheduler_t radio_isr(void)
{
  return DO_NOT_KICK_SCHEDULER;
}


void RADIO_IRQHandler(void)
{
  if (NRF_RADIO->EVENTS_READY && (NRF_RADIO->INTENSET & RADIO_INTENSET_READY_Msk))
  {
    NRF_RADIO->EVENTS_READY = 0;
    radio_vars.event_ready= true;
  }

  if (NRF_RADIO->EVENTS_ADDRESS && (NRF_RADIO->INTENSET & RADIO_INTENSET_ADDRESS_Msk))
  {
    NRF_RADIO->EVENTS_ADDRESS = 0;
    radio_vars.event_address= true;

    if (radio_vars.startFrame_cb)
    {
      radio_vars.startFrame_cb(sctimer_readCounter());
    }
  }

  if (NRF_RADIO->EVENTS_END && (NRF_RADIO->INTENSET & RADIO_INTENSET_END_Msk))
  {
    NRF_RADIO->EVENTS_END = 0;
    radio_vars.event_end= true;

    if (radio_vars.endFrame_cb)
    {
      radio_vars.endFrame_cb(sctimer_readCounter());
    }
  }
}
