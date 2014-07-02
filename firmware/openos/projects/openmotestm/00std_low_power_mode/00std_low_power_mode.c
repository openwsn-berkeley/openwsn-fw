/**
\brief This program shows the use of the "radio" bsp module.

Since the bsp modules for different platforms have the same declaration, you
can use this project with any platform.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012
*/


#include "board.h"
#include "radio.h"
#include "leds.h"
#include "bsp_timer.h"
#include "nvic.h"
#include "gpio.h"


//=========================== defines =========================================

#define LENGTH_PACKET   125+LENGTH_CRC ///< maximum length is 127 bytes
#define CHANNEL         26             ///< 2.480GHz
#define TIMER_PERIOD    65535          ///< 2s @ 32kHz
#define ID              0xab           ///< byte sent in the packets

// voltage: 3.8V
//#define RADIO_SLEEP //measured: 49uA in stop mode
//#define RADIO_TRXOFF //measured: 1.31mA in stop mode
//#define RADIO_PLL_ON //measured: 6.43mA in stop mode
//#define RADIO_RX_ON //measured: 13.84mA in stop mode
//#define RADIO_SLEEP_IN_RUN_MODE //measured: 23.7mA in run mode 
#define RADIO_BUSY_TX //calculated: 41mA-23.7mA+49uA = 17.35mA

//=========================== variables =======================================

enum {
   APP_FLAG_START_FRAME = 0x01,
   APP_FLAG_END_FRAME   = 0x02,
   APP_FLAG_TIMER       = 0x04,
};

typedef enum {
   APP_STATE_TX         = 0x01,
   APP_STATE_RX         = 0x02,
} app_state_t;

typedef struct {
   uint8_t num_radioTimerOverflows;
   uint8_t num_radioTimerCompare;
   uint8_t num_startFrame;
   uint8_t num_endFrame;
   uint8_t num_timer;
} app_dbg_t;

app_dbg_t app_dbg;

typedef struct {
   uint8_t     flags;
   app_state_t state;
   uint8_t     packet[LENGTH_PACKET];
   uint8_t     packet_len;
    int8_t     rxpk_rssi;
   uint8_t     rxpk_lqi;
   bool        rxpk_crc;
} app_vars_t;

app_vars_t app_vars;

//=========================== prototypes ======================================

uint16_t getRandomPeriod();
void     cb_radioTimerOverflows();
void     cb_radioTimerCompare();
void     cb_startFrame(uint16_t timestamp);
void     cb_endFrame(uint16_t timestamp);
void     cb_timer();

void EXTI_Configuration(void);
void board_stopmode();
//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int mote_main(void) {
   uint8_t i;
   
   // clear local variables
   memset(&app_vars,0,sizeof(app_vars_t));
   
   // initialize board
   board_init();
   GPIO_Configuration();
   EXTI_Configuration();
   
   GPIOD->BRR = (uint32_t)GPIO_Pin_2;
   
#ifdef RADIO_SLEEP
   PORT_PIN_RADIO_SLP_TR_CNTL_HIGH();
#ifdef RADIO_SLEEP_IN_RUN_MODE
   while(1);
#endif
   board_stopmode();
#endif
   
#ifdef RADIO_TRXOFF
   board_stopmode();
#endif
 
   // add callback functions radio
   radio_setOverflowCb(cb_radioTimerOverflows);
   radio_setCompareCb(cb_radioTimerCompare);
   radio_setStartFrameCb(cb_startFrame);
   radio_setEndFrameCb(cb_endFrame);
   
   // prepare packet
   app_vars.packet_len = sizeof(app_vars.packet);
   for (i=0;i<app_vars.packet_len;i++) {
      app_vars.packet[i] = ID;
   }

   // start bsp timer
   bsp_timer_set_callback(cb_timer);
   bsp_timer_scheduleIn(TIMER_PERIOD);
   
   // prepare radio
   radio_rfOn();
   radio_setFrequency(CHANNEL);
   
   // switch in RX by default
   radio_rxEnable();
   app_vars.state = APP_STATE_RX;
   
#ifdef RADIO_RX_ON
   leds_all_off();
   board_stopmode();
#endif
   
   // start by a transmit
   app_vars.flags |= APP_FLAG_TIMER;

   while (1) {
      // sleep while waiting for at least one of the flags to be set
      while (app_vars.flags==0x00) {
         board_stopmode();
      }
      // handle and clear every flag
      while (app_vars.flags) {

    	  if (app_vars.flags & APP_FLAG_START_FRAME) {
            // start of frame
            
            switch (app_vars.state) {
               case APP_STATE_RX:
                  // started receiving a packet
                  leds_error_on();
                  break;
               case APP_STATE_TX:
                  // started sending a packet
                  leds_sync_on();
                  break;
            }
            // clear flag
            app_vars.flags &= ~APP_FLAG_START_FRAME;
         }
         
         if (app_vars.flags & APP_FLAG_END_FRAME) {
            // end of frame
            
            switch (app_vars.state) {
               case APP_STATE_RX:
                  // done receiving a packet
            	   app_vars.packet_len = sizeof(app_vars.packet);
            	     for (i=0;i<app_vars.packet_len;i++) {
            	        app_vars.packet[i] = 0;
            	     }
                  // get packet from radio
                  radio_getReceivedFrame(app_vars.packet,
                                         &app_vars.packet_len,
                                         sizeof(app_vars.packet),
                                         &app_vars.rxpk_rssi,
                                         &app_vars.rxpk_lqi,
                                         &app_vars.rxpk_crc);
                  
                  leds_error_off();
                  break;
               case APP_STATE_TX:
                  // done sending a packet
                  
                  // switch to RX mode
                  radio_rxEnable();
                  app_vars.state = APP_STATE_RX;
                  
                  leds_sync_off();
                  break;
            }
            // clear flag
            app_vars.flags &= ~APP_FLAG_END_FRAME;
         }
         
         if (app_vars.flags & APP_FLAG_TIMER) {
            // timer fired
            
            if (app_vars.state==APP_STATE_RX) {
               // stop listening
               radio_rfOff();
               
               // prepare packet
               app_vars.packet_len = sizeof(app_vars.packet);
               for (i=0;i<app_vars.packet_len;i++) {
                  app_vars.packet[i] = ID;
               }
               
               // start transmitting packet
               radio_loadPacket(app_vars.packet,app_vars.packet_len);
               radio_txEnable();
               
#ifdef RADIO_PLL_ON
               leds_all_off();
               board_stopmode();
#endif
               
               radio_txNow();
#ifdef RADIO_BUSY_TX
               leds_all_off();
               while(1) { //keep sending
                    PORT_PIN_RADIO_SLP_TR_CNTL_HIGH();
                    PORT_PIN_RADIO_SLP_TR_CNTL_LOW();
               }
               board_stopmode();
#endif
               
               app_vars.state = APP_STATE_TX;
            }
            
            // clear flag
            app_vars.flags &= ~APP_FLAG_TIMER;
         }
      }
   }
}

//=========================== callbacks =======================================

void cb_radioTimerOverflows() {
   app_dbg.num_radioTimerOverflows++;
}

void cb_radioTimerCompare() {
   app_dbg.num_radioTimerCompare++;
}

void cb_startFrame(uint16_t timestamp) {
   // set flag
   app_vars.flags |= APP_FLAG_START_FRAME;
   // update debug stats
   app_dbg.num_startFrame++;
}

void cb_endFrame(uint16_t timestamp) {
   // set flag
   app_vars.flags |= APP_FLAG_END_FRAME;
   // update debug stats
   app_dbg.num_endFrame++;
}

void cb_timer() {
   // set flag
   app_vars.flags |= APP_FLAG_TIMER;
   // update debug stats
   app_dbg.num_timer++;
   // schedule again
   bsp_timer_scheduleIn(TIMER_PERIOD);
}


/**
  * @brief  Configures EXTI Line4.
  * @param  None
  * @retval : None
  */
void EXTI_Configuration(void)
{
  EXTI_InitTypeDef EXTI_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
  
  /* Configure PA.4 as input floating (EXTI Line4) */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  /* Connect EXTI Line4 to PC.4 */
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource4);

  /* Configure EXTI Line4 to generate an event or an interrupt on falling edge */
  EXTI_ClearITPendingBit(EXTI_Line4);
  EXTI_InitStructure.EXTI_Line = EXTI_Line4;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure); 
}

void board_stopmode() {
    DBGMCU_Config(DBGMCU_STOP, ENABLE);
    
    // Enable PWR and BKP clock
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
    // Desable the SRAM and FLITF clock in Stop mode
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_SRAM | RCC_AHBPeriph_FLITF, DISABLE);

    PWR_EnterSTOPMode(PWR_Regulator_ON,PWR_STOPEntry_WFI);
}