/**
\brief This program shows the use of the "radio" bsp module using action timer 
that scum provide.

With this project, SLOT_FSM_IMPLEMENTATION_MULTIPLE_TIMER_INTERRUPT need to be 
defined in board_info.h, or this project doesn't compile.

This project is to verify the StartOfFrame interrupt is not triggered when 
schedule Tx_SEND command with a compare timer.

\author Tengfei Chang <tengfei.chang@inria.fr>, August 2018.
*/

#include "board.h"
#include "radio.h"
#include "leds.h"
#include "sctimer.h"
#include "debugpins.h"
#include "memory_map.h"

//=========================== defines =========================================

#define LENGTH_PACKET   125+LENGTH_CRC ///< maximum length is 127 bytes
#define CHANNEL         11             ///< 11=2.405GHz
#define TIMER_PERIOD    0xffff         ///< 0xffff = 2s@32kHz
#define ID              0x99           ///< byte sent in the packets

// FSM timer durations (combinations of atomic durations)
// TX
#define DURATION_tt1 131-PORT_delayTx-PORT_maxTxDataPrepare
#define DURATION_tt2 131-PORT_delayTx
#define DURATION_tt3 131-PORT_delayTx+33
#define DURATION_tt4 164
#define DURATION_tt5 151-16-delayRx-PORT_maxRxAckPrepare
#define DURATION_tt6 151-16-delayRx
#define DURATION_tt7 151+16
#define DURATION_tt8 PORT_maxTxAckPrepare
// RX
#define DURATION_rt1 131-36-PORT_maxRxDataPrepare
#define DURATION_rt2 131-36
#define DURATION_rt3 131+36
#define DURATION_rt4 164
#define DURATION_rt5 151-PORT_delayTx-PORT_maxTxAckPrepare
#define DURATION_rt6 151-PORT_delayTx
#define DURATION_rt7 151-PORT_delayTx+33
#define DURATION_rt8 PORT_maxTxAckPrepare

//=========================== variables =======================================

enum {
   APP_FLAG_START_FRAME = 0x01,
   APP_FLAG_END_FRAME   = 0x02,
   APP_FLAG_TIMER       = 0x04,
};

typedef enum {
   APP_STATE_TX         = 0x01,
   APP_STATE_RX         = 0x02,
   APP_STATE_READY      = 0x04,
   APP_STATE_DELAY      = 0x08,
} app_state_t;

typedef struct {
   uint8_t              num_startFrame;
   uint8_t              num_endFrame;
   uint8_t              num_timer;
} app_dbg_t;

app_dbg_t app_dbg;

typedef struct {
   uint8_t              flags;
   app_state_t          state;
   uint8_t              packet[LENGTH_PACKET];
   uint8_t              packet_len;
    int8_t              rxpk_rssi;
   uint8_t              rxpk_lqi;
   bool                 rxpk_crc;
  uint32_t              referenceTime;
} app_vars_t;

app_vars_t app_vars;

//=========================== prototypes ======================================

void     cb_startFrame(PORT_TIMER_WIDTH timestamp);
void     cb_endFrame(PORT_TIMER_WIDTH timestamp);
void     cb_timer(void);
void     cb_action_timer(void);

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
 
   // add callback functions radio
   radio_setStartFrameCb(cb_startFrame);
   radio_setEndFrameCb(cb_endFrame);
   
   // prepare packet
   app_vars.packet_len = sizeof(app_vars.packet);
   for (i=0;i<app_vars.packet_len;i++) {
      app_vars.packet[i] = ID;
   }
   
   // start bsp timer
   sctimer_set_callback(cb_timer);
   sctimer_set_actionCallback(cb_action_timer);
   sctimer_setCompare(sctimer_readCounter()+TIMER_PERIOD);
   sctimer_enable();
   
   // prepare radio
   radio_rfOn();
   radio_setFrequency(CHANNEL);
   
   // switch in RX by default
   radio_rxEnable_scum();
   app_vars.state = APP_STATE_RX;
   
   // start by a transmit
   app_vars.flags |= APP_FLAG_TIMER;
   
   while (1) {
      
      // sleep while waiting for at least one of the flags to be set
      while (app_vars.flags==0x00) {
         board_sleep();
      }
      
      // handle and clear every flag
      while (app_vars.flags) {
         
         
         //==== APP_FLAG_START_FRAME (TX or RX)
         
         if (app_vars.flags & APP_FLAG_START_FRAME) {
            // start of frame
            
            switch (app_vars.state) {
               case APP_STATE_RX:
                  // started receiving a packet
                  
                  // led
                  leds_error_on();
                  break;
               case APP_STATE_TX:
                  // started sending a packet
                  
                  // led
                  leds_sync_on();
                  break;
            }
            
            // clear flag
            app_vars.flags &= ~APP_FLAG_START_FRAME;
         }
         
         
         //==== APP_FLAG_END_FRAME (TX or RX)
         
         if (app_vars.flags & APP_FLAG_END_FRAME) {
            // end of frame
            
            switch (app_vars.state) {
               
               case APP_STATE_RX:
                  
                  // done receiving a packet
                  app_vars.packet_len = sizeof(app_vars.packet);
                  
                  // get packet from radio
                  radio_getReceivedFrame(
                     app_vars.packet,
                     &app_vars.packet_len,
                     sizeof(app_vars.packet),
                     &app_vars.rxpk_rssi,
                     &app_vars.rxpk_lqi,
                     &app_vars.rxpk_crc
                  );
                  
                  // led
                  leds_error_off();
                  break;
               case APP_STATE_TX:
                  // done sending a packet
                  
                  // switch to RX mode
                  radio_rxEnable();
                  app_vars.state = APP_STATE_RX;
                  
                  // led
                  leds_sync_off();
                  break;
            }
            // clear flag
            app_vars.flags &= ~APP_FLAG_END_FRAME;
         }
         
         
         //==== APP_FLAG_TIMER
         
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
               radio_loadPacket_prepare(app_vars.packet,app_vars.packet_len);
               // 1. schedule timer for loading packet
               sctimer_scheduleActionIn(
                    ACTION_LOAD_PACKET,
                    app_vars.referenceTime+DURATION_tt1
               );
               // 2. schedule timer for sending packet
               sctimer_scheduleActionIn(
                    ACTION_SEND_PACKET,
                    app_vars.referenceTime+DURATION_tt2
               );
               
               sctimer_setCapture(ACTION_TX_SFD_DONE);
               sctimer_setCapture(ACTION_TX_SEND_DONE);
               
               app_vars.state = APP_STATE_READY;
            }
            
            // clear flag
            app_vars.flags &= ~APP_FLAG_TIMER;
         }
      }
   }
}

//=========================== callbacks =======================================

void cb_startFrame(PORT_TIMER_WIDTH timestamp) {
   // set flag
   app_vars.flags |= APP_FLAG_START_FRAME;
   
   // update debug stats
   app_dbg.num_startFrame++;
    
   UART_REG__TX_DATA = 'S';
   UART_REG__TX_DATA = 'o';
   UART_REG__TX_DATA = 'F';
    
}

void cb_endFrame(PORT_TIMER_WIDTH timestamp) {
   // set flag
   app_vars.flags |= APP_FLAG_END_FRAME;
   
   // update debug stats
   app_dbg.num_endFrame++;
    
   UART_REG__TX_DATA = 'E';
   UART_REG__TX_DATA = 'o';
   UART_REG__TX_DATA = 'F';
}

void cb_timer(void) {
    
   UART_REG__TX_DATA = 'T';
   UART_REG__TX_DATA = 'i';
   UART_REG__TX_DATA = 'm';
   UART_REG__TX_DATA = 'e';
   UART_REG__TX_DATA = 'r';
    
    debugpins_fsm_clr();
    
    // set flag
    app_vars.flags |= APP_FLAG_TIMER;

    // update debug stats
    app_dbg.num_timer++;

    app_vars.referenceTime = sctimer_readCounter();

    sctimer_setCompare(app_vars.referenceTime+TIMER_PERIOD);
}

void     cb_action_timer(void){
    
   debugpins_fsm_toggle();
    
   UART_REG__TX_DATA = 'A';
   UART_REG__TX_DATA = 'C';
   UART_REG__TX_DATA = 'T';
   UART_REG__TX_DATA = 'i';
   UART_REG__TX_DATA = 'o';
   UART_REG__TX_DATA = 'n';
    
    if (app_vars.state & APP_STATE_READY){
        UART_REG__TX_DATA = '0'+APP_STATE_READY;
        app_vars.state = APP_STATE_DELAY;
        return;
    }
    
    if (app_vars.state & APP_STATE_DELAY){
        UART_REG__TX_DATA = '0'+APP_STATE_DELAY;
        app_vars.state = APP_STATE_TX;
        return;
    }
}
