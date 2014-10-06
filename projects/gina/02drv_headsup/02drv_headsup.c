/**
\brief This is a program which shows how to use the "opentimers "driver module.

Since the driver modules for different platforms have the same declaration, you
can use this project with any platform.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, March 2012.
*/

#include "stdint.h"
#include "stdio.h"
// bsp modules required
#include "board.h"
#include "leds.h"
#include "uart.h"
// driver modules required
#include "opentimers.h"
#include "sensitive_accel_temperature.h"

//=========================== defines =========================================

#define APP_DLY_TIMER_ms     500
#define MEASUREMENENT_LENGTH 10
#define SERIALPKT_LENGTH     3+MEASUREMENENT_LENGTH+3

//=========================== variables =======================================

enum {
   APP_FLAG_TIMER       = 0x01,
   APP_FLAG_TXBYTE      = 0x02,
};

typedef struct {
   uint16_t num_timer;
   uint16_t num_txByte;
} app_dbg_t;

app_dbg_t app_dbg;

typedef struct {
   uint8_t  flags;
   uint8_t  lastSentByte;
   uint8_t  lastMeasurement[MEASUREMENENT_LENGTH];
   uint8_t  serialPk[SERIALPKT_LENGTH];
} app_vars_t;

app_vars_t app_vars;

//=========================== prototypes ======================================

void cb_timer();
void cb_uart_tx();
void cb_uart_rx();

//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int mote_main(void) {
   
   // clear local variables
   memset(&app_dbg, 0,sizeof(app_dbg_t) );
   memset(&app_vars,0,sizeof(app_vars_t));
   
   // initialize the board (i.e. the BSP)
   board_init();
   
   // intialize the drivers we will use
   opentimers_init();
   
   // enable UART
   uart_setCallbacks(cb_uart_tx,
                     cb_uart_rx);     // send callbacks
   uart_clearTxInterrupts();
   uart_clearRxInterrupts();          // clear possible pending interrupts
   uart_enableInterrupts();           // Enable USCI_A1 TX & RX interrupt
   
   // start the timer
   opentimers_start(APP_DLY_TIMER_ms,
                    TIMER_PERIODIC,TIME_MS,
                    cb_timer);
      
   while(1) {
      // sleep while waiting for at least one of the flags to be set
      while (app_vars.flags==0x00) {
         board_sleep();
      }
      
      // handle and clear every flag
      while (app_vars.flags) {
         
         if (app_vars.flags & APP_FLAG_TIMER) {
            // timer fired
            
            // toggle the LED
            leds_sync_toggle();
            
            // get a measurements from the accelerometer
            sensitive_accel_temperature_get_measurement(app_vars.lastMeasurement);
            
            // prepare the serial packet
            app_vars.serialPk[0] = '^';
            app_vars.serialPk[1] = '^';
            app_vars.serialPk[2] = '^';
            memcpy(&app_vars.serialPk[3],app_vars.lastMeasurement,MEASUREMENENT_LENGTH);
            app_vars.serialPk[13] = '$';
            app_vars.serialPk[14] = '$';
            app_vars.serialPk[15] = '$';
            
            // start by sending the first byte
            app_vars.lastSentByte = 0;
            
            // send the first byte
            uart_writeByte(app_vars.serialPk[app_vars.lastSentByte]);
            
            // clear flag
            app_vars.flags &= ~APP_FLAG_TIMER;
         }
      }
   }
}

//=========================== callbacks =======================================

void cb_timer() {
   // set flag
   app_vars.flags |= APP_FLAG_TIMER;
   // update debug stats
   app_dbg.num_timer++;
}

void cb_uart_tx() {
   // toggle the LED
   leds_radio_toggle();
   
   // plan on sending the next byte
   app_vars.lastSentByte++;
   
   // send byte, if applicable
   if (app_vars.lastSentByte<SERIALPKT_LENGTH) {
      uart_writeByte(app_vars.serialPk[app_vars.lastSentByte]);
   }
   
   // update debug stats
   app_dbg.num_txByte++;
}

void cb_uart_rx() {
   // this should never happen
   
   // simply toggle the error LED
   leds_error_toggle();
}