/**
\brief Python-specific definition of the "security" bsp module.

\author Marcelo Barros <marcelobarrosalmeida@gmail.com>, May 2014.
*/

#include <stdio.h>
#include "openwsn.h"
#include "IEEE802154E.h"
#include "IEEE802154.h"
#include "openqueue.h"
#include "security_obj.h"

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

#define TRACE_ON 1

void security_init(OpenMote* self) {
   PyObject*   result;
   
#ifdef TRACE_ON
   printf("C@0x%x: security_init()... \n",self);
#endif
   
   // forward to Python
   result     = PyObject_CallObject(self->callback[MOTE_NOTIF_security_init],NULL);
   if (result == NULL) {
      printf("[CRITICAL] security_init() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   
#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}

uint8_t security_decrypt(OpenMote* self, OpenQueueEntry_t* msg, ieee802154_sec_hdr_t sec_hdr) {
   PyObject* result;
   PyObject* arglist;
   
#ifdef TRACE_ON
   printf("C@0x%x: security_decrypt()... \n",self);
#endif

   // forward to Python
   arglist = Py_BuildValue("(i)",sec_hdr.sec_ctrl.sec_level);
   result  = PyObject_CallObject(self->callback[MOTE_NOTIF_security_decrypt],arglist);
   if (result == NULL) {
      printf("[CRITICAL] security_decrypt() returned NULL\r\n");
      return;
   }
   Py_DECREF(result);
   Py_DECREF(arglist);

#ifdef TRACE_ON
   printf("C@0x%x: ...done.\n",self);
#endif
}
//=========================== private =========================================