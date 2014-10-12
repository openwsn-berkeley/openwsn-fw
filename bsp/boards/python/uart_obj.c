/**
\brief Python-specific definition of the "uart" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, May 2013.
*/

#include "uart_obj.h"

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== callbacks =======================================

void uart_setCallbacks(OpenMote* self, uart_tx_cbt txCb, uart_rx_cbt rxCb) {
   
#ifdef TRACE_ON
   printf("C@0x%x: uart_setCallbacks(txCb=0x%x, rxCb=0x%x)... \n",self,txCb,rxCb);
#endif
   
   self->uart_icb.txCb = txCb;
   self->uart_icb.rxCb = rxCb;
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

//=========================== public ==========================================

void uart_init(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: uart_init()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_uart_init],NULL);
   if (result == NULL) {
      printf("[CRITICAL] uart_init() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

void uart_enableInterrupts(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: uart_enableInterrupts()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_uart_enableInterrupts],NULL);
   if (result == NULL) {
      printf("[CRITICAL] uart_enableInterrupts() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

void uart_disableInterrupts(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: uart_disableInterrupts()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_uart_disableInterrupts],NULL);
   if (result == NULL) {
      printf("[CRITICAL] uart_disableInterrupts() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

void uart_clearRxInterrupts(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: uart_clearRxInterrupts()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_uart_clearRxInterrupts],NULL);
   if (result == NULL) {
      printf("[CRITICAL] uart_clearRxInterrupts() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

void uart_clearTxInterrupts(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: uart_clearTxInterrupts()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_uart_clearTxInterrupts],NULL);
   if (result == NULL) {
      printf("[CRITICAL] uart_clearTxInterrupts() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

#ifdef FASTSIM
void uart_writeByte(OpenMote* self, uint8_t byteToWrite) {
   printf("[CRITICAL] uart_writeByte() should not be called\r\n");
}
#else
void uart_writeByte(OpenMote* self, uint8_t byteToWrite) {
   PyObject*   result;
   PyObject*   arglist;
   
#ifdef TRACE_ON
   printf("C@0x%x: uart_writeByte()... \n",self);
#endif
   
   // forward to Python
   arglist    = Py_BuildValue("(i)",byteToWrite);
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_uart_writeByte],arglist);
   if (result == NULL) {
      printf("[CRITICAL] uart_writeByte() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   Py_DECREF(arglist);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}
#endif

void uart_writeCircularBuffer_FASTSIM(OpenMote* self, uint8_t* buffer, uint8_t* outputBufIdxR, uint8_t* outputBufIdxW) {
   PyObject*   frame;
   PyObject*   arglist;
   PyObject*   result;
   PyObject*   item;
   int         res;
   uint8_t     len;
   uint8_t     i;
   
#ifdef TRACE_ON
   printf("C@0x%x: uart_writeCircularBuffer_FASTSIM(buffer=%x,outputBufIdxR=%x,outputBufIdxW=%x)... \n",
      self,
      buffer,
      outputBufIdxR,
      outputBufIdxW
   );
#endif
   
   // forward to Python
   len        = (*outputBufIdxW)-(*outputBufIdxR);
   frame      = PyList_New(len);
   i = 0;
   while (*outputBufIdxR!=*outputBufIdxW) {
      
      // get element at outputBufIdxR
      item    = PyInt_FromLong(buffer[*outputBufIdxR]);
      res     = PyList_SetItem(frame,i,item);
      if (res!=0) {
         printf("[CRITICAL] uart_writeCircularBuffer_FASTSIM() failed setting list item\r\n");
         return;
      }
      
      // increment index
      (*outputBufIdxR)++;
      i++;
   }
   arglist    = Py_BuildValue("(O)",frame);
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_uart_writeCircularBuffer_FASTSIM],arglist);
   if (result == NULL) {
      printf("[CRITICAL] uart_writeCircularBuffer_FASTSIM() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   Py_DECREF(arglist);
   Py_DECREF(frame);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

void uart_writeBufferByLen_FASTSIM(OpenMote* self, uint8_t* buffer, uint8_t len) {
   PyObject*   frame;
   PyObject*   arglist;
   PyObject*   result;
   PyObject*   item;
   uint8_t     i;
   int         res;
   
#ifdef TRACE_ON
   printf("C@0x%x: uart_writeBufferByLen_FASTSIM(buffer=%x,len=%d)... \n",
      self,
      buffer,
      len
   );
#endif
   
   // forward to Python
   frame      = PyList_New(len);
   if (frame==NULL) {
      printf("[CRITICAL] PyList_New(%d) failed in uart_writeBufferByLen_FASTSIM\r\n",len);
      return;
   }
   for (i=0;i<len;i++) {
      item    = PyInt_FromLong(buffer[i]);
      res     = PyList_SetItem(frame,i,item);
      if (res!=0) {
         printf("[CRITICAL] uart_writeBufferByLen_FASTSIM() failed setting list item\r\n");
         return;
      }
   }
   arglist    = Py_BuildValue("(O)",frame);
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_uart_writeBufferByLen_FASTSIM],arglist);
   if (result == NULL) {
      printf("[CRITICAL] uart_writeBufferByLen_FASTSIM() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   Py_DECREF(arglist);
   Py_DECREF(frame);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

uint8_t uart_readByte(OpenMote* self) {
   PyObject*  result;
   uint8_t    returnVal;
   
#ifdef TRACE_ON
   printf("C@0x%x: uart_readByte()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_uart_readByte],NULL);
   if (result == NULL) {
      printf("[CRITICAL] uart_readByte() returned NULL\r\n");
      return 0;
   }
   if (!PyInt_Check(result)) {
      printf("[CRITICAL] uart_readByte() returned NULL\r\n");
      return 0;
   }
   returnVal = PyInt_AsLong(result);
   
   // dispose of returned value
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...got %d.\n",self,returnVal);
#endif
   
   return returnVal;
}

//=========================== interrupt handlers ==============================

void uart_intr_tx(OpenMote* self) {

#ifdef TRACE_ON
   printf("C@0x%x: uart_intr_tx(), calling 0x%x... \n",self,self->uart_icb.txCb);
#endif
   
   self->uart_icb.txCb(self);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

void uart_intr_rx(OpenMote* self) {

#ifdef TRACE_ON
   printf("C@0x%x: uart_intr_rx(), calling 0x%x... \n",self,self->uart_icb.txCb);
#endif
   
   self->uart_icb.rxCb(self);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}