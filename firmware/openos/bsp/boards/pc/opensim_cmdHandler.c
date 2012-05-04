/**
\brief Handler of commmands received from the OpenSim server.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, April 2012.
*/

#include <stdio.h>
#include "opensim_cmdHandler.h"
#include "opensim_proto.h"
// bsp
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
   switch (cmdType) {
      case OPENSIM_CMD_bsp_timer_isr:
         fprintf(stderr,"[opensim_cmdHandler] FATAL: OPENSIM_CMD_bsp_timer_isr not implemented\n");
         exit(1);
         break;
      case OPENSIM_CMD_radio_isr_startFrame:
         fprintf(stderr,"[opensim_cmdHandler] FATAL: OPENSIM_CMD_radio_isr_startFrame not implemented\n");
         exit(1);
         break;
      case OPENSIM_CMD_radio_isr_endFrame:
         fprintf(stderr,"[opensim_cmdHandler] FATAL: OPENSIM_CMD_radio_isr_endFrame not implemented\n");
         exit(1);
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
