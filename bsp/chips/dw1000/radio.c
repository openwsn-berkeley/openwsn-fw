/**
\brief DW1000-specific definition of the "radio" bsp module.

\author Jean-Michel Rubillon <jmrubillon@theiet.org>
*/


#include "board.h"
#include "opendefs.h"
#include "radio.h"
#include "deca_device_api.h"
#include "deca_regs.h"
#include "deca_port.h"
#include "spi.h"
#include "sctimer.h"
#include "debugpins.h"
#include "leds.h"

//=========================== defines =========================================
#define TX_ANT_DLY 16436
#define RX_ANT_DLY 16436
#define TX_TO_RX_DELAY_UUS 60
//#define RX_RESP_TO_UUS 5000
#define RX_RESP_TO_UUS 0

//=========================== variables =======================================

typedef struct {
   radio_capture_cbt         startFrame_cb;
   radio_capture_cbt         endFrame_cb;
   radio_state_t             state; 
   uint16_t 				 rx_frameLen;
   uint32_t					 radio_status;
   kick_scheduler_t			 isr_retValue;
   uint32_t					 rxg_count;
   uint32_t					 txg_count;
   uint32_t					 txa_count;
   uint32_t		             rxErrTo_count;
} radio_vars_t;

radio_vars_t radio_vars;
dwt_config_t UWBConfigs[6] = 
{
	{1, DWT_PRF_64M, DWT_PLEN_1024, DWT_PAC32, 9, 9, 1, DWT_BR_110K,DWT_PHRMODE_STD, (1025+64-32)},
	{2, DWT_PRF_64M, DWT_PLEN_1024, DWT_PAC32, 9, 9, 1, DWT_BR_110K,DWT_PHRMODE_STD, (1025+64-32)},
	{3, DWT_PRF_64M, DWT_PLEN_1024, DWT_PAC32, 9, 9, 1, DWT_BR_110K,DWT_PHRMODE_STD, (1025+64-32)},
	{4, DWT_PRF_64M, DWT_PLEN_1024, DWT_PAC32, 17, 17, 1, DWT_BR_110K,DWT_PHRMODE_STD, (1025+64-32)},
	{5, DWT_PRF_64M, DWT_PLEN_1024, DWT_PAC32, 11, 11, 1, DWT_BR_110K,DWT_PHRMODE_STD, (1025+64-32)},
	{7, DWT_PRF_64M, DWT_PLEN_1024, DWT_PAC32, 17, 17, 1, DWT_BR_110K,DWT_PHRMODE_STD, (1025+64-32)},
};


//=========================== prototypes ======================================
void error_handler(void);
void radio_txDoneCb(const dwt_cb_data_t* cb_data);
void radio_rxOkCb(const dwt_cb_data_t* cb_data);
void radio_rxErrAndToCb(const dwt_cb_data_t* cb_data);
void radio_txStartOfFrameCb(const dwt_cb_data_t* cb_data);
void radio_rxStartOfFrameCb(const dwt_cb_data_t* cb_data);

//=========================== public ==========================================

//===== admin

void radio_init() {

	GPIO_InitTypeDef GPIO_InitStructure;
	uint32_t devID = 0;
	
	// clear variables
	memset(&radio_vars,0,sizeof(radio_vars_t));

	// change state
	radio_vars.state          = RADIOSTATE_STOPPED;
	radio_vars.isr_retValue   = DO_NOT_KICK_SCHEDULER;

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
	if(DWT_SUCCESS != dwt_initialise(DWT_LOADNONE))
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
	dwt_setcallbacks(radio_txDoneCb, radio_rxOkCb, radio_rxErrAndToCb, radio_rxErrAndToCb, radio_rxStartOfFrameCb, radio_txStartOfFrameCb);
	dwt_setinterrupt(DWT_INT_TFRS|     // Frame sent OK
					 DWT_INT_TXFRB|    // TX frame begins
					 DWT_INT_RXSFD|    // RX start of frame detected
					 DWT_INT_RFCG|     // Frame received OK
					 DWT_INT_RPHE|     // Rx phy header error
					 DWT_INT_RFCE|     // Rx CRC error
					 DWT_INT_RFSL|     // Rx RF sync lost
					 DWT_INT_RFTO|     // Rx timeout
					 DWT_INT_SFDT|     // Start of frame timeout
					 DWT_INT_RXPTO,    // Preamble detect timeout
				 1);
	dwt_setrxaftertxdelay(TX_TO_RX_DELAY_UUS);
	dwt_setrxtimeout(RX_RESP_TO_UUS);

	// change state
	radio_vars.state          = RADIOSTATE_RFOFF;
}


void radio_setStartFrameCb(radio_capture_cbt cb) {
   radio_vars.startFrame_cb  = cb;
}

void radio_setEndFrameCb(radio_capture_cbt cb) {
   radio_vars.endFrame_cb    = cb;
}

//===== reset

void radio_reset() {
	// Make sure the SPI is running at low speed
	dwt_softreset();
	//leds_radio_off();
}

//===== RF admin
void radio_setFrequency(uint8_t frequency) {
	// change state
	radio_vars.state = RADIOSTATE_SETTING_FREQUENCY;
	// ieee815e expects channels between 11 and 26 to be available.
	// in UWB this is not the case we're limited to only 6 channels
	// i.e. 1..5, 7 which the following block enforces
	if( frequency == 26){
		frequency = 5;
	}else{
		frequency = (frequency -11) % 5;
	}
	
	dwt_configure(&UWBConfigs[frequency]);

	// change state
	radio_vars.state = RADIOSTATE_FREQUENCY_SET;
}

void radio_rfOn() {
   //leds_radio_on();
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
   
	dwt_writetxdata(len-2, packet, 0);
	dwt_writetxfctrl(len-2, 0, 0);
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
	dwt_starttx(DWT_START_TX_IMMEDIATE|DWT_RESPONSE_EXPECTED);
    if (radio_vars.startFrame_cb!=NULL) {
        // call the callback
		val=sctimer_readCounter();
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
	radio_vars.isr_retValue = DO_NOT_KICK_SCHEDULER;
	radio_vars.state = RADIOSTATE_LISTENING;
}

void radio_rxNow() {
}

void radio_getReceivedFrame(uint8_t* pBufRead,
                            uint8_t* pLenRead,
                            uint8_t  maxBufLen,
                             int8_t* pRssi,
                            uint8_t* pLqi,
                               bool* pCrc) {

	dwt_rxdiag_t rx_diag;
    
	// read the packet in
	if( (radio_vars.rx_frameLen <= maxBufLen ) && (radio_vars.rx_frameLen != 0)){
		dwt_readrxdata(pBufRead,radio_vars.rx_frameLen,0);
	}
	// get diagnostics data for the received frame
	dwt_readdiagnostics(&rx_diag);
	// use max noise instead. There is a RSSI formula which we could apply too.
	*pRssi = (uint8_t)(rx_diag.maxNoise>>8);
	// LQI doesn't exist either use the first path amplitude instead
	*pLqi = (uint8_t)(rx_diag.firstPathAmp1>>8);
	*pCrc = (radio_vars.radio_status & (SYS_STATUS_RXDFR| SYS_STATUS_RXFCG))?1:0;
	*pLenRead = radio_vars.rx_frameLen;
}

//=========================== private =========================================
void error_handler(void){

	for(;;){
		leds_error_blink();
	}
}

//=========================== callbacks =======================================
void radio_txStartOfFrameCb(const dwt_cb_data_t* cb_data){
	PORT_TIMER_WIDTH capturedTime;

	// capture the time
	capturedTime = sctimer_readCounter();
	radio_vars.state = RADIOSTATE_TRANSMITTING;
	radio_vars.isr_retValue = KICK_SCHEDULER;
	radio_vars.txa_count += 1;
	radio_vars.radio_status = cb_data->status;
	if( radio_vars.startFrame_cb != NULL){
		radio_vars.startFrame_cb(capturedTime);
	}
	
}

void radio_txDoneCb(const dwt_cb_data_t* cb_data){
	PORT_TIMER_WIDTH capturedTime;

	// capture the time
	capturedTime = sctimer_readCounter();
	radio_vars.rx_frameLen = 0;
	radio_vars.state = RADIOSTATE_TXRX_DONE;
	radio_vars.isr_retValue = KICK_SCHEDULER;
	radio_vars.radio_status = cb_data->status;
	radio_vars.txg_count += 1;
	if( radio_vars.endFrame_cb != NULL){
		radio_vars.endFrame_cb(capturedTime);
	}
}

void radio_rxStartOfFrameCb(const dwt_cb_data_t* cb_data){
	PORT_TIMER_WIDTH capturedTime;

	// capture the time
	capturedTime = sctimer_readCounter();
	radio_vars.radio_status = cb_data->status;
	radio_vars.state = RADIOSTATE_RECEIVING;
	radio_vars.isr_retValue = KICK_SCHEDULER;
	radio_vars.rxg_count += 1;
	if( radio_vars.startFrame_cb != NULL){
		radio_vars.startFrame_cb(capturedTime);
	}
}

void radio_rxOkCb(const dwt_cb_data_t* cb_data){
	PORT_TIMER_WIDTH capturedTime;

	// capture the time
	capturedTime = sctimer_readCounter();
	// The DW1000 calculates the CRC automatically and does not pass it to the received frame.
	// To be compatible with the higher level layers we add the CRC length to the received frame,
	// knowning that it will be discarded without being read.
	// Should the higher layers ever start reading the CRC then we will need to calculate it here.
	radio_vars.rx_frameLen = cb_data->datalength +2;
	radio_vars.radio_status = cb_data->status;
	radio_vars.state = RADIOSTATE_TXRX_DONE;
	radio_vars.isr_retValue = KICK_SCHEDULER;
	radio_vars.rxg_count += 1;
	if( radio_vars.endFrame_cb != NULL){
		radio_vars.endFrame_cb(capturedTime);
	}
}

// error and timeout handler
void radio_rxErrAndToCb(const dwt_cb_data_t* cb_data){
	radio_vars.rxErrTo_count += 1;
	radio_rxEnable();
}
//=========================== interrupt handlers ==============================

kick_scheduler_t radio_isr() {
	dwt_isr();
	return 	radio_vars.isr_retValue;
}
