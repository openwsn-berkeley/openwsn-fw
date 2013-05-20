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
   radiotimer_setOverflowCb(self, cb);
}

void radio_setCompareCb(OpenMote* self, radiotimer_compare_cbt cb) {
   radiotimer_setCompareCb(self, cb);
}

void radio_setStartFrameCb(OpenMote* self, radiotimer_capture_cbt cb) {
   self->radio_icb.startFrame_cb  = cb;
}

void radio_setEndFrameCb(OpenMote* self, radiotimer_capture_cbt cb) {
   self->radio_icb.endFrame_cb    = cb;
}

//=========================== public ==========================================

//===== admin

void radio_init(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C: radio_init()... \n");
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radio_init],NULL);
   if (result == NULL) {
      printf("[CRITICAL] radio_init() returned NULL\r\n");
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C: ...done.\n");
#endif
}

//===== reset

void radio_reset(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C: radio_reset()... \n");
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radio_reset],NULL);
   if (result == NULL) {
      printf("[CRITICAL] radio_reset() returned NULL\r\n");
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C: ...done.\n");
#endif
}

//===== timer

void radio_startTimer(OpenMote* self, PORT_TIMER_WIDTH period) {
   PyObject*   result;
   PyObject*   arglist;
   
#ifdef TRACE_ON
   printf("C: radio_startTimer(period=%d)... \n",period);
#endif
   
   // forward to Python
   arglist    = Py_BuildValue("(i)",period);
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radio_startTimer],arglist);
   if (result == NULL) {
      printf("[CRITICAL] radio_startTimer() returned NULL\r\n");
   }
   Py_DECREF(result);
   Py_DECREF(arglist);
   
#ifdef TRACE_ON
   printf("C: ...done.\n");
#endif
}

PORT_TIMER_WIDTH radio_getTimerValue(OpenMote* self) {
   PyObject*            result;
   PORT_TIMER_WIDTH     returnVal;
   
#ifdef TRACE_ON
   printf("C: radio_getTimerValue()... \n");
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radio_getTimerValue],NULL);
   if (result == NULL) {
      printf("[CRITICAL] radio_getTimerValue() returned NULL\r\n");
   }
   if (!PyInt_Check(result)) {
      printf("[CRITICAL] radio_getTimerValue() returned NULL\r\n");
   }
   returnVal = PyInt_AsLong(result);
   
   // dispose of returned value
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C: ...got %d.\n",returnVal);
#endif
   
   return returnVal;
}

void radio_setTimerPeriod(OpenMote* self, PORT_TIMER_WIDTH period) {
   PyObject*   result;
   PyObject*   arglist;
   
#ifdef TRACE_ON
   printf("C: radio_setTimerPeriod(period=%d)... \n",period);
#endif
   
   // forward to Python
   arglist    = Py_BuildValue("(i)",period);
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radio_setTimerPeriod],arglist);
   if (result == NULL) {
      printf("[CRITICAL] radio_setTimerPeriod() returned NULL\r\n");
   }
   Py_DECREF(result);
   Py_DECREF(arglist);
   
#ifdef TRACE_ON
   printf("C: ...done.\n");
#endif
}

PORT_TIMER_WIDTH radio_getTimerPeriod(OpenMote* self) {
   PyObject*            result;
   PORT_TIMER_WIDTH     returnVal;
   
#ifdef TRACE_ON
   printf("C: radio_getTimerPeriod()... \n");
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radio_getTimerPeriod],NULL);
   if (result == NULL) {
      printf("[CRITICAL] radio_getTimerPeriod() returned NULL\r\n");
   }
   if (!PyInt_Check(result)) {
      printf("[CRITICAL] radio_getTimerPeriod() returned something which is not an int\r\n");
   }
   returnVal = PyInt_AsLong(result);
   
   // dispose of returned value
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C: ...got %d.\n",returnVal);
#endif
   
   return returnVal;
}

//===== RF admin

void radio_setFrequency(OpenMote* self, uint8_t frequency) {
   PyObject*   result;
   PyObject*   arglist;
   
#ifdef TRACE_ON
   printf("C: radio_setFrequency(frequency=%d)... \n",frequency);
#endif
   
   // forward to Python
   arglist    = Py_BuildValue("(i)",frequency);
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radio_setFrequency],arglist);
   if (result == NULL) {
      printf("[CRITICAL] radio_setFrequency() returned NULL\r\n");
   }
   Py_DECREF(result);
   Py_DECREF(arglist);
   
#ifdef TRACE_ON
   printf("C: ...done.\n");
#endif
}

void radio_rfOn(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C: radio_rfOn()... \n");
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radio_rfOn],NULL);
   if (result == NULL) {
      printf("[CRITICAL] radio_rfOn() returned NULL\r\n");
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C: ...done.\n");
#endif
}

void radio_rfOff(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C: radio_rfOff()... \n");
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radio_rfOff],NULL);
   if (result == NULL) {
      printf("[CRITICAL] radio_rfOff() returned NULL\r\n");
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C: ...done.\n");
#endif
}

//===== TX

void radio_loadPacket(OpenMote* self, uint8_t* packet, uint8_t len) {
   //opensim_requ_radio_loadPacket_t requparams;
   
   //requparams.len = len;
   //memcpy(requparams.txBuffer,packet,len);
   
   // send request to server and get reply
   /*
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_radio_loadPacket,
                                    &requparams,
                                    sizeof(opensim_requ_radio_loadPacket_t),
                                    0,
                                    0);
   */
   // TODO: replace by call to Python
}

void radio_txEnable(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C: radio_txEnable()... \n");
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radio_txEnable],NULL);
   if (result == NULL) {
      printf("[CRITICAL] radio_txEnable() returned NULL\r\n");
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C: ...done.\n");
#endif
}

void radio_txNow(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C: radio_txNow()... \n");
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radio_txNow],NULL);
   if (result == NULL) {
      printf("[CRITICAL] radio_txNow() returned NULL\r\n");
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C: ...done.\n");
#endif
}

//===== RX

void radio_rxEnable(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C: radio_rxEnable()... \n");
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radio_rxEnable],NULL);
   if (result == NULL) {
      printf("[CRITICAL] radio_rxEnable() returned NULL\r\n");
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C: ...done.\n");
#endif
}

void radio_rxNow(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C: radio_rxNow()... \n");
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radio_rxNow],NULL);
   if (result == NULL) {
      printf("[CRITICAL] radio_rxNow() returned NULL\r\n");
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C: ...done.\n");
#endif
}

void radio_getReceivedFrame(OpenMote* self,
                             uint8_t* pBufRead,
                             uint8_t* pLenRead,
                             uint8_t  maxBufLen,
                              int8_t* pRssi,
                             uint8_t* pLqi,
                             uint8_t* pCrc) {
   /*
   uint8_t numBytesToWrite;
   
   //opensim_repl_radio_getReceivedFrame_t replparams;
   
   // send request to server and get reply
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_radio_getReceivedFrame,
                                    0,
                                    0,
                                    &replparams,
                                    sizeof(opensim_repl_radio_getReceivedFrame_t));
   
   // TODO: replace by call to Python
   
   if (maxBufLen>replparams.len) {
      numBytesToWrite=replparams.len;
   } else {
      numBytesToWrite=maxBufLen;
   }
   
   // write return values
   memcpy(pBufRead,replparams.rxBuffer,numBytesToWrite);
   *pLenRead = replparams.len;
   *pRssi    = replparams.rssi;
   *pLqi     = replparams.lqi;
   *pCrc     = replparams.crc;
   */
}

//=========================== interrupts ======================================

void radio_intr_startOfFrame(OpenMote* self, uint16_t capturedTime) {
   self->radio_icb.startFrame_cb(self, capturedTime);
}

void radio_intr_endOfFrame(OpenMote* self, uint16_t capturedTime) {
   self->radio_icb.endFrame_cb(self, capturedTime);
}

//=========================== private =========================================
