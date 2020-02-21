/**
\brief This program shows the use of the "radio" bsp module.

Since the bsp modules for different platforms have the same declaration, you
can use this project with any platform.

After loading this program, your board will switch on its radio on frequency
CHANNEL.

While receiving a packet (i.e. from the start of frame event to the end of 
frame event), it will turn on its sync LED.

Every TIMER_PERIOD, it will also send a packet containing LENGTH_PACKET bytes
set to ID. While sending a packet (i.e. from the start of frame event to the
end of frame event), it will turn on its error LED.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, August 2014.
*/

#include "board.h"
#include "radio.h"
#include "leds.h"
#include "sctimer.h"

//=========================== defines =========================================

#define LENGTH_PACKET   125+LENGTH_CRC ///< maximum length is 127 bytes
#define CHANNEL         11             ///< 11=2.405GHz
#define TIMER_PERIOD    0xfff         ///< 0xffff = 2s@32kHz
#define ID              0x12           ///< byte sent in the packets

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
    bool                txFlag;
} app_vars_t;

app_vars_t app_vars;

//=========================== prototypes ======================================

void     cb_startFrame(PORT_TIMER_WIDTH timestamp);
void     cb_endFrame(PORT_TIMER_WIDTH timestamp);
void     cb_timer(void);

//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int mote_main(void) {
   uint8_t i;
    uint32_t j;
   
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
   sctimer_setCompare(sctimer_readCounter()+TIMER_PERIOD);
   sctimer_enable();
   
   // prepare radio
   radio_rfOn();
   radio_setFrequency(CHANNEL, APP_STATE_RX);
   
   // switch in RX by default
   radio_rxEnable();
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
//                  if (app_vars.packet_len==22) {
//                        app_vars.flags |= APP_FLAG_TIMER;
//                  }
                  break;
               case APP_STATE_TX:
                  // done sending a packet
               
                  radio_setFrequency(CHANNEL, APP_STATE_RX);
                  
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
            
           // stop listening
           radio_rfOff();
           
           // prepare packet
           app_vars.packet_len = sizeof(app_vars.packet);
           for (i=0;i<app_vars.packet_len;i++) {
              app_vars.packet[i] = ID;
           }
           
           radio_setFrequency(CHANNEL, APP_STATE_TX);
           
           // start transmitting packet
           radio_loadPacket(app_vars.packet,125);
           radio_txEnable();
           app_vars.txFlag = TRUE;
           sctimer_setCompare(sctimer_readCounter()+10);
           app_vars.state = APP_STATE_TX;
            
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
}

void cb_endFrame(PORT_TIMER_WIDTH timestamp) {
   // set flag
   app_vars.flags |= APP_FLAG_END_FRAME;
   
   // update debug stats
   app_dbg.num_endFrame++;
}

void cb_timer(void) {
   // set flag
    if (app_vars.txFlag==FALSE) {
        app_vars.flags |= APP_FLAG_TIMER;
        sctimer_setCompare(sctimer_readCounter()+TIMER_PERIOD);
    } else {
        app_vars.txFlag = FALSE;
        radio_txNow();
        sctimer_setCompare(sctimer_readCounter()+TIMER_PERIOD);
    }
   
   // update debug stats
   app_dbg.num_timer++;
   
}
