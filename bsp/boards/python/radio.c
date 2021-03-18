/**
\brief Python-specific definition of the "radio" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, May 2013.
*/

#include "interface.h"
#include "radio.h"

//=========================== defines =========================================

typedef void (*radio_capture_cb_t)(PORT_TIMER_WIDTH timestamp);

typedef struct {
    radio_capture_cb_t startFrame_cb;
    radio_capture_cb_t endFrame_cb;
} radio_icb_t;

radio_icb_t radio_icb;
//=========================== variables =======================================

//=========================== prototypes ======================================


void radio_setStartFrameCb(radio_capture_cbt cb) {

#ifdef TRACE_ON
    printf("radio_setStartFrameCb (cb = %p)... \n", cb);
#endif

    radio_icb.startFrame_cb = cb;

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void radio_setEndFrameCb(radio_capture_cbt cb) {

#ifdef TRACE_ON
    printf("radio_setEndFrameCb (cb = %p)... \n", cb);
#endif

    radio_icb.endFrame_cb = cb;

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

//=========================== public ==========================================

//===== admin

void radio_init(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("radio_init()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_radio_init], NULL);
    if (result == NULL) {
        printf("[CRITICAL] radio_init() returned NULL\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

//===== reset

void radio_reset(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("radio_reset()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_radio_reset], NULL);
    if (result == NULL) {
        printf("[CRITICAL] radio_reset() returned NULL\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

//===== RF admin

void radio_setFrequency(uint8_t frequency, radio_freq_t tx_or_rx) {
    PyObject *result;
    PyObject *arglist;

    (void) tx_or_rx;

#ifdef TRACE_ON
    printf("radio_setFrequency (frequency = %d)... \n", frequency);
#endif

    // forward to Python
    arglist = Py_BuildValue("(i)", frequency);
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_radio_setFrequency], arglist);
    if (result == NULL) {
        printf("[CRITICAL] radio_setFrequency() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);
    Py_DECREF(arglist);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void radio_rfOn(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("radio_rfOn()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_radio_rfOn], NULL);
    if (result == NULL) {
        printf("[CRITICAL] radio_rfOn() returned NULL\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void radio_rfOff(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("radio_rfOff()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_radio_rfOff], NULL);
    if (result == NULL) {
        printf("[CRITICAL] radio_rfOff() returned NULL\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

//===== TX

void radio_loadPacket(const uint8_t *packet, uint16_t len) {
    PyObject *pkt;
    PyObject *arglist;
    PyObject *result;
    PyObject *item;
    uint16_t i;
    int res;

#ifdef TRACE_ON
    printf("radio_loadPacket (len = %d)... \n", len);
#endif

    // forward to Python
    pkt = PyList_New(len);
    for (i = 0; i < len; i++) {
        item = Py_BuildValue("(i)", packet[i]);
        res = PyList_SetItem(pkt, i, item);
        if (res != 0) {
            printf("[CRITICAL] radio_loadPacket() failed setting list item\n");
            return;
        }
    }
    arglist = Py_BuildValue("(O)", pkt);
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_radio_loadPacket], arglist);
    if (result == NULL) {
        printf("[CRITICAL] radio_loadPacket() returned NULL\n");
        return;
    }
    Py_DECREF(result);
    Py_DECREF(arglist);
    Py_DECREF(pkt);
}

void radio_txEnable(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("radio_txEnable()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_radio_txEnable], NULL);
    if (result == NULL) {
        printf("[CRITICAL] radio_txEnable() returned NULL\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void radio_txNow(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("radio_txNow()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_radio_txNow], NULL);
    if (result == NULL) {
        printf("[CRITICAL] radio_txNow() returned NULL\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

//===== RX

void radio_rxEnable(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("radio_rxEnable()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_radio_rxEnable], NULL);
    if (result == NULL) {
        printf("[CRITICAL] radio_rxEnable() returned NULL\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void radio_rxNow(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("radio_rxNow()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_radio_rxNow], NULL);
    if (result == NULL) {
        printf("[CRITICAL] radio_rxNow() returned NULL\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf(" ...done.\n");
#endif
}

void radio_getReceivedFrame(uint8_t *pBufRead,
                            uint8_t *pLenRead,
                            uint8_t maxBufLen,
                            int8_t *pRssi,
                            uint8_t *pLqi,
                            bool *pCrc) {
    PyObject *result;
    PyObject *item;
    PyObject *subitem;
    int8_t lenRead;
    int8_t i;

    (void) maxBufLen;

#ifdef TRACE_ON
    printf("radio_getReceivedFrame() ... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_radio_getReceivedFrame], NULL);
    if (result == NULL) {
        printf("[CRITICAL] radio_getReceivedFrame() returned NULL\n");
        return;
    }

    // verify
    if (!PySequence_Check(result)) {
        printf("[CRITICAL] radio_getReceivedFrame() did not return a tuple\n");
        return;
    }
    if (PyTuple_Size(result) != 4) {
        printf("[CRITICAL] radio_getReceivedFrame() did not return a tuple of exactly 4 elements %ld\n",
               PyList_Size(result));
        return;
    }

    //==== item 0: rxBuffer

    item = PyTuple_GetItem(result, 0);
    lenRead = (int8_t) PyList_Size(item);

    if (lenRead < 0){
        printf("[CRITICAL] radio_getReceivedFrame() got a frame with an illegal length %d", lenRead);
        return;
    }

    *pLenRead = lenRead;
    // store retrieved information
    for (i = 0; i < lenRead; i++) {
        subitem = PyList_GetItem(item, i);
        pBufRead[i] = (uint8_t) PyLong_AsLong(subitem);
    }

    //==== item 1: rssi

    item = PyTuple_GetItem(result, 1);
    *pRssi = (int8_t) PyLong_AsLong(item);

    //==== item 2: lqi

    item = PyTuple_GetItem(result, 2);
    *pLqi = (uint8_t) PyLong_AsLong(item);

    //==== item 3: crc

    item = PyTuple_GetItem(result, 3);
    *pCrc = (uint8_t) PyLong_AsLong(item);

}

//=========================== interrupts ======================================

void radio_intr_startOfFrame(uint32_t capturedTime) {
    radio_icb.startFrame_cb(capturedTime);
}

void radio_intr_endOfFrame(uint32_t capturedTime) {
    radio_icb.endFrame_cb(capturedTime);
}

//=========================== private =========================================
