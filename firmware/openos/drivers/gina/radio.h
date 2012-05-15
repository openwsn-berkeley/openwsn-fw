#ifndef __RADIO_H
#define __RADIO_H

/**
\addtogroup drivers
\{
\addtogroup Radio
\{
*/

#include "openwsn.h"
#include "msp430x26x.h"
#include "atmel.h"

//=========================== define ==========================================

/**
\brief Possible values for the status of the radio.

After you get an interrupt from the radio, read the status register
(<tt>RG_IRQ_STATUS</tt>) to know what type it is, amoung the following.
*/
enum radio_irqstatus_enum {
   AT_IRQ_BAT_LOW                 = 0x80,   /**< Supply voltage below the programmed threshold. */
   AT_IRQ_TRX_UR                  = 0x40,   /**< Frame buffer access violation. */
   AT_IRQ_AMI                     = 0x20,   /**< Address matching. */
   AT_IRQ_CCA_ED_DONE             = 0x10,   /**< End of a CCA or ED measurement. */
   AT_IRQ_TRX_END                 = 0x08,   /**< Completion of a frame transmission/reception. */
   AT_IRQ_RX_START                = 0x04,   /**< Start of a PSDU reception. */
   AT_IRQ_PLL_UNLOCK              = 0x02,   /**< PLL unlock. */
   AT_IRQ_PLL_LOCK                = 0x01,   /**< PLL lock. */
};

/**
\brief Current state of the radio.

\note This radio driver is very minimal in that it does not follow a state machine.
      It is up to the MAC layer to ensure that the different radio operations 
      are called in the righr order. The radio keeps a state for debugging purposes only.
*/
enum radio_state_enum {
   RADIOSTATE_STOPPED             = 0x00,   /**< Completely stopped. */
   RADIOSTATE_RFOFF               = 0x01,   /**< Listening for commands by RF chain is off. */
   RADIOSTATE_SETTING_FREQUENCY   = 0x02,   /**< Configuring the frequency. */
   RADIOSTATE_FREQUENCY_SET       = 0x03,   /**< Done configuring the frequency. */
   RADIOSTATE_LOADING_PACKET      = 0x04,   /**< Loading packet to send over SPI. */
   RADIOSTATE_PACKET_LOADED       = 0x05,   /**< Packet is loaded in the TX buffer. */
   RADIOSTATE_ENABLING_TX         = 0x06,   /**< The RF Tx chaing is being enabled (includes locked the PLL). */
   RADIOSTATE_TX_ENABLED          = 0x07,   /**< Radio completely ready to transmit. */
   RADIOSTATE_TRANSMITTING        = 0x08,   /**< Busy transmitting bytes. */
   RADIOSTATE_ENABLING_RX         = 0x09,   /**< The RF Rx chaing is being enabled (includes locked the PLL). */
   RADIOSTATE_LISTENING           = 0x0a,   /**< RF chain is on, listening, but no packet received yet. */
   RADIOSTATE_RECEIVING           = 0x0b,   /**< Busy receiving bytes. */
   RADIOSTATE_TXRX_DONE           = 0x0c,   /**< Frame has been sent/received completely. */
   RADIOSTATE_TURNING_OFF         = 0x0d,   /**< Turning the RF chain off. */
};

/**
\brief Setting for which antenna to use

The following setting are the options which can be written
in the radio's <tt>RG_ANT_DIV</tt> register, which sets which of the 
two antennas to use.
*/
enum radio_antennaselection_enum {
   RADIO_UFL_ANTENNA              = 0x06,   /**< Use the antenna connected by U.FL. */
   RADIO_CHIP_ANTENNA             = 0x05,   /**< Use the on-board chip antenna. */
};

#define DELAYRXINT 8 // in 32kHz tickss -> 244us (measured 234 us)

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

// called from the MAC layer
void radio_init();
void radio_setFrequency(uint8_t frequency);
void radio_loadPacket(OpenQueueEntry_t* packet);
void radio_txEnable();
void radio_txNow();
void radio_rxEnable();
void radio_rxNow();
void radio_getReceivedFrame(OpenQueueEntry_t* writeToBuffer);
void radio_rfOff();

// interrupt handlers
void isr_radio();

/**
\}
\}
*/

#endif
