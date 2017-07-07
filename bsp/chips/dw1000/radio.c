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
   uint16_t 				 rx_frameLen;
} radio_vars_t;

radio_vars_t radio_vars;
dwt_config_t UWBConfigs[6] = 
{
	{1,11,11,DWT_PRF_64M,1,DWT_BR_6M8,DWT_PLEN_128,DWT_PAC8,DWT_PHRMODE_STD,256},
	{2,11,11,DWT_PRF_64M,1,DWT_BR_6M8,DWT_PLEN_128,DWT_PAC8,DWT_PHRMODE_STD,256},
	{3,11,11,DWT_PRF_64M,1,DWT_BR_6M8,DWT_PLEN_128,DWT_PAC8,DWT_PHRMODE_STD,256},
	{4,17,17,DWT_PRF_64M,1,DWT_BR_6M8,DWT_PLEN_128,DWT_PAC8,DWT_PHRMODE_STD,256},
	{5,11,11,DWT_PRF_64M,1,DWT_BR_6M8,DWT_PLEN_128,DWT_PAC8,DWT_PHRMODE_STD,256},
	{7,17,17,DWT_PRF_64M,1,DWT_BR_6M8,DWT_PLEN_128,DWT_PAC8,DWT_PHRMODE_STD,256}
};


//=========================== prototypes ======================================
void error_handler(void);
void radio_txDoneCb(const dwt_cb_data_t* cb_data);
void radio_rxOkCb(const dwt_cb_data_t* cb_data);
void radio_rxErrCb(const dwt_cb_data_t* cb_data);
void radio_rxToCb(const dwt_cb_data_t* cb_data);

//=========================== public ==========================================

//===== admin

void radio_init() {

	GPIO_InitTypeDef GPIO_InitStructure;
	uint32_t devID = 0;
	
	// clear variables
	memset(&radio_vars,0,sizeof(radio_vars_t));

	// change state
	radio_vars.state          = RADIOSTATE_STOPPED;

	leds_all_on();
	deca_sleep(500);
	leds_all_off();
	deca_spi_init(0);
	GPIO_ResetBits(GPIOA, GPIO_Pin_0);
	deca_sleep(2);
    // The nRST pin is a floating input.
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
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
	dwt_configure(&UWBConfigs[0]);

	leds_circular_shift();
	dwt_setrxantennadelay(RX_ANT_DLY);
    dwt_settxantennadelay(TX_ANT_DLY);
    dwt_setleds(0x03);
	// set the DW1000 callbacks
	dwt_setcallbacks(radio_txDoneCb, radio_rxOkCb, radio_rxToCb, radio_rxErrCb);
	dwt_setinterrupt(DWT_INT_TFRS|
					 DWT_INT_RPHE|
					 DWT_INT_RFCG|
					 DWT_INT_RFCE|
					 DWT_INT_RFSL|
					 DWT_INT_RFTO|
					 DWT_INT_SFDT|
					 DWT_INT_RXPTO,
					 1);
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
	// Make sure the SPI is running at low speed
	dwt_softreset();
	leds_radio_off();
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
	if( (frequency >= 1) && (frequency<=5) ){
	   frequency = frequency -1;
	} else if( frequency == 7){
		frequency = frequency - 2;
	} else {
		frequency = 0;
	}
	
	dwt_configure(&UWBConfigs[frequency]);

	// change state
	radio_vars.state = RADIOSTATE_FREQUENCY_SET;
}

void radio_rfOn() {
   leds_radio_on();
}

void radio_rfOff() {
	// change state
	radio_vars.state = RADIOSTATE_TURNING_OFF;
	// Switch the radio to idle mode (RX and TX are OFF)
	dwt_forcetrxoff();
	dwt_rxreset();

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
   
	dwt_writetxdata(len, packet, 0);
	dwt_writetxfctrl(len, 0, 0);
   // change state
   radio_vars.state = RADIOSTATE_PACKET_LOADED;
}

void radio_txEnable() {
   // change state
   radio_vars.state = RADIOSTATE_ENABLING_TX;
   
   // wiggle debug pin
   debugpins_radio_set();
   leds_radio_on();
  
   // change state
   radio_vars.state = RADIOSTATE_TX_ENABLED;
}

void radio_txNow() {
	PORT_TIMER_WIDTH val;
	// change state
	radio_vars.state = RADIOSTATE_TRANSMITTING;

	dwt_starttx(DWT_START_TX_IMMEDIATE);
    if (radio_vars.startFrame_cb!=NULL) {
        // call the callback
		val=radiotimer_getCapturedTime();
		radio_vars.startFrame_cb(val);
	}
}

//===== RX

void radio_rxEnable() {
	// change state
	radio_vars.state = RADIOSTATE_ENABLING_RX;

	// put radio in reception mode
	dwt_rxenable(DWT_START_RX_IMMEDIATE);
	// wiggle debug pin
	debugpins_radio_set();
	leds_radio_on();
	// change state
	radio_vars.state = RADIOSTATE_LISTENING;
}

void radio_rxNow() {
   radio_vars.state = RADIOSTATE_RECEIVING;
}

void radio_getReceivedFrame(uint8_t* pBufRead,
                            uint8_t* pLenRead,
                            uint8_t  maxBufLen,
                             int8_t* pRssi,
                            uint8_t* pLqi,
                               bool* pCrc) {

	dwt_rxdiag_t rx_diag;
    
	// read the packet in
	if( (maxBufLen <= radio_vars.rx_frameLen) && (radio_vars.rx_frameLen != 0)){
		dwt_readrxdata(pBufRead,radio_vars.rx_frameLen,0);
	}
	if( (radio_vars.rx_frameLen != 0) && (radio_vars.state == RADIOSTATE_TXRX_DONE)){
		// get diagnostics data for the received frame
		dwt_readdiagnostics(&rx_diag);
		// In UWB there is no RSSI as such, use CIR growth instead
		*pRssi = (uint8_t)(rx_diag.maxGrowthCIR>>8);
		// LQI doesn't exist either use the first path amplitude instead
		*pLqi = (uint8_t)(rx_diag.firstPathAmp1>>8);
		*pCrc = 1;
	} else{
		*pRssi = 0;
		*pLqi = 0;
		*pCrc = 0;
	}
	*pLenRead = radio_vars.rx_frameLen;
}

//=========================== private =========================================
void error_handler(void){

	for(;;){
		leds_error_blink();
	}
}

//=========================== callbacks =======================================
void radio_txDoneCb(const dwt_cb_data_t* cb_data){
	PORT_TIMER_WIDTH capturedTime;

	// capture the time
	capturedTime = radiotimer_getCapturedTime();
	radio_vars.rx_frameLen = 0;
	radio_vars.state = RADIOSTATE_TXRX_DONE;
	if( radio_vars.endFrame_cb != NULL){
		radio_vars.endFrame_cb(capturedTime);
	}
}

void radio_rxOkCb(const dwt_cb_data_t* cb_data){
	PORT_TIMER_WIDTH capturedTime;

	// capture the time
	capturedTime = radiotimer_getCapturedTime();
	radio_vars.rx_frameLen = cb_data->datalength;
	radio_vars.state = RADIOSTATE_TXRX_DONE;
	if( radio_vars.endFrame_cb != NULL){
		radio_vars.endFrame_cb(capturedTime);
	}
}

void radio_rxErrCb(const dwt_cb_data_t* cb_data){
	PORT_TIMER_WIDTH capturedTime;

	// capture the time
	capturedTime = radiotimer_getCapturedTime();
	radio_vars.rx_frameLen = 0;
	radio_vars.state = RADIOSTATE_TXRX_DONE;
	if( radio_vars.endFrame_cb != NULL){
		radio_vars.endFrame_cb(capturedTime);
	}
	
}

void radio_rxToCb(const dwt_cb_data_t* cb_data){
	PORT_TIMER_WIDTH capturedTime;

	// capture the time
	capturedTime = radiotimer_getCapturedTime();
	radio_vars.rx_frameLen = 0;
	radio_vars.state = RADIOSTATE_TXRX_DONE;
	if( radio_vars.endFrame_cb != NULL){
		radio_vars.endFrame_cb(capturedTime);
	}
}

//=========================== interrupt handlers ==============================

kick_scheduler_t radio_isr() {
	dwt_isr();
	if(radio_vars.state == RADIOSTATE_TXRX_DONE){
		return KICK_SCHEDULER;
	}
	return DO_NOT_KICK_SCHEDULER;
}
