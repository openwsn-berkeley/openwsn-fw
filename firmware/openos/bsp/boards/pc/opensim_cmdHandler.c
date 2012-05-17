/**
\brief Handler of commmands received from the OpenSim server.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, April 2012.
*/

#include <stdio.h>
#include "opensim_cmdHandler.h"
#include "opensim_proto.h"
// bsp
#include "bsp_timer.h"
#include "radiotimer.h"

//=========================== variables =======================================

/*
typedef struct {
} opensim_cmdHandler_vars_t;

opensim_cmdHandler_vars_t opensim_cmdHandler_vars;
*/

//=========================== prototypes ======================================

//=========================== public ==========================================

void opensim_cmdHandler_handle(int  cmdType,
                               int  paramLen,
                               int* paramBuf) {
   
   opensim_intr_radio_startOfFrame_t* radio_startOfFrame;
   opensim_intr_radio_endOfFrame_t*   radio_endOfFrame;
   
   switch (cmdType) {
      case OPENSIM_CMD_bsp_timer_isr:
         bsp_timer_isr();
         break;
      case OPENSIM_CMD_radio_isr_startFrame:
         if (paramLen!=sizeof(opensim_intr_radio_startOfFrame_t)) {
            fprintf(stderr,"[opensim_cmdHandler] FATAL: wrong param length in OPENSIM_CMD_radio_isr_startFrame\n");
            exit(1);
         }
         radio_startOfFrame = (opensim_intr_radio_startOfFrame_t*)paramBuf;
         radio_intr_startOfFrame(radio_startOfFrame->capturedTime);
         break;
      case OPENSIM_CMD_radio_isr_endFrame:
         if (paramLen!=sizeof(opensim_intr_radio_endOfFrame_t)) {
            fprintf(stderr,"[opensim_cmdHandler] FATAL: wrong param length in OPENSIM_CMD_radio_isr_startFrame\n");
            exit(1);
         }
         radio_endOfFrame = (opensim_intr_radio_endOfFrame_t*)paramBuf;
         radio_intr_endOfFrame(radio_endOfFrame->capturedTime);
         break;
      case OPENSIM_CMD_radiotimer_isr_compare:
         radiotimer_intr_compare();
         break;
      case OPENSIM_CMD_radiotimer_isr_overflow:
         radiotimer_intr_overflow();
         break;
      case OPENSIM_CMD_uart_isr_tx:
         fprintf(stderr,"[opensim_cmdHandler] FATAL: OPENSIM_CMD_uart_isr_tx not implemented\n");
         exit(1);
         break;
      case OPENSIM_CMD_uart_isr_rx:
         fprintf(stderr,"[opensim_cmdHandler] FATAL: OPENSIM_CMD_uart_isr_rx not implemented\n");
         exit(1);
         break;
      case OPENSIM_CMD_supply_on:
         fprintf(stderr,"[opensim_cmdHandler] FATAL: OPENSIM_CMD_supply_on not implemented\n");
         exit(1);
         break;
      case OPENSIM_CMD_supply_off:
         fprintf(stderr,"[opensim_cmdHandler] FATAL: OPENSIM_CMD_supply_off not implemented\n");
         exit(1);
         break;
      default:
         fprintf(stderr,"[opensim_cmdHandler] FATAL: unexcepted command %d\n",cmdType);
         exit(1);
         break;
   }
}

//=========================== private =========================================
