/**
\brief LPC-specific definition of the "timers" bsp module.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, February 2012.
*/

#include "string.h"
#include "timer.h"
#include "bsp_timers.h"
#include "board.h"

//=========================== defines =========================================

#define TIMER_COUNT     6              // number of available timer;

//=========================== variables =======================================

typedef struct {
   uint16_t        period[TIMER_COUNT];
   timer_type_t    type[TIMER_COUNT];
   timer_cbt       timers_cb[TIMER_COUNT];
} timers_vars_t;

timers_vars_t timers_vars;

//=========================== prototypes ======================================

void timer_compare_isr_0(uint8_t reg);
void timer_compare_isr_1(uint8_t reg);
void timer_capture_isr_0(uint8_t reg);
void timer_capture_isr_1(uint8_t reg);

//=========================== public ==========================================

void timers_init() {

   // clear local variables
   memset(&timers_vars,0,sizeof(timers_vars));

   //set timer hooks.
   timer_set_isr_compare_hook(TIMER_NUM0,timer_compare_isr_0);
   timer_set_isr_compare_hook(TIMER_NUM1,timer_compare_isr_1);
   timer_set_isr_capture_hook(TIMER_NUM0,timer_capture_isr_0);
   timer_set_isr_capture_hook(TIMER_NUM1,timer_capture_isr_1);

   timer_init(TIMER_NUM0);
   timer_init(TIMER_NUM1);
   timer_enable(TIMER_NUM0);
   timer_enable(TIMER_NUM1);
}

void timers_start(uint8_t id, uint16_t duration, timer_type_t type, timer_cbt callback) {
   uint32_t current=0;

   // register timer
   timers_vars.period[id]    = duration;
   timers_vars.type[id]      = type;
   timers_vars.timers_cb[id] = callback;

   switch(id) {
      case 0:
         current = timer_get_current_value(TIMER_NUM0);
         timer_set_compare(TIMER_NUM0,
                           TIMER_COMPARE_REG0,
                           current+timers_vars.period[id]*TIMER_to_32kHz);
         break;
      case 1:
         current = timer_get_current_value(TIMER_NUM0);
         timer_set_compare(TIMER_NUM0,
                           TIMER_COMPARE_REG1,
                           current+timers_vars.period[id]*TIMER_to_32kHz);
         break;
      case 2:
         current = timer_get_current_value(TIMER_NUM0);
         timer_set_compare(TIMER_NUM0,
                           TIMER_COMPARE_REG2,
                           current+timers_vars.period[id]*TIMER_to_32kHz);
         break;
      case 3:
         current = timer_get_current_value(TIMER_NUM1);
         timer_set_compare(TIMER_NUM1,
                           TIMER_COMPARE_REG0,
                           current+timers_vars.period[id]*TIMER_to_32kHz);
         break;
      case 4:
         current = timer_get_current_value(TIMER_NUM1);
         timer_set_compare(TIMER_NUM1,
                           TIMER_COMPARE_REG1,
                           current+timers_vars.period[id]*TIMER_to_32kHz);
         break;
      case 5:
         current = timer_get_current_value(TIMER_NUM1);
         timer_set_compare(TIMER_NUM1,
                           TIMER_COMPARE_REG2,
                           current+timers_vars.period[id]*TIMER_to_32kHz);
         break;
   }
}

void timers_stop(uint8_t id) {

   // unregister timer
   timers_vars.period[id]    = 0;
   timers_vars.timers_cb[id] = NULL;

   // play with HW registers
   switch(id) {
      case 0:
         timer_reset_compare(TIMER_NUM0,TIMER_COMPARE_REG0);
         break;
      case 1:
         timer_reset_compare(TIMER_NUM0,TIMER_COMPARE_REG1);
         break;
      case 2:
         timer_reset_compare(TIMER_NUM0,TIMER_COMPARE_REG2);
         break;
      case 3:
         timer_reset_compare(TIMER_NUM1,TIMER_COMPARE_REG0);
         break;
      case 4:
         timer_reset_compare(TIMER_NUM1,TIMER_COMPARE_REG1);
         break;
      case 5:
         timer_reset_compare(TIMER_NUM1,TIMER_COMPARE_REG2);
         break;
   }
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================

/**
\brief Interrupt handler for TIMER_NUM0, compare
*/
void timer_compare_isr_0(uint8_t reg) {

   uint8_t id=TIMER_NUM0+reg;
   uint32_t current=0;

   if (timers_vars.type[id]==TIMER_PERIODIC) {
      current=timer_get_current_value(TIMER_NUM0);
      // continuous timer: schedule next instant
      timer_set_compare(TIMER_NUM0,reg,current+timers_vars.period[id]*TIMER_to_32kHz);
   } else {
      timer_reset_compare(TIMER_NUM0,reg);
   }
   // call the callback
   timers_vars.timers_cb[id]();
   // kick the OS
   // TODO
}

/**
\brief Interrupt handler for TIMER_NUM1, compare
*/
void timer_compare_isr_1(uint8_t reg) {

   uint8_t id=TIMER_NUM1+reg;
   uint32_t current=0;

   if (timers_vars.type[id]==TIMER_PERIODIC) {
      current=timer_get_current_value(TIMER_NUM1);
      // continuous timer: schedule next instant
      timer_set_compare(TIMER_NUM1,reg,current+timers_vars.period[id]*TIMER_to_32kHz);
   } else {
      timer_reset_compare(TIMER_NUM1,reg);
   }
   // call the callback
   timers_vars.timers_cb[id]();
   // kick the OS
   // TODO
}


/**
\brief Interrupt handler for TIMER_NUM0, capture
*/
void timer_capture_isr_0(uint8_t reg) {
   // TODO
}

/**
\brief Interrupt handler for TIMER_NUM1, capture
*/
void timer_capture_isr_1(uint8_t reg) {
   // TODO
}
