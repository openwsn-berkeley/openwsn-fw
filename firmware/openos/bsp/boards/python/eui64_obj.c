/**
\brief PC-specific definition of the "eui64" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, April 2012.
*/

#include "eui64_obj.h"

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void eui64_get(OpenMote* self, uint8_t* addressToWrite) {
   //opensim_repl_eui64_get_t replparams;
   
   // send request to server and get reply
   /*
   opensim_client_sendAndWaitForAck(OPENSIM_CMD_eui64_get,
                                    0,
                                    0,
                                    &replparams,
                                    sizeof(opensim_repl_eui64_get_t));
   */
   // TODO: replace by call to Python
                                    
   // copy into addressToWrite
   //memcpy(addressToWrite,replparams.eui64,8);
}

//=========================== private =========================================