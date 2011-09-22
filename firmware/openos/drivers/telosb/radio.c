#include "gina_config.h"
#include "radio.h"
#include "packetfunctions.h"
#include "IEEE802154E.h"
#include "ieee154etimer.h"
#include "spi.h"
#include "openserial.h"
#include "leds.h"

//=========================== variables =======================================

/**
Internal variables of the Radio module.
*/
typedef struct {
   /**
   The current state of the radio. Possible values are listed in
   radio_state_enum. Note that the radio driver does not enforce any state
   machine, so this state is kept up to date by the driver only for inspection
   during debug.
   */
   uint8_t state;
} radio_vars_t;

radio_vars_t radio_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

/**
\brief Initializes this module.

\pre Call this function once before any other function in this module.
*/
void radio_init() {
   // change state
   radio_vars.state = RADIOSTATE_STOPPED;
   
   // set the radio debug pin as output
   DEBUG_PIN_RADIO_INIT();
   
   // initialize communication between MSP430 and radio
   //-- 4-wire SPI
   spi_init();
   //-- RF_SLP_TR_CNTL (P4.7) pin (output)
   P4OUT  &= ~0x80;                              // set low
   P4DIR  |=  0x80;                              // configure as output
   //-- IRQ_RF (P1.6) pin (input)
   P1OUT  &= ~0x40;                              // set low
   P1DIR  &= ~0x40;                              // input direction
   P1IES  &= ~0x40;                              // interrup when transition is low-to-high
   P1IE   |=  0x40;                              // enable interrupt
   
   // configure the radio
   spi_write_register(RG_TRX_STATE, CMD_FORCE_TRX_OFF);  // turn radio off
   spi_write_register(RG_IRQ_MASK, 0x0C);                // tell radio to fire interrupt on TRX_END and RX_START
   spi_read_register(RG_IRQ_STATUS);                     // deassert the interrupt pin (P1.6) in case is high
   spi_write_register(RG_ANT_DIV, RADIO_CHIP_ANTENNA);// use chip antenna
#define RG_TRX_CTRL_1 0x04
   spi_write_register(RG_TRX_CTRL_1, 0x20);              // have the radio calculate CRC   

   //busy wait until radio status is TRX_OFF
   while((spi_read_register(RG_TRX_STATUS) & 0x1F) != TRX_OFF);
   
   // change state
   radio_vars.state = RADIOSTATE_RFOFF;
}

//======= sending a packet

/**
\brief Set the radio frequency.

Writes the frequency register in the radio over SPI.

\pre The RF chain of the radio must be off for this
     to have any effect.

\param [in] frequency The frequency to set the radio at: an
                      integer between 11 (for 2.405GHz) and
                      26 (for 2.480GHz).
*/
void radio_setFrequency(uint8_t frequency) {
   // change state
   radio_vars.state = RADIOSTATE_SETTING_FREQUENCY;
   
   // make sure the frequency asked for is within bounds
   if (frequency < 11 || frequency > 26){
      frequency = 26;
   }
   
   // configure the radio to the right frequecy
   spi_write_register(RG_PHY_CC_CCA,0x20+frequency);
   
   // change state
   radio_vars.state = RADIOSTATE_FREQUENCY_SET;
}

/**
\brief Load a packet in the radio's TX buffer.

\pre The RF chain of the radio must be off when calling this function.

\param [in] packet The packet to write into the buffer.
*/
void radio_loadPacket(OpenQueueEntry_t* packet) {
   // change state
   radio_vars.state = RADIOSTATE_LOADING_PACKET;
   
   // don't declare radio as owner or else MAC will not be able to retransmit
   
   // add 1B length at the beginning (PHY header)
   packetfunctions_reserveHeaderSize(packet,1);
   packet->payload[0] = packet->length-1;   // length (not counting length field)
   
   // add 1B SPI address at the beginning (internally for SPI)
   packetfunctions_reserveHeaderSize(packet,1);
   packet->payload[0] = 0x60;
   
   // load packet in TXFIFO
   spi_write_buffer(packet);
   
   // remove the 2 header bytes just added so MAC layer doesn't get confused
   // when retransmitting
   packetfunctions_tossHeader(packet,2);
   
   // change state
   radio_vars.state = RADIOSTATE_PACKET_LOADED;
}

/**
\brief Enable the radio in TX mode.

Turns everything on in the radio so it can transmit

\pre Call #radio_loadPacket before calling this function.

\post Everything is ready in the radio to transmit the packet, but only
      when #radio_txNow is called in the packet actually sent. #radio_txEnable
      and #radio_txNow are always supposed to be called in sequence.
*/
void radio_txEnable() {
   // change state
   radio_vars.state = RADIOSTATE_ENABLING_TX;
   
   // wiggle debug pin
   DEBUG_PIN_RADIO_SET();
   LED_RADIO_ON();
   
   // turn on radio's PLL
   spi_write_register(RG_TRX_STATE, CMD_PLL_ON);
   while((spi_read_register(RG_TRX_STATUS) & 0x1F) != PLL_ON); // busy wait until done
   
   // change state
   radio_vars.state = RADIOSTATE_TX_ENABLED;
}

/**
\brief Start transmitting.

Tells the radio to transmit the packet immediately.

\pre Call #radio_txEnable before calling this function.

\note After this function is called, it takes the radio
      about 220us to transmit the SFD. This is the time it takes to transmit
      a 4B phyiscal preamble (120us) and some 100us of overhead. This value
      might change if you configure the radio to sent more/less preamble bytes.
*/
void radio_txNow() {
   // change state
   radio_vars.state = RADIOSTATE_TRANSMITTING;
   
   // send packet by pulsing the RF_SLP_TR_CNTL pin
   P4OUT |=  0x80;
   P4OUT &= ~0x80;
   
   // The AT86RF231 does not generate an interrupt when the radio transmits the
   // SFD, which messes up the MAC state machine. The danger is that, if we leave
   // this funtion like this, any radio watchdog timer will expire.
   // Instead, we cheat an mimick a start of frame event by calling
   // ieee154e_startOfFrame from here. This also means that software can never catch
   // a radio glitch by which #radio_txEnable would not be followed by a packet being
   // transmitted (I've never seen that).
   ieee154e_startOfFrame(ieee154etimer_getCapturedTime());
}

//======= receiving a packet

/**
\brief Enable the radio in RX mode.

Turns everything on in the radio so it can receive.

\pre The radio should not be busy transmitting when calling this function.

\note This radio also actually starts receiving. From MAC's point
of view, it needs to call #radio_rxNow to start receiving, but this
radio does not allow this functionality.
*/
void radio_rxEnable() {
   // change state
   radio_vars.state = RADIOSTATE_ENABLING_RX;
   
   // put radio in reception mode
   spi_write_register(RG_TRX_STATE, CMD_RX_ON);
   
   // wiggle debug pin
   DEBUG_PIN_RADIO_SET();
   LED_RADIO_ON();
   
   //busy wait until radio status is PLL_ON
   while((spi_read_register(RG_TRX_STATUS) & 0x1F) != RX_ON);
   
   // change state
   radio_vars.state = RADIOSTATE_LISTENING;
}

/**
\brief Start receiving.

\pre Call #radio_rxEnable before calling this function.

\note This is a dummy function which does not do anything since
this radio is already listening after #radio_rxEnable is called.
This function is however "implemented" to be compliant with the MAC.
*/
void radio_rxNow() {
}

/**
\brief Retrieves the received frame from the radio's RX buffer.

Copies the bytes of the packet in the buffer provided as an argument.
If, for some reason, the length byte of the received packet indicates
a length >127 (which is impossible), only the first two bytes will
be received. And the reception will be aborted.

This function also populates the #OpenQueueEntry_t->l1_crc and
#OpenQueueEntry_t->l1_rssi fields.

\pre You must have received a packet before calling this function.

\param [out] writeToBuffer The buffer to which to write the bytes.
*/
void radio_getReceivedFrame(OpenQueueEntry_t* writeToBuffer) {
   uint8_t temp_reg_value;
   
   // initialize the buffer
   writeToBuffer->payload = &(writeToBuffer->packet[0]);
   
   // read whether CRC was is correct
   temp_reg_value = spi_read_register(RG_PHY_RSSI);
   writeToBuffer->l1_crc  = (temp_reg_value & 0x80)>>7;  // msb is whether packet passed CRC
   
   // read the RSSI
   // according to section 8.4.3 of the AT86RF231, the RSSI is calculate as:
   // -91 + ED [dBm]
   temp_reg_value = spi_read_register(RG_PHY_ED_LEVEL);
   writeToBuffer->l1_rssi = -91 + temp_reg_value;
   
   // copy packet from rx buffer in radio over SPI
   spi_read_buffer(writeToBuffer,2); // first read only 2 bytes to receive the length
   writeToBuffer->length = writeToBuffer->payload[1];
   if (writeToBuffer->length>2 && writeToBuffer->length<=127) {
      // retrieve the whole packet (including 1B SPI address, 1B length, the packet, 1B LQI)
      spi_read_buffer(writeToBuffer,1+1+writeToBuffer->length+1);
      // shift start by 2B (1B answer received when MSP sent SPI address + 1B length).
      writeToBuffer->payload += 2;
      // read 1B "footer" (LQI) and store that information
      writeToBuffer->l1_lqi = writeToBuffer->payload[writeToBuffer->length];
      // toss CRC (2 last bytes)
      packetfunctions_tossFooter(writeToBuffer, 2);
   }
}

//======= Turning radio off

/**
\brief Turn the radio off.

This does not turn the radio entirely off, i.e. it is still listening for
commands over SPI. It does, however, make sure the power hungry components
such as the PLL are switched off.
*/
void radio_rfOff() {
   // change state
   radio_vars.state = RADIOSTATE_TURNING_OFF;
   
   // turn radio off
   spi_write_register(RG_TRX_STATE, CMD_FORCE_TRX_OFF);
   while((spi_read_register(RG_TRX_STATUS) & 0x1F) != TRX_OFF); // busy wait until done
   
   // wiggle debug pin
   DEBUG_PIN_RADIO_CLR();
   LED_RADIO_OFF();
   
   // change state
   radio_vars.state = RADIOSTATE_RFOFF;
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================

/**
\brief Radio interrupt handler function.

Called by the scheduler when the radio pulses the <tt>IRQ_RF</tt> pin.
This function checks what caused the interrupt by looking at the contents
of the <tt>RG_IRQ_STATUS</tt> register of the radio. Reading this register
also lowers the <tt>IRQ_RF</tt> pin.
*/
void isr_radio() {
   uint16_t capturedTime;
   uint8_t irq_status;
   // capture the time
   capturedTime = ieee154etimer_getCapturedTime();
   // reading IRQ_STATUS causes IRQ_RF (P1.6) to go low
   irq_status = spi_read_register(RG_IRQ_STATUS);
   switch (irq_status) {
      case AT_IRQ_RX_START:
         // change state
         radio_vars.state = RADIOSTATE_RECEIVING;
         // call MAC layer
         ieee154e_startOfFrame(capturedTime);
         break;
      case AT_IRQ_TRX_END:
         // change state
         radio_vars.state = RADIOSTATE_TXRX_DONE;
         // call MAC layer
         ieee154e_endOfFrame(capturedTime);
         break;
      default:
         openserial_printError(COMPONENT_PACKETFUNCTIONS,ERR_WRONG_IRQ_STATUS,
                               (errorparameter_t)irq_status,
                               (errorparameter_t)0);
         break;
   }
}