/**
\brief at86rf215-specific definition of the "radio" bsp module.

\author Jonathan Munoz <jonathan.munoz@inria.fr>, July 2016.
*/

#include "board.h"
//#include "radio.h"
#include "radio_subghz.h"

#include "at86rf215.h"
#include "spi.h"
#include "debugpins.h"
#include "leds.h"
#include "sctimer.h"

#include <headers/hw_ioc.h>
#include <headers/hw_memmap.h>
#include <headers/hw_ssi.h>
#include <headers/hw_sys_ctrl.h>
#include <headers/hw_types.h>

#include <source/interrupt.h>
#include <source/ioc.h>
#include <source/gpio.h>
#include <source/sys_ctrl.h>

#define AT86RF215_IRQ_BASE      ( GPIO_D_BASE )
#define AT86RF215_IRQ_PIN       ( GPIO_PIN_0 )
#define AT86RF215_IRQ_IOC       ( IOC_OVERRIDE_DIS )
#define AT86RF215_IRQ_EDGE      ( GPIO_RISING_EDGE )


//=========================== defines =========================================

//=========================== variables =======================================

typedef struct {
  radio_capture_cbt           startFrame_cb;
  radio_capture_cbt           endFrame_cb;
  radio_state_t               state;
  uint8_t                     rf09_isr;
  uint8_t                     rf24_isr;
  uint8_t                     bb0_isr;
  uint8_t                     bb1_isr;    
} radiosubghz_vars_t;

radiosubghz_vars_t radiosubghz_vars;

//=========================== public ==========================================
static void radiosubghz_read_isr(void);
static void radiosubghz_clear_isr(void);

//isr handler for the radio
static void radiosubghz_isr(void);

//===== admin

void  radiosubghz_setFunctions(radio_functions_t * funcs){
    funcs->radio_change_modulation  = radiosubghz_change_modulation;
    funcs->radio_powerOn            = radiosubghz_powerOn;
    // RF admin
    funcs->radio_init               = radiosubghz_init;
    funcs->radio_setStartFrameCb    = radiosubghz_setStartFrameCb;
    funcs->radio_setEndFrameCb      = radiosubghz_setEndFrameCb;
    // RF admin
    funcs->radio_rfOn               = radiosubghz_rfOn;
    funcs->radio_rfOff              = radiosubghz_rfOff;
    funcs->radio_setFrequency       = radiosubghz_setFrequency;
    funcs->radio_change_modulation  = radiosubghz_change_modulation;
    funcs->radio_change_size        = radiosubghz_change_size;
    // reset
    funcs->radio_reset              = radiosubghz_reset;
    // TX
    funcs->radio_loadPacket_prepare = radiosubghz_loadPacket_prepare;
    funcs->radio_txEnable           = radiosubghz_txEnable;
    funcs->radio_txNow              = radiosubghz_txNow;
    funcs->radio_loadPacket         = radiosubghz_loadPacket;
    // RX
    funcs->radio_rxPacket_prepare   = radiosubghz_rxPacket_prepare;
    funcs->radio_rxEnable           = radiosubghz_rxEnable;
    funcs->radio_rxEnable_scum      = radiosubghz_rxEnable_scum;
    funcs->radio_rxNow              = radiosubghz_rxNow;
    funcs->radio_getReceivedFrame   = radiosubghz_getReceivedFrame;
    funcs->radio_getCRCLen          = radiosubghz_getCRCLen;
}

void radiosubghz_powerOn(void)
{
  volatile uint32_t delay;
  
  GPIOPinTypeGPIOOutput(GPIO_C_BASE, GPIO_PIN_0);
  GPIOPinTypeGPIOOutput(GPIO_D_BASE, GPIO_PIN_1);
  
  //set radio pwr off
  GPIOPinWrite(GPIO_C_BASE, GPIO_PIN_0, 0);
  GPIOPinWrite(GPIO_D_BASE, GPIO_PIN_1, 0);
  for(delay=0;delay<0xA2C2;delay++);
  
  //init the radio, pwr up the radio
  GPIOPinWrite(GPIO_C_BASE, GPIO_PIN_0, GPIO_PIN_0);
  for(delay=0;delay<0xA2C2;delay++);
  
  //reset the radio 
  GPIOPinWrite(GPIO_D_BASE, GPIO_PIN_1, GPIO_PIN_1);
 
}

//===== reset
void radiosubghz_reset(void) {
  at86rf215_spiWriteReg( RG_RF_RST, CMD_RF_RESET); 
}

void radiosubghz_init(void) {

  //power it on and configure pins
  radiosubghz_powerOn();

  // clear variables
  memset(&radiosubghz_vars,0,sizeof(radiosubghz_vars_t));
  
  // change state
  radiosubghz_vars.state          = RADIOSTATE_STOPPED;
   
   // reset radio
  radiosubghz_reset();
  
  at86rf215_spiStrobe(CMD_RF_TRXOFF);
  while(at86rf215_status() != RF_STATE_TRXOFF);
  
  // change state
  radiosubghz_vars.state          = RADIOSTATE_RFOFF;
  
   //configure external radio interrupt in pin D0
  GPIOPinIntDisable(AT86RF215_IRQ_BASE, AT86RF215_IRQ_PIN);
  
  /* The gpio is an input GPIO on rising edge */
  GPIOPinTypeGPIOInput(AT86RF215_IRQ_BASE, AT86RF215_IRQ_PIN);
  
  GPIOIntTypeSet(AT86RF215_IRQ_BASE, AT86RF215_IRQ_PIN, GPIO_RISING_EDGE);
  
  GPIOPinIntClear(AT86RF215_IRQ_BASE, AT86RF215_IRQ_PIN);
  /* Register the interrupt */
  GPIOPortIntRegister(AT86RF215_IRQ_BASE, radiosubghz_isr);
  
  /* Clear and enable the interrupt */
  GPIOPinIntEnable(AT86RF215_IRQ_BASE, AT86RF215_IRQ_PIN); 
    
  //check part number and version
  if ((at86rf215_spiReadReg(RG_RF_PN) != 0x34) | (at86rf215_spiReadReg(RG_RF_VN) != 0x03)) {
    while(1); //UNKNOWN DEVICE, FINISH
  }
  // Write registers to radio -- default configuration OFDM 400kbps
  for(uint16_t i = 0; i < (sizeof(basic_settings_ofdm_1_mcs2)/sizeof(registerSetting_t)); i++) {
    at86rf215_spiWriteReg( basic_settings_ofdm_1_mcs2[i].addr, basic_settings_ofdm_1_mcs2[i].data);
  };
  
  radiosubghz_read_isr();
}

void radiosubghz_change_size(uint16_t* size){
  static int i = 0;
  *size = sizes[i%4];
  i++;
}

void     radiosubghz_change_modulation(registerSetting_t * mod){
  static int mod_list = 1;
  
  at86rf215_spiStrobe(CMD_RF_TRXOFF);
  while(at86rf215_status() != RF_STATE_TRXOFF);
  
  for(uint16_t i = 0; i < (sizeof(*mod)/sizeof(registerSetting_t)); i++) {
    at86rf215_spiWriteReg( mod[i].addr, mod[i].data);
  };
  radiosubghz_read_isr();
  mod_list++;
}

void radiosubghz_setStartFrameCb(radio_capture_cbt cb) {
  radiosubghz_vars.startFrame_cb  = cb;
}

void radiosubghz_setEndFrameCb(radio_capture_cbt cb) {
  radiosubghz_vars.endFrame_cb    = cb;
}

//===== RF admin
//channel spacing in KHz
//frequency_0 in kHz
//frequency_nb integer
void radiosubghz_setFrequency(uint16_t channel_spacing, uint32_t frequency_0, uint16_t channel) {
  frequency_0 = (frequency_0/25);
  at86rf215_spiWriteReg(RG_RF09_CS, (uint8_t)(channel_spacing/25));
  at86rf215_spiWriteReg(RG_RF09_CCF0L, (uint8_t)(frequency_0%256));
  at86rf215_spiWriteReg(RG_RF09_CCF0H, (uint8_t)(frequency_0/256));
  at86rf215_spiWriteReg(RG_RF09_CNL, (uint8_t)(channel%256));
  at86rf215_spiWriteReg(RG_RF09_CNM, (uint8_t)(channel/256));
  // change state
  radiosubghz_vars.state = RADIOSTATE_FREQUENCY_SET;
}

void radiosubghz_rfOn(void) {
  //put the radio in the TRXPREP state
  at86rf215_spiStrobe(CMD_RF_TRXOFF);
  //while(radiosubghz_vars.state != RADIOSTATE_TX_ENABLED);
}

void radiosubghz_rfOff(void) {
  
  // change state
  radiosubghz_vars.state = RADIOSTATE_TURNING_OFF;
  
  at86rf215_spiStrobe(CMD_RF_TRXOFF);
  // wiggle debug pin
  debugpins_radio_clr();
  leds_radio_off();
  
  // change state
  radiosubghz_vars.state = RADIOSTATE_RFOFF;
}

//===== TX

void radiosubghz_loadPacket(uint8_t* packet, uint16_t len) {
  
  radiosubghz_vars.state = RADIOSTATE_LOADING_PACKET;
  at86rf215_spiWriteFifo(packet, len);
  // change state
  radiosubghz_vars.state = RADIOSTATE_PACKET_LOADED;
  //at86rf215_readBurst(0x0306, packet, len);
}

void radiosubghz_txEnable(void) {
  
  // change state
  radiosubghz_vars.state = RADIOSTATE_ENABLING_TX;
  at86rf215_spiStrobe(CMD_RF_TXPREP);
  // wiggle debug pin
  debugpins_radio_set();
  leds_radio_on();
  at86rf215_spiReadReg(0);
  while(radiosubghz_vars.state != RADIOSTATE_TX_ENABLED); 
  // change state
  
}

void radiosubghz_txNow(void) {
  // change state
  radiosubghz_vars.state = RADIOSTATE_TRANSMITTING;
  
  at86rf215_spiStrobe(CMD_RF_TX);
  while(radiosubghz_vars.state != RADIOSTATE_TXRX_DONE);
  at86rf215_spiStrobe(RF_TRXOFF);
}

//===== RX

void radiosubghz_rxEnable(void) {
  // change state
  radiosubghz_vars.state = RADIOSTATE_ENABLING_RX; 
  // wiggle debug pin
  debugpins_radio_set();
  leds_radio_on();
  at86rf215_spiStrobe(CMD_RF_RX);
  // change state
  radiosubghz_vars.state = RADIOSTATE_LISTENING;
}

void radiosubghz_rxNow(void) {
  //nothing to do
}

void radiosubghz_getReceivedFrame(
                                  uint8_t* bufRead,
                                  uint16_t* lenRead,
                                  uint16_t  maxBufLen,
                                  int8_t*  rssi,
                                  uint8_t* lqi,
                                  bool*    crc,
                                  uint8_t* mcs
                                    )
{
  // read the received packet from the RXFIFO
  at86rf215_spiReadRxFifo(bufRead, lenRead);
  *rssi   = at86rf215_spiReadReg(RG_RF09_EDV);
  *crc    = (at86rf215_spiReadReg(RG_BBC0_PC)>>5);
  *mcs    = (at86rf215_spiReadReg(RG_BBC0_OFDMPHRRX)&OFDMPHRRX_MCS_MASK);
}

//returns the crc len for this radio
uint8_t  radiosubghz_getCRCLen(void){
  return LENGTH_CRC_SUBGHZ;
}

//=========================== private ========================================= 

void radiosubghz_read_isr(){
  uint8_t flags[4];
  at86rf215_read_isr(flags);
  
  radiosubghz_vars.rf09_isr = flags[0];
  radiosubghz_vars.rf24_isr = flags[1];
  radiosubghz_vars.bb0_isr = flags[2];
  radiosubghz_vars.bb1_isr = flags[3];
}
//=========================== callbacks =======================================

//=========================== interrupt handlers ==============================
void radiosubghz_isr() {
  
  PORT_TIMER_WIDTH capturedTime;
 // kick_scheduler_t result = DO_NOT_KICK_SCHEDULER;
  
  GPIOPinIntClear(AT86RF215_IRQ_BASE, AT86RF215_IRQ_PIN);
  
  // capture the time
  capturedTime = sctimer_readCounter();
  //get isr that happened from radio
  radiosubghz_read_isr();
  
  if (radiosubghz_vars.rf09_isr & IRQS_TRXRDY_MASK){
    radiosubghz_vars.state = RADIOSTATE_TX_ENABLED;
   // result = DO_NOT_KICK_SCHEDULER;
  }
  
  if (radiosubghz_vars.bb0_isr & IRQS_RXFS_MASK){
    radiosubghz_vars.state = RADIOSTATE_RECEIVING;
    if (radiosubghz_vars.startFrame_cb!=NULL) {
      // call the callback
      radiosubghz_vars.startFrame_cb(capturedTime);
      // kick the OS
     // result = KICK_SCHEDULER;
    } else {
      //while(1);
    }
  } 
  else if ((radiosubghz_vars.bb0_isr & IRQS_TXFE_MASK)){ 
    radiosubghz_vars.state = RADIOSTATE_TXRX_DONE;
    if (radiosubghz_vars.endFrame_cb!=NULL) {
      // call the callback
      radiosubghz_vars.endFrame_cb(capturedTime);
      // kick the OS
     // result = KICK_SCHEDULER;
    } else {
      //while(1);
    }                
  }            
  else if ((radiosubghz_vars.bb0_isr & IRQS_RXFE_MASK)){ 
    radiosubghz_vars.state = RADIOSTATE_TXRX_DONE;
    if (radiosubghz_vars.endFrame_cb!=NULL) {
      // call the callback
      radiosubghz_vars.endFrame_cb(capturedTime);
      // kick the OS
      //result = KICK_SCHEDULER;   
    } else {
     // while(1);
    }                
  }
  radiosubghz_clear_isr();
}

port_INLINE void radiosubghz_clear_isr(){
  radiosubghz_vars.rf09_isr = 0;
  radiosubghz_vars.rf24_isr = 0;
  radiosubghz_vars.bb0_isr = 0;
  radiosubghz_vars.bb1_isr = 0;
}


void    radiosubghz_loadPacket_prepare(uint8_t* packet, uint8_t len){}
void    radiosubghz_rxPacket_prepare(void){}
void    radiosubghz_rxEnable_scum(void){}
