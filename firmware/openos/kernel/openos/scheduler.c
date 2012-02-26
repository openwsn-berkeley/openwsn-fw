//ISR_BUTTON
//ISR_SPI

#include "scheduler.h"
#include "opentimers.h"
#include "ieee154etimer.h"
#include "IEEE802154E.h"
#include "i2c.h"
#include "res.h"
#include "openserial.h"
#include "debugpins.h"

//=========================== variables =======================================

typedef struct {
   uint8_t task_list[MAX_NUM_TASKS];
   uint8_t num_tasks;
} scheduler_vars_t;

scheduler_vars_t scheduler_vars;

extern opentimers_vars_t opentimers_vars;

//=========================== prototypes ======================================

__monitor uint8_t isThereTask(uint8_t taskId);
__monitor void    consumeTask(uint8_t taskId);

//=========================== public ==========================================

void scheduler_init() {
   uint8_t i;
   // enable debug pins
   DEBUG_PIN_TASK_INIT();
   DEBUG_PIN_ISR_INIT();
   // initialization module variables
   scheduler_vars.num_tasks       = 0;
   for (i=0;i<MAX_NUM_TASKS;i++) {
      scheduler_vars.task_list[i] = 0;
   }
   // enable the comparatorA interrupt to SW can wake up the scheduler
   CACTL1 = CAIE;
}

void scheduler_start() {
   while (1) {
      while(scheduler_vars.num_tasks>0) {
         if        (isThereTask(TASKID_RESNOTIF_RX)==TRUE) {
            consumeTask(TASKID_RESNOTIF_RX);
            task_resNotifReceive();
         } else if (isThereTask(TASKID_RESNOTIF_TXDONE)==TRUE) {
            consumeTask(TASKID_RESNOTIF_TXDONE);
            task_resNotifSendDone();
         } else if (isThereTask(TASKID_RES)==TRUE) {
            consumeTask(TASKID_RES);
            opentimers_res_fired();
         } else if (isThereTask(TASKID_RPL)==TRUE) {
            consumeTask(TASKID_RPL);
            opentimers_rpl_fired();
         } else if (isThereTask(TASKID_TCP_TIMEOUT)==TRUE) {
            consumeTask(TASKID_TCP_TIMEOUT);
            opentimers_tcp_fired();
         } else if (isThereTask(TASKID_COAP)==TRUE) {
            consumeTask(TASKID_COAP);
            opentimers_coap_fired();
         } else if (isThereTask(TASKID_TIMERB4)==TRUE) {
            consumeTask(TASKID_TIMERB4);
            // timer available, put your function here
         } else if (isThereTask(TASKID_TIMERB5)==TRUE) {
            consumeTask(TASKID_TIMERB5);
            // timer available, put your function here
         } else if (isThereTask(TASKID_TIMERB6)==TRUE) {
            consumeTask(TASKID_TIMERB6);
            // timer available, put your function here
         } else if (isThereTask(TASKID_BUTTON)==TRUE) {
            consumeTask(TASKID_BUTTON);
#ifdef ISR_BUTTON
            isr_button();
#endif
         } else {
            while(1);
         }
      }
      DEBUG_PIN_TASK_CLR();
      __bis_SR_register(GIE+LPM3_bits);          // sleep, but leave interrupts and ACLK on 
      DEBUG_PIN_TASK_SET();                      // IAR should halt here if nothing to do
   }
}

__monitor void scheduler_push_task(int8_t task_id) {
   if(task_id>=MAX_NUM_TASKS) { while(1); }
   scheduler_vars.task_list[task_id]++;
   scheduler_vars.num_tasks++;
}

//=========================== private =========================================

__monitor uint8_t isThereTask(uint8_t taskId) {
   return (scheduler_vars.task_list[taskId]>0);
}

__monitor void consumeTask(uint8_t taskId) {
   scheduler_vars.task_list[taskId]--;
   scheduler_vars.num_tasks--;
}

//=========================== interrupt handlers ==============================

//======= interrupt which wakes up the scheduler from SW

#pragma vector = COMPARATORA_VECTOR
__interrupt void COMPARATORA_ISR (void) {
   CAPTURE_TIME();
   DEBUG_PIN_ISR_SET();
   __bic_SR_register_on_exit(CPUOFF);                 // restart CPU
   DEBUG_PIN_ISR_CLR();
}

//======= interrupts which post a task

#pragma vector = PORT2_VECTOR
__interrupt void PORT2_ISR (void) {
   CAPTURE_TIME();
   DEBUG_PIN_ISR_SET();
#ifdef ISR_BUTTON
   //interrupt from button connected to P2.7
   if ((P2IFG & 0x80)!=0) {
      P2IFG &= ~0x80;                                 // clear interrupt flag
      scheduler_push_task(ID_ISR_BUTTON);             // post task
      __bic_SR_register_on_exit(CPUOFF);              // restart CPU
   }
#endif
   DEBUG_PIN_ISR_CLR();
}

// TimerB CCR0 interrupt service routine
#pragma vector = TIMERB0_VECTOR
__interrupt void TIMERB0_ISR (void) {
   CAPTURE_TIME();
   DEBUG_PIN_ISR_SET();
   if (opentimers_vars.continuous[0]==TRUE) {
      TBCCR0 += opentimers_vars.period[0];            // continuous timer: schedule next instant
   } else {
      TBCCTL0 = 0;                                    // stop the timer
      TBCCR0  = 0;
   }
   scheduler_push_task(TASKID_RES);                   // post the corresponding task
   __bic_SR_register_on_exit(CPUOFF);                 // restart CPU
   DEBUG_PIN_ISR_CLR();
}

// TimerB CCR1-6 interrupt service routine
#pragma vector = TIMERB1_VECTOR
__interrupt void TIMERB1through6_ISR (void) {
   CAPTURE_TIME();
   DEBUG_PIN_ISR_SET();
   uint16_t tbiv_temp = TBIV;                         // read only once because accessing TBIV resets it
   switch (tbiv_temp) {
      case 0x0002: // timerB CCR1
         if (opentimers_vars.continuous[1]==TRUE) {
            TBCCR1 += opentimers_vars.period[1];      // continuous timer: schedule next instant
         } else {
            TBCCTL1 = 0;                              // stop the timer
            TBCCR1  = 0;
         }
         scheduler_push_task(TASKID_RPL);             // post the corresponding task
         __bic_SR_register_on_exit(CPUOFF);           // restart CPU
         break;
      case 0x0004: // timerB CCR2
         if (opentimers_vars.continuous[2]==TRUE) {
            TBCCR2 += opentimers_vars.period[2];      // continuous timer: schedule next instant
         } else {
            TBCCTL2 = 0;                              // stop the timer
            TBCCR2  = 0;
         }
         scheduler_push_task(TASKID_TCP_TIMEOUT);     // post the corresponding task
         __bic_SR_register_on_exit(CPUOFF);           // restart CPU
         break;
      case 0x0006: // timerB CCR3
         if (opentimers_vars.continuous[3]==TRUE) {
            TBCCR3 += opentimers_vars.period[3];      // continuous timer: schedule next instant
         } else {
            TBCCTL3 = 0;                              // stop the timer
            TBCCR3  = 0;
         }
         scheduler_push_task(TASKID_COAP);            // post the corresponding task
         __bic_SR_register_on_exit(CPUOFF);           // restart CPU
         break;
      case 0x0008: // timerB CCR4
         if (opentimers_vars.continuous[4]==TRUE) {
            TBCCR4 += opentimers_vars.period[4];      // continuous timer: schedule next instant
         } else {
            TBCCTL4 = 0;                              // stop the timer
            TBCCR4  = 0;
         }
         scheduler_push_task(TASKID_TIMERB4);         // post the corresponding task
         __bic_SR_register_on_exit(CPUOFF);           // restart CPU
         break;
      case 0x000A: // timerB CCR5
         if (opentimers_vars.continuous[5]==TRUE) {
            TBCCR5 += opentimers_vars.period[5];      // continuous timer: schedule next instant
         } else {
            TBCCTL5 = 0;                              // stop the timer
            TBCCR5  = 0;
         }
         scheduler_push_task(TASKID_TIMERB5);         // post the corresponding task
         __bic_SR_register_on_exit(CPUOFF);           // restart CPU
         break;
      case 0x000C: // timerB CCR6
         if (opentimers_vars.continuous[6]==TRUE) {
            TBCCR6 += opentimers_vars.period[6];      // continuous timer: schedule next instant
         } else {
            TBCCTL6 = 0;                              // stop the timer
            TBCCR6  = 0;
         }
         scheduler_push_task(TASKID_TIMERB6);         // post the corresponding task
         __bic_SR_register_on_exit(CPUOFF);           // restart CPU
         break;
      default:
         while(1);                                    // this should not happen
   }
   DEBUG_PIN_ISR_CLR();
}

//======= interrupts handled directly in ISR mode

/*
// TimerA CCR0 interrupt service routine
#pragma vector = TIMERA0_VECTOR
__interrupt void TIMERA0_ISR (void) {
   CAPTURE_TIME();
   DEBUG_PIN_ISR_SET();
   isr_ieee154e_newSlot();
   DEBUG_PIN_ISR_CLR();
}
*/

// TimerA CCR1-2 interrupt service routine
#pragma vector = TIMERA1_VECTOR
__interrupt void TIMERA1and2_ISR (void) {
   CAPTURE_TIME();
   DEBUG_PIN_ISR_SET();
   uint16_t taiv_temp = TAIV;                    // read only once because accessing TAIV resets it
   switch (taiv_temp) {
      case 0x0002: // capture/compare CCR1
         isr_ieee154e_timer();
         break;
      case 0x000a: // timer overflows
         isr_ieee154e_newSlot();
         break;
      case 0x0004: // capture/compare CCR2
      default:
         while(1);                               // this should not happen
   }
   DEBUG_PIN_ISR_CLR();
}

#pragma vector = PORT1_VECTOR
__interrupt void PORT1_ISR (void) {
    CAPTURE_TIME();
    DEBUG_PIN_ISR_SET();
   //interrupt from radio through IRQ_RF connected to P1.6
   if ((P1IFG & 0x40)!=0) {
      P1IFG &= ~0x40;                            // clear interrupt flag
      isr_radio();
   }
   DEBUG_PIN_ISR_CLR();
}

/* 
 * The GINA board has three buses: I2C, SPI, UART. We handle the
 * related interrupts directly.
 *
 * UCA1 = serial
 * UCB1 = I2C
 * UCA0 = SPI
 */

#pragma vector = USCIAB1TX_VECTOR
__interrupt void USCIAB1TX_ISR(void) {
   CAPTURE_TIME();
   DEBUG_PIN_ISR_SET();
   if ( ((UC1IFG & UCB1TXIFG) && (UC1IE & UCB1TXIE)) ||
        ((UC1IFG & UCB1RXIFG) && (UC1IE & UCB1RXIE)) ) {
      isr_i2c_tx(1);                         // implemented in I2C driver
   }
   if ( (UC1IFG & UCA1TXIFG) && (UC1IE & UCA1TXIE) ){
      isr_openserial_tx();                       // implemented in serial driver
   }
   DEBUG_PIN_ISR_CLR();
}

#pragma vector = USCIAB1RX_VECTOR
__interrupt void USCIAB1RX_ISR(void) {
   CAPTURE_TIME();
   DEBUG_PIN_ISR_SET();
   if ( ((UC1IFG & UCB1RXIFG) && (UC1IE & UCB1RXIE)) ||
         (UCB1STAT & UCNACKIFG) ) {
      isr_i2c_rx(1);                             // implemented in I2C driver
   }
   if ( (UC1IFG & UCA1RXIFG) && (UC1IE & UCA1RXIE) ){
      isr_openserial_rx();                  // implemented in serial driver
   }
   DEBUG_PIN_ISR_CLR();
}

#pragma vector = USCIAB0RX_VECTOR
__interrupt void USCIAB0RX_ISR (void) {
    CAPTURE_TIME();
    DEBUG_PIN_ISR_SET();
#ifdef ISR_SPI
   if ( (IFG2 & UCA0RXIFG) && (IE2 & UCA0RXIE) ) {
      isr_spi_rx();                         // implemented in SPI driver
   }
#endif
   if ( ((IFG2 & UCB0RXIFG) && (IE2 & UCB0RXIE)) ||
        (UCB0STAT & UCNACKIFG) ) {
      isr_i2c_rx(0);                             // implemented in I2C driver
   }
   DEBUG_PIN_ISR_CLR();
}

//======= handled as CPUOFF

// TODO: this is bad practice, should redo, even a busy wait is better

#pragma vector = ADC12_VECTOR
__interrupt void ADC12_ISR (void) {
   CAPTURE_TIME();
   DEBUG_PIN_ISR_SET();
   ADC12IFG &= ~0x1F;                            // clear interrupt flags
   __bic_SR_register_on_exit(CPUOFF);            // restart CPU
   DEBUG_PIN_ISR_CLR();
}
