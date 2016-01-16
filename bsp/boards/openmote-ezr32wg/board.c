/**
 * Author: Xavier Vilajosana (xvilajosana@eecs.berkeley.edu)
 *         Pere Tuset (peretuset@openmote.com)
 * Date:   Jan 2016
 * Description: ezr32wg-specific definition of the "board" bsp module.
 */


#include "board.h"
#include "leds.h"
#include "bsp_timer.h"
#include "radiotimer.h"
#include "debugpins.h"
#include "uart.h"
#include "radio.h"
#include "i2c.h"
#include "sensors.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

void antenna_init(void);
void antenna_internal(void);
void antenna_external(void);

static void clock_init(void);
static void gpio_init(void);
static void button_init(void);


//=========================== main ============================================

extern int mote_main(void);

int main(void) {
   return mote_main();
}

//=========================== public ==========================================

void board_init(void) {
   gpio_init();
   clock_init();

   board_timer_init();

   antenna_init();
   antenna_external();

   leds_init();
   debugpins_init();
   button_init();
   bsp_timer_init();
   radiotimer_init();
   uart_init();
   radio_init();
   i2c_init();
   sensors_init();
}

/**
 * Puts the board to sleep
 */
void board_sleep(void) {

}

/**
 * Resets the board
 */
void board_reset(void) {

}

/**
 * Configures the antenna using a RF switch
 * INT is the internal antenna (chip) configured through ANT1_SEL (V1)
 * EXT is the external antenna (connector) configured through ANT2_SEL (V2)
 */
void antenna_init(void) {

}

/**
 * Selects the external (connector) antenna
 */
void antenna_external(void) {

}

/**
 * Selects the internal (chip) antenna
 */
void antenna_internal(void) {

}

//=========================== private =========================================

static void gpio_init(void) {

}

static void clock_init(void) {

}

/**
 * Configures the user button as input source
 */
static void button_init(void) {

}
//=========================== interrupt handlers ==============================

