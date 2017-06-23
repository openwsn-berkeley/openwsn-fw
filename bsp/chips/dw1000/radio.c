/**
\brief DW1000-specific definition of the "radio" bsp module.

\author Jean-Michel Rubillon <jmrubillon@theiet.org>
*/


#include "board.h"
#include "opendefs.h"
#include "radio.h"
#include "deca_device_api.h"
#include "deca_regs.h"
#include "spi.h"
#include "radiotimer.h"
#include "debugpins.h"
#include "leds.h"

//=========================== defines =========================================
#define TX_ANT_DLY 16436
#define RX_ANT_DLY 16436

//=========================== variables =======================================

typedef struct {
   radiotimer_capture_cbt    startFrame_cb;
   radiotimer_capture_cbt    endFrame_cb;
   radio_state_t             state; 
} radio_vars_t;

radio_vars_t radio_vars;

//=========================== prototypes ======================================
void error_handler(void);
//=========================== public ==========================================

//===== admin

void radio_init() {

	// clear variables
	memset(&radio_vars,0,sizeof(radio_vars_t));

	// change state
	radio_vars.state          = RADIOSTATE_STOPPED;

   	dwt_config_t UWBConfig;
	uint32_t devID = 0;
	uint8_t frame_seq_nb = 0;
	uint32_t status_reg;
/* Hold copy of frame length of frame received (if good) so that it can be examined at a debug breakpoint. */
	uint16 frame_len = 0;
	int i = 0;

	leds_all_on();
	deca_sleep(500);
	leds_all_off();
	// Make sure the SPI is running at low speed
	deca_spi_init(0);
	GPIO_ResetBits(GPIOA, GPIO_Pin_0);
	deca_sleep(2);
	GPIO_SetBits(GPIOA, GPIO_Pin_0);
	deca_sleep(2);
	if(DWT_SUCCESS != dwt_initialise(DWT_LOADUCODE))
	{
		error_handler();
	}

	
	// Switch to high speed SPI mode
	deca_spi_init(1);

	//Verify that the switch happened without issues
	devID = dwt_readdevid();
	if(devID != DWT_DEVICE_ID)
	{
		// The switch to higher SPI speed failed
		error_handler();
	}


	// Configure the DW1000 to default settings
	UWBConfig.chan = 2;
	UWBConfig.txCode = 9;
	UWBConfig.rxCode = 9;
	UWBConfig.prf = DWT_PRF_64M;
	UWBConfig.nsSFD = 1; // Use non-standard SFD
	UWBConfig.dataRate = DWT_BR_110K;
	UWBConfig.txPreambLength = DWT_PLEN_1024;
	UWBConfig.rxPAC = DWT_PAC32;
	UWBConfig.phrMode = DWT_PHRMODE_STD;
	UWBConfig.sfdTO = 1057;
	dwt_configure(&UWBConfig);
	leds_circular_shift();
	dwt_setrxantennadelay(RX_ANT_DLY);
    dwt_settxantennadelay(TX_ANT_DLY);
    dwt_setleds(0x03);

	// change state
	radio_vars.state          = RADIOSTATE_RFOFF;
}

void radio_setOverflowCb(radiotimer_compare_cbt cb) {
   radiotimer_setOverflowCb(cb);
}

void radio_setCompareCb(radiotimer_compare_cbt cb) {
   radiotimer_setCompareCb(cb);
}

void radio_setStartFrameCb(radiotimer_capture_cbt cb) {
   radio_vars.startFrame_cb  = cb;
}

void radio_setEndFrameCb(radiotimer_capture_cbt cb) {
   radio_vars.endFrame_cb    = cb;
}

//===== reset

void radio_reset() {
   PORT_PIN_RADIO_RESET_LOW();
}

//===== timer

void radio_startTimer(PORT_TIMER_WIDTH period) {
   radiotimer_start(period);
}

PORT_TIMER_WIDTH radio_getTimerValue() {
   return radiotimer_getValue();
}

void radio_setTimerPeriod(PORT_TIMER_WIDTH period) {
   radiotimer_setPeriod(period);
}

PORT_TIMER_WIDTH radio_getTimerPeriod() {
   return radiotimer_getPeriod();
}

//===== RF admin

void radio_setFrequency(uint8_t frequency) {
   // change state
   radio_vars.state = RADIOSTATE_SETTING_FREQUENCY;
   
   // configure the radio to the right frequecy
//   radio_spiWriteReg(RG_PHY_CC_CCA,0x20+frequency);
   
   // change state
   radio_vars.state = RADIOSTATE_FREQUENCY_SET;
}

void radio_rfOn() {
   PORT_PIN_RADIO_RESET_LOW();
   PORT_PIN_RADIO_RESET_HIGH();
}

void radio_rfOff() {
   // change state
   radio_vars.state = RADIOSTATE_TURNING_OFF;
//   radio_spiReadReg(RG_TRX_STATUS);
   // turn radio off
//   radio_spiWriteReg(RG_TRX_STATE, CMD_FORCE_TRX_OFF);
   //radio_spiWriteReg(RG_TRX_STATE, CMD_TRX_OFF);
//   while((radio_spiReadReg(RG_TRX_STATUS) & 0x1F) != TRX_OFF); // busy wait until done
   
   // wiggle debug pin
   debugpins_radio_clr();
   leds_radio_off();
   
   // change state
   radio_vars.state = RADIOSTATE_RFOFF;
}

//===== TX

void radio_loadPacket(uint8_t* packet, uint16_t len) {
   // change state
   radio_vars.state = RADIOSTATE_LOADING_PACKET;
   
   // load packet in TXFIFO
//   radio_spiWriteTxFifo(packet,len);
   
   // change state
   radio_vars.state = RADIOSTATE_PACKET_LOADED;
}

void radio_txEnable() {
   // change state
   radio_vars.state = RADIOSTATE_ENABLING_TX;
   
   // wiggle debug pin
   debugpins_radio_set();
   leds_radio_on();
   
   // turn on radio's PLL
//   radio_spiWriteReg(RG_TRX_STATE, CMD_PLL_ON);
//   while((radio_spiReadReg(RG_TRX_STATUS) & 0x1F) != PLL_ON); // busy wait until done
   
   // change state
   radio_vars.state = RADIOSTATE_TX_ENABLED;
}

void radio_txNow() {
   PORT_TIMER_WIDTH val;
   // change state
   radio_vars.state = RADIOSTATE_TRANSMITTING;
   
   // send packet by pulsing the SLP_TR_CNTL pin
//   PORT_PIN_RADIO_SLP_TR_CNTL_HIGH();
 //  PORT_PIN_RADIO_SLP_TR_CNTL_LOW();
   
   // The AT86RF231 does not generate an interrupt when the radio transmits the
   // SFD, which messes up the MAC state machine. The danger is that, if we leave
   // this funtion like this, any radio watchdog timer will expire.
   // Instead, we cheat an mimick a start of frame event by calling
   // ieee154e_startOfFrame from here. This also means that software can never catch
   // a radio glitch by which #radio_txEnable would not be followed by a packet being
   // transmitted (I've never seen that).
//   if (radio_vars.startFrame_cb!=NULL) {
      // call the callback
//      val=radiotimer_getCapturedTime();
 //     radio_vars.startFrame_cb(val);
//   }
}

//===== RX

void radio_rxEnable() {
   // change state
   radio_vars.state = RADIOSTATE_ENABLING_RX;
   
   // put radio in reception mode
//   radio_spiWriteReg(RG_TRX_STATE, CMD_RX_ON);
   
   // wiggle debug pin
   debugpins_radio_set();
   leds_radio_on();
   
   // busy wait until radio really listening
//   while((radio_spiReadReg(RG_TRX_STATUS) & 0x1F) != RX_ON);
   
   // change state
   radio_vars.state = RADIOSTATE_LISTENING;
}

void radio_rxNow() {
   // nothing to do
}

void radio_getReceivedFrame(uint8_t* pBufRead,
                            uint8_t* pLenRead,
                            uint8_t  maxBufLen,
                             int8_t* pRssi,
                            uint8_t* pLqi,
                               bool* pCrc) {
   uint8_t temp_reg_value;
   
   //===== crc
//   temp_reg_value  = radio_spiReadReg(RG_PHY_RSSI);
//   *pCrc           = (temp_reg_value & 0x80)>>7;  // msb is whether packet passed CRC
   
   //===== rssi
   // as per section 8.4.3 of the AT86RF231, the RSSI is calculate as:
   // -91 + ED [dBm]
//   temp_reg_value  = radio_spiReadReg(RG_PHY_ED_LEVEL);
//   *pRssi          = -91 + temp_reg_value;
   
   //===== packet
//   radio_spiReadRxFifo(pBufRead,
//                       pLenRead,
//                       maxBufLen,
//                       pLqi);
}

//=========================== private =========================================
void error_handler(void){
uint16_t i;

	for(;;){
		leds_error_blink();
	}
}

//=========================== callbacks =======================================

//=========================== interrupt handlers ==============================

kick_scheduler_t radio_isr() {
   PORT_TIMER_WIDTH capturedTime;
   uint8_t  irq_status;

   // capture the time
   capturedTime = radiotimer_getCapturedTime();

   // reading IRQ_STATUS causes radio's IRQ pin to go low
//   irq_status = radio_spiReadReg(RG_IRQ_STATUS);
    
   // start of frame event
//   if (irq_status & AT_IRQ_RX_START) {
      // change state
//      radio_vars.state = RADIOSTATE_RECEIVING;
//      if (radio_vars.startFrame_cb!=NULL) {
         // call the callback
//         radio_vars.startFrame_cb(capturedTime);
         // kick the OS
//         return KICK_SCHEDULER;
//      } else {
//         while(1);
//      }
//   }
   // end of frame event
//   if (irq_status & AT_IRQ_TRX_END) {
      // change state
//      radio_vars.state = RADIOSTATE_TXRX_DONE;
//      if (radio_vars.endFrame_cb!=NULL) {
         // call the callback
//         radio_vars.endFrame_cb(capturedTime);
         // kick the OS
//         return KICK_SCHEDULER;
//      } else {
//         while(1);
//      }
//   }
   
   return DO_NOT_KICK_SCHEDULER;
}
