/**
\brief Python-specific definition of the "radio" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, May 2013.
*/

#include "radio_python_obj.h"

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================


void radio_python_setFunctions(OpenMote* self, radio_functions_t * funcs){
    
#ifdef TRACE_ON
    printf("C@0x%x: radio_setFunctions(funcs=0x%x)... \n",self,funcs);
#endif

    funcs->radio_powerOn_cb            = radio_python_powerOn;
    // RF admin
    funcs->radio_init_cb               = radio_python_init;
    funcs->radio_setStartFrameCb_cb    = radio_python_setStartFrameCb;
    funcs->radio_setEndFrameCb_cb      = radio_python_setEndFrameCb;
    // RF admin
    funcs->radio_rfOn_cb               = radio_python_rfOn;
    funcs->radio_rfOff_cb              = radio_python_rfOff;
    funcs->radio_setFrequency_cb       = radio_python_setFrequency;
    funcs->radio_change_modulation_cb  = radio_python_change_modulation;
    funcs->radio_change_size_cb        = radio_python_change_size;
    // reset
    funcs->radio_reset_cb              = radio_python_reset;
    // TX
    funcs->radio_loadPacket_prepare_cb = radio_python_loadPacket_prepare;
    funcs->radio_txEnable_cb           = radio_python_txEnable;
    funcs->radio_txNow_cb              = radio_python_txNow;
    funcs->radio_loadPacket_cb         = radio_python_loadPacket;
    // RX
    funcs->radio_rxPacket_prepare_cb   = radio_python_rxPacket_prepare;
    funcs->radio_rxEnable_cb           = radio_python_rxEnable;
    funcs->radio_rxEnable_scum_cb      = radio_python_rxEnable_scum;
    funcs->radio_rxNow_cb              = radio_python_rxNow;
    funcs->radio_getReceivedFrame_cb   = radio_python_getReceivedFrame;
    funcs->radio_getCRCLen_cb          = radio_python_getCRCLen;
    funcs->radio_calculateFrequency_cb = radio_python_calculateFrequency;
    funcs->radio_getDelayTx_cb         = radio_python_getDelayTx;
    funcs->radio_getDelayRx_cb         = radio_python_getDelayRx;
    
#ifdef TRACE_ON
    printf("C@0x%x: ...done.\n",self);
#endif
}


void radio_python_setStartFrameCb(OpenMote* self, radio_capture_cbt cb) {
   
#ifdef TRACE_ON
    printf("C@0x%x: radio_python_setStartFrameCb(cb=0x%x)... \n",self,cb);
#endif
    
    self->radio_icb.startFrame_cb  = cb;
    
#ifdef TRACE_ON
    printf("C@0x%x: ...done.\n",self);
#endif
}

void radio_python_setEndFrameCb(OpenMote* self, radio_capture_cbt cb) {
    
#ifdef TRACE_ON
    printf("C@0x%x: radio_python_setEndFrameCb(cb=0x%x)... \n",self,cb);
#endif
    
    self->radio_icb.endFrame_cb    = cb;
    
#ifdef TRACE_ON
    printf("C@0x%x: ...done.\n",self);
#endif
}

//=========================== public ==========================================

//===== admin

void radio_python_init(OpenMote* self) {
    PyObject*   result;
    
#ifdef TRACE_ON
    printf("C@0x%x: radio_python_init()... \n",self);
#endif
   
    // forward to Python
    result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radio_init],NULL);
    if (result == NULL) {
        printf("[CRITICAL] radio_python_init() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);
    
#ifdef TRACE_ON
    printf("C@0x%x: ...done.\n",self);
#endif
}

//===== reset

void radio_python_reset(OpenMote* self) {
    PyObject*   result;
    
#ifdef TRACE_ON
    printf("C@0x%x: radio_python_reset()... \n");
#endif
    
    // forward to Python
    result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radio_reset],NULL);
    if (result == NULL) {
        printf("[CRITICAL] radio_python_reset() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);
    
#ifdef TRACE_ON
    printf("C@0x%x: ...done.\n",self);
#endif
}

//===== RF admin

void radio_python_setFrequency(OpenMote* self, uint16_t channel_spacing, uint32_t frequency_0, uint16_t channel) {
    PyObject*   result;
    PyObject*   arglist;
    
#ifdef TRACE_ON
    printf("C@0x%x: radio_python_setFrequency(frequency=%d)... \n",self,channel);
#endif
    
    // forward to Python
    arglist    = Py_BuildValue("(i)",channel);
    result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radio_setFrequency],arglist);
    if (result == NULL) {
        printf("[CRITICAL] radio_python_setFrequency() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);
    Py_DECREF(arglist);
    
#ifdef TRACE_ON
    printf("C@0x%x: ...done.\n",self);
#endif
}

void radio_python_rfOn(OpenMote* self) {
    PyObject*   result;
    
#ifdef TRACE_ON
    printf("C@0x%x: radio_python_rfOn()... \n");
#endif
    
    // forward to Python
    result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radio_rfOn],NULL);
    if (result == NULL) {
        printf("[CRITICAL] radio_python_rfOn() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);
    
#ifdef TRACE_ON
    printf("C@0x%x: ...done.\n",self);
#endif
}

void radio_python_rfOff(OpenMote* self) {
    PyObject*   result;
    
#ifdef TRACE_ON
    printf("C@0x%x: radio_python_rfOff()... \n");
#endif
    
    // forward to Python
    result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radio_rfOff],NULL);
    if (result == NULL) {
        printf("[CRITICAL] radio_python_rfOff() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);
    
#ifdef TRACE_ON
    printf("C@0x%x: ...done.\n",self);
#endif
}

//===== TX

void radio_python_loadPacket(OpenMote* self, uint8_t* packet, uint16_t len) {
    PyObject*   pkt;
    PyObject*   arglist;
    PyObject*   result;
    PyObject*   item;
    int8_t      i;
    int         res;
    
#ifdef TRACE_ON
    printf("C@0x%x: radio_python_loadPacket(len=%d)... \n",self,len);
#endif
    
    // forward to Python
    pkt        = PyList_New(len);
    for (i=0;i<len;i++) {
        item    = PyInt_FromLong(packet[i]);
        res     = PyList_SetItem(pkt,i,item);
        if (res!=0) {
            printf("[CRITICAL] radio_python_loadPacket() failed setting list item\r\n");
            return;
        }
    }
    arglist    = Py_BuildValue("(O)",pkt);
    result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radio_loadPacket],arglist);
    if (result == NULL) {
        printf("[CRITICAL] radio_python_loadPacket() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);
    Py_DECREF(arglist);
    Py_DECREF(pkt);
}

void radio_python_txEnable(OpenMote* self) {
    PyObject*   result;
    
#ifdef TRACE_ON
    printf("C@0x%x: radio_python_txEnable()... \n");
#endif
    
    // forward to Python
    result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radio_txEnable],NULL);
    if (result == NULL) {
        printf("[CRITICAL] radio_python_txEnable() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);
    
#ifdef TRACE_ON
    printf("C@0x%x: ...done.\n",self);
#endif
}

void radio_python_txNow(OpenMote* self) {
    PyObject*   result;
    
#ifdef TRACE_ON
    printf("C@0x%x: radio_python_txNow()... \n");
#endif
    
    // forward to Python
    result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radio_txNow],NULL);
    if (result == NULL) {
        printf("[CRITICAL] radio_python_txNow() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);
    
#ifdef TRACE_ON
    printf("C@0x%x: ...done.\n",self);
#endif
}

//===== RX

void radio_python_rxEnable(OpenMote* self) {
    PyObject*   result;
    
#ifdef TRACE_ON
    printf("C@0x%x: radio_python_rxEnable()... \n");
#endif
    
    // forward to Python
    result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radio_rxEnable],NULL);
    if (result == NULL) {
        printf("[CRITICAL] radio_python_rxEnable() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);
    
#ifdef TRACE_ON
    printf("C@0x%x: ...done.\n",self);
#endif
}

void radio_python_rxNow(OpenMote* self) {
    PyObject*   result;
    
#ifdef TRACE_ON
    printf("C@0x%x: radio_python_rxNow()... \n");
#endif
    
    // forward to Python
    result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radio_rxNow],NULL);
    if (result == NULL) {
        printf("[CRITICAL] radio_python_rxNow() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);
    
#ifdef TRACE_ON
    printf("C@0x%x: ...done.\n",self);
#endif
}

void radio_python_getReceivedFrame(OpenMote*   self, 
                            uint8_t*    pBufRead,
                            uint16_t*   pLenRead,
                            uint16_t    maxBufLen,
                            int8_t*     pRssi,
                            uint8_t*    pLqi,
                            bool*       pCrc,
                            uint8_t*    mcs) {
    PyObject*  result;
    PyObject*  item;
    PyObject*  subitem;
    int8_t     lenRead;
    int8_t     i;
    
#ifdef TRACE_ON
    printf("C@0x%x: radio_python_getReceivedFrame()... \n",self);
#endif
    
    // forward to Python
    result     = PyObject_CallObject(self->callback[MOTE_NOTIF_radio_getReceivedFrame],NULL);
    if (result == NULL) {
        printf("[CRITICAL] radio_python_getReceivedFrame() returned NULL\r\n");
        return;
    }
    
    // verify
    if (!PySequence_Check(result)) {
        printf("[CRITICAL] radio_python_getReceivedFrame() did not return a tuple\r\n");
        return;
    }
    if (PyTuple_Size(result)!=4) {
        printf("[CRITICAL] radio_python_getReceivedFrame() did not return a tuple of exactly 4 elements %d\r\n",PyList_Size(result));
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

    //==== item 4: mcs

    item       = PyTuple_GetItem(result,4);
    *mcs       = (uint8_t)PyInt_AsLong(item);
}

uint8_t radio_python_getCRCLen(OpenMote* self){
#ifdef TRACE_ON
    printf("C@0x%x: radio_python_getCRCLen()... returnVal=%d.\n",self,LENGTH_CRC);
#endif
    return LENGTH_CRC;
}

uint8_t radio_python_calculateFrequency(OpenMote* self, uint8_t channelOffset, uint8_t asnOffset, uint8_t numChannels, uint8_t* hopSeq, bool singleChannel){
    
    uint8_t returnVal;
    
#ifdef TRACE_ON
    printf("C@0x%x: radio_python_calculateFrequency()...\n",self);
#endif
    
    if (singleChannel) {
        returnVal = channelOffset; // single channel
    } else {
        // channel hopping enabled, use the channel depending on hopping template
        returnVal = 11 + hopSeq[(asnOffset+channelOffset)%numChannels];
    }
    
#ifdef TRACE_ON
   printf("returnVal=%d.\n",returnVal);
#endif
    return returnVal;
}

uint8_t radio_python_getDelayTx(OpenMote* self){
    
#ifdef TRACE_ON
    printf("C@0x%x: radio_python_getDelayTx is called... returnVal=%d.\n",self,delayTx_2D4GHZ);
#endif
    return delayTx_2D4GHZ;
}

uint8_t radio_python_getDelayRx(OpenMote* self){
    
#ifdef TRACE_ON
    printf("C@0x%x: radio_python_getDelayRx is called... returnVal=%d.\n",self,delayRx_2D4GHZ);
#endif
    return delayRx_2D4GHZ;
}

//=========================== interrupts ======================================

void radio_intr_startOfFrame(OpenMote* self, uint32_t capturedTime) {
    self->radio_icb.startFrame_cb(self, capturedTime);
}

void radio_intr_endOfFrame(OpenMote* self, uint32_t capturedTime) {
    self->radio_icb.endFrame_cb(self, capturedTime);
}

//=========================== private =========================================

// not used
void  radio_python_powerOn(OpenMote* self){}
void  radio_python_change_modulation(OpenMote* self, registerSetting_t * mod){}
void  radio_python_change_size(OpenMote* self, uint16_t* size){}
void  radio_python_loadPacket_prepare(OpenMote* self, uint8_t* packet, uint8_t len){}
void  radio_python_rxPacket_prepare(OpenMote* self){}
void  radio_python_rxEnable_scum(OpenMote* self){}