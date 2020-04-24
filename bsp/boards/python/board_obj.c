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
#include "radio_obj.h"
#include "eui64_obj.h"
#include "sctimer_obj.h"

//=========================== variables ============================================
slot_board_vars_t slot_board_vars [MAX_SLOT_TYPES];
slotType_t selected_slot_type;

//=========================== prototypes ======================================

//=========================== public ==========================================

void board_init(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: board_init()...\n",self);
#endif
   
   // initialize bsp modules
   debugpins_init(self);
   leds_init(self);
   sctimer_init(self);
   uart_init(self);
   radio_init(self);
   
   board_init_slot_vars();

   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_board_init],NULL);
   if (result == NULL) {
      printf("[CRITICAL] board_init() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}


//==== bootstrapping slot info lookup table
void board_init_slot_vars(void){
    //10ms slot
    slot_board_vars [SLOT_10ms].slotDuration                         = 328  ;   // ms 
    slot_board_vars [SLOT_10ms].maxTxDataPrepare                     = 10  ;  //  305us (measured  82us)
    slot_board_vars [SLOT_10ms].maxRxAckPrepare                      = 10  ;  //  305us (measured  83us)
    slot_board_vars [SLOT_10ms].maxRxDataPrepare                     =  4  ;  //  122us (measured  22us)
    slot_board_vars [SLOT_10ms].maxTxAckPrepare                      =  4  ;  //  122us (measured  94us)
    slot_board_vars [SLOT_10ms].delayTx                              =  7  ;  //  213us (measured xxxus)
    slot_board_vars [SLOT_10ms].delayRx                              =  0  ; //    0us (can not measure)
}

// To get the current slotDuration at any time
// used during initialization by sixtop to fire the first sixtop EB
uint16_t board_getSlotDuration (void)
{
    return slot_board_vars [selected_slot_type].slotDuration;
}

// Getter function for slot_board_vars
slot_board_vars_t board_getSlotTemplate (void)
{
  return slot_board_vars [selected_slot_type];
}

// Getter function for selected_slot_type
void board_setSlotType(slotType_t slot_type)
{
  selected_slot_type = slot_type;
}
    
void board_sleep(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: board_sleep()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_board_sleep],NULL);
   if (result == NULL) {
      printf("[CRITICAL] board_sleep() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

void board_reset(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: board_reset()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_board_reset],NULL);
   if (result == NULL) {
      printf("[CRITICAL] board_reset() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

//=========================== private =========================================
