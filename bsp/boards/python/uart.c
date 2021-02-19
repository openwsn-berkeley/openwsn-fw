/**
\brief Python-specific definition of the "uart" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, May 2013.
*/

#include "interface.h"
#include "uart.h"

//=========================== defines =========================================
#define OUTPUT_BUFFER_MASK          0x3FF

//=========================== typedefs ========================================

typedef struct {
    uart_tx_cbt txCb;
    uart_rx_cbt rxCb;
} uart_icb_t;

//=========================== variables =======================================

uart_icb_t uart_icb;

//=========================== prototypes ======================================

void uart_setCallbacks(uart_tx_cbt txCb, uart_rx_cbt rxCb) {

#ifdef TRACE_ON
    printf("uart_setCallbacks (txCb = %p, rxCb = %p)... \n", txCb, rxCb);
#endif

    uart_icb.txCb = txCb;
    uart_icb.rxCb = rxCb;

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

//=========================== public ==========================================

void uart_init(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("uart_init()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_uart_init], NULL);
    if (result == NULL) {
        printf("[CRITICAL] uart_init() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void uart_enableInterrupts(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("uart_enableInterrupts()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_uart_enableInterrupts], NULL);
    if (result == NULL) {
        printf("[CRITICAL] uart_enableInterrupts() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void uart_disableInterrupts(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("uart_disableInterrupts()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_uart_disableInterrupts], NULL);
    if (result == NULL) {
        printf("[CRITICAL] uart_disableInterrupts() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void uart_clearRxInterrupts(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("uart_clearRxInterrupts()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_uart_clearRxInterrupts], NULL);
    if (result == NULL) {
        printf("[CRITICAL] uart_clearRxInterrupts() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void uart_clearTxInterrupts(void) {
    PyObject *result;

#ifdef TRACE_ON
    printf("uart_clearTxInterrupts()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_uart_clearTxInterrupts], NULL);
    if (result == NULL) {
        printf("[CRITICAL] uart_clearTxInterrupts() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

#ifdef FASTSIM
void uart_writeByte(OpenMote* self, uint8_t byteToWrite) {
   printf("[CRITICAL] uart_writeByte() should not be called\r\n");
}
#else

void uart_writeByte(uint8_t byteToWrite) {
    PyObject *result;
    PyObject *arglist;

#ifdef TRACE_ON
    printf("uart_writeByte()... \n");
#endif

    // forward to Python
    arglist = Py_BuildValue("(i)", byteToWrite);
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_uart_writeByte], arglist);
    if (result == NULL) {
        printf("[CRITICAL] uart_writeByte() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);
    Py_DECREF(arglist);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

#endif

void
uart_writeCircularBuffer_FASTSIM(uint8_t *buffer, uint16_t *outputBufIdxR, uint16_t *outputBufIdxW) {
    PyObject *frame;
    PyObject *arglist;
    PyObject *result;
    PyObject *item;
    int res;
    int16_t len;
    uint16_t i;

#ifdef TRACE_ON
    printf("uart_writeCircularBuffer_FASTSIM(buffer = %p, outputBufIdxR = %p, outputBufIdxW = %p)... \n",
           buffer,
           outputBufIdxR,
           outputBufIdxW
    );
#endif

    // forward to Python
    len = (*outputBufIdxW) - (*outputBufIdxR);
    if (len < 0) {
        len = len + OUTPUT_BUFFER_MASK + 1;
    }

    frame = PyList_New(len);
    i = 0;
    while (*outputBufIdxR != *outputBufIdxW) {

        // get element at outputBufIdxR
        item = PyLong_FromLong(buffer[OUTPUT_BUFFER_MASK & *outputBufIdxR]);
        res = PyList_SetItem(frame, i, item);
        if (res != 0) {
            printf("[CRITICAL] uart_writeCircularBuffer_FASTSIM() failed setting list item\r\n");
            return;
        }

        // increment index
        (*outputBufIdxR)++;
        i++;
    }
    arglist = Py_BuildValue("(O)", frame);
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_uart_writeCircularBuffer_FASTSIM], arglist);
    if (result == NULL) {
        printf("[CRITICAL] uart_writeCircularBuffer_FASTSIM() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);
    Py_DECREF(arglist);
    Py_DECREF(frame);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void uart_writeBufferByLen_FASTSIM(uint8_t *buffer, uint16_t len) {
    PyObject *frame;
    PyObject *arglist;
    PyObject *result;
    PyObject *item;
    uint16_t i;
    int res;

#ifdef TRACE_ON
    printf("uart_writeBufferByLen_FASTSIM (buffer = %p, len = %d)... \n", buffer, len);
#endif

    // forward to Python
    frame = PyList_New(len);
    if (frame == NULL) {
        printf("[CRITICAL] PyList_New(%d) failed in uart_writeBufferByLen_FASTSIM\r\n", len);
        return;
    }
    for (i = 0; i < len; i++) {
        item = PyLong_FromLong(buffer[i]);
        res = PyList_SetItem(frame, i, item);
        if (res != 0) {
            printf("[CRITICAL] uart_writeBufferByLen_FASTSIM() failed setting list item\r\n");
            return;
        }
    }
    arglist = Py_BuildValue("(O)", frame);
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_uart_writeBufferByLen_FASTSIM], arglist);
    if (result == NULL) {
        printf("[CRITICAL] uart_writeBufferByLen_FASTSIM() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);
    Py_DECREF(arglist);
    Py_DECREF(frame);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

uint8_t uart_readByte(void) {
    PyObject *result;
    uint8_t returnVal;

#ifdef TRACE_ON
    printf("uart_readByte()... \n");
#endif

    // forward to Python
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_uart_readByte], NULL);
    if (result == NULL) {
        printf("[CRITICAL] uart_readByte() returned NULL\r\n");
        return 0;
    }
    if (!PyLong_Check(result)) {
        printf("[CRITICAL] uart_readByte() returned NULL\r\n");
        return 0;
    }
    returnVal = (uint8_t) PyLong_AsLong(result);

    // dispose of returned value
    Py_DECREF(result);

#ifdef TRACE_ON
    printf("...got %d.\n", returnVal);
#endif

    return returnVal;
}

#ifdef FASTSIM
void uart_setCTS(OpenMote* self, bool state) {
   printf("[CRITICAL] uart_setCTS() should not be called\r\n");
}
#else

void uart_setCTS(bool state) {
    PyObject *result;
    PyObject *arglist;

#ifdef TRACE_ON
    printf("uart_setCTS()... \n");
#endif

    // forward to Python
    arglist = Py_BuildValue("(i)", state);
    result = PyObject_CallObject(callbacks[MOTE_NOTIF_uart_setCTS], arglist);
    if (result == NULL) {
        printf("[CRITICAL] uart_setCTS() returned NULL\r\n");
        return;
    }
    Py_DECREF(result);
    Py_DECREF(arglist);

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

#endif

//=========================== interrupt handlers ==============================

void uart_intr_tx(void) {

#ifdef TRACE_ON
    printf("uart_intr_tx(), calling %p... \n", &uart_icb.txCb);
#endif

    uart_icb.txCb();

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}

void uart_intr_rx(void) {

#ifdef TRACE_ON
    printf("uart_intr_rx(), calling %p... \n", &uart_icb.txCb);
#endif

    uart_icb.rxCb();

#ifdef TRACE_ON
    printf("...done.\n");
#endif
}