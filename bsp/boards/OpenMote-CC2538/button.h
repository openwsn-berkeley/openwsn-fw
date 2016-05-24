/**
 * Author: Xavier Vilajosana (xvilajosana@eecs.berkeley.edu)
 *         Pere Tuset (peretuset@openmote.com)
 * Date:   July 2013
 * Description: CC2538-specific definition of the "board" bsp module.
 */

#include <headers/hw_ioc.h>
#include <headers/hw_memmap.h>
#include <headers/hw_ssi.h>
#include <headers/hw_sys_ctrl.h>
#include <headers/hw_types.h>

#include <source/ioc.h>
#include <source/gpio.h>
#include <source/gptimer.h>
#include <source/sys_ctrl.h>
#include <source/interrupt.h>
#include <source/flash.h> 

#include "board.h"
#include "leds.h"
#include "gpio.h"
#include "bsp_timer.h"
#include "radiotimer.h"
#include "debugpins.h"
#include "uart.h"
#include "radio.h"
#include "i2c.h"
#include "sensors.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

void button_init(void);

//=========================== public ==========================================

//=========================== private =========================================

//=========================== interrupt handlers ==============================
