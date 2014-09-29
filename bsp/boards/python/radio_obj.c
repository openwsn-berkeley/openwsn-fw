/**
\brief Python-specific definition of the "radio" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, May 2013.
*/

#include "radio_obj.h"

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== callbacks =======================================

void radio_setOverflowCb(OpenMote* self, radiotimer_compare_cbt cb) {
   
#ifdef TRACE_ON
   printf("C@0x%x: radio_setOverflowCb(cb=0x%x)... \n",self,cb);
#endif
   
   radiotimer_setOverflowCb(self, cb);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

void radio_setCompareCb(OpenMote* self, radiotimer_compare_cbt cb) {
   
#ifdef TRACE_ON
   printf("C@0x%x: radio_setCompareCb(cb=0x%x)... \n",self,cb);
#endif
   
   radiotimer_setCompareCb(self, cb);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

void radio_setStartFrameCb(OpenMote* self, radiotimer_capture_cbt cb) {
   
#ifdef TRACE_ON
   printf("C@0x%x: radio_setStartFrameCb(cb=0x%x)... \n",self,cb);
#endif
   
   self->radio_icb.startFrame_cb  = cb;
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

void radio_setEndFrameCb(OpenMote* self, radiotimer_capture_cbt cb) {
   
#ifdef TRACE_ON
   printf("C@0x%x: radio_setEndFrameCb(cb=0x%x)... \n",self,cb);
#endif
   
   self->radio_icb.endFrame_cb    = cb;
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

//=========================== public ==========================================

//===== admin

void radio_init(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: radio_init()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radio_init],NULL);
   if (result == NULL) {
      printf("[CRITICAL] radio_init() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

//===== reset

void radio_reset(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: radio_reset()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radio_reset],NULL);
   if (result == NULL) {
      printf("[CRITICAL] radio_reset() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

//===== timer

void radio_startTimer(OpenMote* self, PORT_TIMER_WIDTH period) {
   PyObject*   result;
   PyObject*   arglist;
   
#ifdef TRACE_ON
   printf("C@0x%x: radio_startTimer(period=%d)... \n",self,period);
#endif
   
   // forward to Python
   arglist    = Py_BuildValue("(i)",period);
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radio_startTimer],arglist);
   if (result == NULL) {
      printf("[CRITICAL] radio_startTimer() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   Py_DECREF(arglist);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

PORT_TIMER_WIDTH radio_getTimerValue(OpenMote* self) {
   PyObject*            result;
   PORT_TIMER_WIDTH     returnVal;
   
#ifdef TRACE_ON
   printf("C@0x%x: radio_getTimerValue()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radio_getTimerValue],NULL);
   if (result == NULL) {
      printf("[CRITICAL] radio_getTimerValue() returned NULL\r\n");
      return 0;
   }
   if (!PyInt_Check(result)) {
      printf("[CRITICAL] radio_getTimerValue() returned NULL\r\n");
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

void radio_setTimerPeriod(OpenMote* self, PORT_TIMER_WIDTH period) {
   PyObject*   result;
   PyObject*   arglist;
   
#ifdef TRACE_ON
   printf("C@0x%x: radio_setTimerPeriod(period=%d)... \n",self,period);
#endif
   
   // forward to Python
   arglist    = Py_BuildValue("(i)",period);
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radio_setTimerPeriod],arglist);
   if (result == NULL) {
      printf("[CRITICAL] radio_setTimerPeriod() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   Py_DECREF(arglist);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

PORT_TIMER_WIDTH radio_getTimerPeriod(OpenMote* self) {
   PyObject*            result;
   PORT_TIMER_WIDTH     returnVal;
   
#ifdef TRACE_ON
   printf("C@0x%x: radio_getTimerPeriod()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radio_getTimerPeriod],NULL);
   if (result == NULL) {
      printf("[CRITICAL] radio_getTimerPeriod() returned NULL\r\n");
      return 0;
   }
   if (!PyInt_Check(result)) {
      printf("[CRITICAL] radio_getTimerPeriod() returned something which is not an int\r\n");
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

//===== RF admin

void radio_setFrequency(OpenMote* self, uint8_t frequency) {
   PyObject*   result;
   PyObject*   arglist;
   
#ifdef TRACE_ON
   printf("C@0x%x: radio_setFrequency(frequency=%d)... \n",self,frequency);
#endif
   
   // forward to Python
   arglist    = Py_BuildValue("(i)",frequency);
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radio_setFrequency],arglist);
   if (result == NULL) {
      printf("[CRITICAL] radio_setFrequency() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   Py_DECREF(arglist);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

void radio_rfOn(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: radio_rfOn()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radio_rfOn],NULL);
   if (result == NULL) {
      printf("[CRITICAL] radio_rfOn() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

void radio_rfOff(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: radio_rfOff()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radio_rfOff],NULL);
   if (result == NULL) {
      printf("[CRITICAL] radio_rfOff() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

//===== TX

void radio_loadPacket(OpenMote* self, uint8_t* packet, uint8_t len) {
   PyObject*   pkt;
   PyObject*   arglist;
   PyObject*   result;
   PyObject*   item;
   int8_t      i;
   int         res;
   
#ifdef TRACE_ON
   printf("C@0x%x: radio_loadPacket(len=%d)... \n",self,len);
#endif
   
   // forward to Python
   pkt        = PyList_New(len);
   for (i=0;i<len;i++) {
      item    = PyInt_FromLong(packet[i]);
      res     = PyList_SetItem(pkt,i,item);
      if (res!=0) {
         printf("[CRITICAL] radio_loadPacket() failed setting list item\r\n");
         return;
      }
   }
   arglist    = Py_BuildValue("(O)",pkt);
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radio_loadPacket],arglist);
   if (result == NULL) {
      printf("[CRITICAL] radio_loadPacket() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   Py_DECREF(arglist);
   Py_DECREF(pkt);
}

void radio_txEnable(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: radio_txEnable()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radio_txEnable],NULL);
   if (result == NULL) {
      printf("[CRITICAL] radio_txEnable() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

void radio_txNow(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: radio_txNow()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radio_txNow],NULL);
   if (result == NULL) {
      printf("[CRITICAL] radio_txNow() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

//===== RX

void radio_rxEnable(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: radio_rxEnable()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radio_rxEnable],NULL);
   if (result == NULL) {
      printf("[CRITICAL] radio_rxEnable() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

void radio_rxNow(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: radio_rxNow()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radio_rxNow],NULL);
   if (result == NULL) {
      printf("[CRITICAL] radio_rxNow() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

void radio_getReceivedFrame(OpenMote* self,
                             uint8_t* pBufRead,
                             uint8_t* pLenRead,
                             uint8_t  maxBufLen,
                              int8_t* pRssi,
                             uint8_t* pLqi,
                                bool* pCrc) {
   PyObject*  result;
   PyObject*  item;
   PyObject*  subitem;
   int8_t     lenRead;
   int8_t     i;
   
#ifdef TRACE_ON
   printf("C@0x%x: radio_getReceivedFrame()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radio_getReceivedFrame],NULL);
   if (result == NULL) {
      printf("[CRITICAL] radio_getReceivedFrame() returned NULL\r\n");
      return;
   }
   
   // verify
   if (!PySequence_Check(result)) {
      printf("[CRITICAL] radio_getReceivedFrame() did not return a tuple\r\n");
      return;
   }
   if (PyTuple_Size(result)!=4) {
      printf("[CRITICAL] radio_getReceivedFrame() did not return a tuple of exactly 4 elements %d\r\n",PyList_Size(result));
      return;
   }
   
   //==== item 0: rxBuffer
   
   item       = PyTuple_GetItem(result,0);
   lenRead    = PyList_Size(item);
   *pLenRead  = lenRead;
   // store retrieved information
   for (i=0;i<lenRead;i++) {
      subitem = PyList_GetItem(item, i);
      pBufRead[i] = (uint8_t)PyInt_AsLong(subitem);
   }
   
   //==== item 1: rssi
   
   item       = PyTuple_GetItem(result,1);
   *pRssi     = (int8_t)PyInt_AsLong(item);
   
   //==== item 2: lqi
   
   item       = PyTuple_GetItem(result,2);
   *pLqi      = (uint8_t)PyInt_AsLong(item);
   
   //==== item 3: crc
   
   item       = PyTuple_GetItem(result,3);
   *pCrc      = (uint8_t)PyInt_AsLong(item);
}

//=========================== interrupts ======================================

void radio_intr_startOfFrame(OpenMote* self, uint16_t capturedTime) {
   self->radio_icb.startFrame_cb(self, capturedTime);
}

void radio_intr_endOfFrame(OpenMote* self, uint16_t capturedTime) {
   self->radio_icb.endFrame_cb(self, capturedTime);
}

//=========================== private =========================================
