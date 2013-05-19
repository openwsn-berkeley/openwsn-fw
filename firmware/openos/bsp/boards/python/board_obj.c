/**
\brief Python-specific definition of the "board" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, May 2013.
*/

#include <stdio.h>
#include "board_obj.h"
// bsp modules
#include "debugpins_obj.h"
#include "leds_obj.h"
#include "uart_obj.h"
#include "bsp_timer_obj.h"
#include "radio_obj.h"
#include "radiotimer_obj.h"
#include "eui64_obj.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void board_init(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C: board_init()\n");
#endif
   
   // initialize bsp modules
   debugpins_init(self);
   leds_init(self);
   uart_init(self);
   bsp_timer_init(self);
   radio_init(self);
   radiotimer_init(self);
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_board_init],NULL);
   if (result == NULL) {
      printf("[CRITICAL] board_init() returned NULL\r\n");
   }
   Py_DECREF(result);
}

void board_sleep(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C: board_sleep()\n");
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_board_sleep],NULL);
   if (result == NULL) {
      printf("[CRITICAL] board_sleep() returned NULL\r\n");
   }
   Py_DECREF(result);
}

void board_reset(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C: board_reset()\n");
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_board_reset],NULL);
   if (result == NULL) {
      printf("[CRITICAL] board_reset() returned NULL\r\n");
   }
   Py_DECREF(result);
}

//=========================== private =========================================
