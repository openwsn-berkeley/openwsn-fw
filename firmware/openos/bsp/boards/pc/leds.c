/**
\brief PC-specific definition of the "leds" bsp module.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, April 2012.
*/

#include "leds.h"
#include "opensim_proto.h"

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

void leds_init() {
   opensim_requ_leds_init_t requ;
   opensim_repl_leds_init_t repl;
   
   // prepare request
   requ.cmdId = OPENSIM_CMD_leds_init;
   
   // send request to server and get reply
   opensim_client_send((void*)&requ,
                       sizeof(opensim_requ_leds_init_t),
                       (void*)&repl,
                       sizeof(opensim_repl_leds_init_t));
                       
   // make sure server did not return any errors
   if (repl.rc!=OPENSIM_ERR_NONE) {
      printf("ERROR rc=%d for leds_init\n",repl.rc);
   }
}

void    leds_error_on() {
   opensim_requ_leds_error_on_t requ;
   opensim_repl_leds_error_on_t repl;
   
   // prepare request
   requ.cmdId = OPENSIM_CMD_leds_error_on;
   
   // send request to server and get reply
   opensim_client_send((void*)&requ,
                       sizeof(opensim_requ_leds_error_on_t),
                       (void*)&repl,
                       sizeof(opensim_repl_leds_error_on_t));
                       
   // make sure server did not return any errors
   if (repl.rc!=OPENSIM_ERR_NONE) {
      printf("ERROR rc=%d for leds_error_on\n",repl.rc);
   }
}
void    leds_error_off() {
   opensim_requ_leds_error_off_t requ;
   opensim_repl_leds_error_off_t repl;
   
   // prepare request
   requ.cmdId = OPENSIM_CMD_leds_error_off;
   
   // send request to server and get reply
   opensim_client_send((void*)&requ,
                       sizeof(opensim_requ_leds_error_off_t),
                       (void*)&repl,
                       sizeof(opensim_repl_leds_error_off_t));
                       
   // make sure server did not return any errors
   if (repl.rc!=OPENSIM_ERR_NONE) {
      printf("ERROR rc=%d for leds_error_off\n",repl.rc);
   }
}
void    leds_error_toggle() {
   opensim_requ_leds_error_toggle_t requ;
   opensim_repl_leds_error_toggle_t repl;
   
   // prepare request
   requ.cmdId = OPENSIM_CMD_leds_error_toggle;
   
   // send request to server and get reply
   opensim_client_send((void*)&requ,
                       sizeof(opensim_requ_leds_error_toggle_t),
                       (void*)&repl,
                       sizeof(opensim_repl_leds_error_toggle_t));
                       
   // make sure server did not return any errors
   if (repl.rc!=OPENSIM_ERR_NONE) {
      printf("ERROR rc=%d for leds_error_toggle\n",repl.rc);
   }
}
uint8_t leds_error_isOn() {
   opensim_requ_leds_error_isOn_t requ;
   opensim_repl_leds_error_isOn_t repl;
   
   // prepare request
   requ.cmdId = OPENSIM_CMD_leds_error_isOn;
   
   // send request to server and get reply
   opensim_client_send((void*)&requ,
                       sizeof(opensim_requ_leds_error_isOn_t),
                       (void*)&repl,
                       sizeof(opensim_repl_leds_error_isOn_t));
                       
   // make sure server did not return any errors
   if (repl.rc!=OPENSIM_ERR_NONE) {
      printf("ERROR rc=%d for leds_error_isOn\n",repl.rc);
   }
}

void    leds_radio_on() {
   opensim_requ_leds_radio_on_t requ;
   opensim_repl_leds_radio_on_t repl;
   
   // prepare request
   requ.cmdId = OPENSIM_CMD_leds_radio_on;
   
   // send request to server and get reply
   opensim_client_send((void*)&requ,
                       sizeof(opensim_requ_leds_radio_on_t),
                       (void*)&repl,
                       sizeof(opensim_repl_leds_radio_on_t));
                       
   // make sure server did not return any errors
   if (repl.rc!=OPENSIM_ERR_NONE) {
      printf("ERROR rc=%d for leds_radio_on\n",repl.rc);
   }
}
void    leds_radio_off() {
   opensim_requ_leds_radio_off_t requ;
   opensim_repl_leds_radio_off_t repl;
   
   // prepare request
   requ.cmdId = OPENSIM_CMD_leds_radio_off;
   
   // send request to server and get reply
   opensim_client_send((void*)&requ,
                       sizeof(opensim_requ_leds_radio_off_t),
                       (void*)&repl,
                       sizeof(opensim_repl_leds_radio_off_t));
                       
   // make sure server did not return any errors
   if (repl.rc!=OPENSIM_ERR_NONE) {
      printf("ERROR rc=%d for leds_radio_off\n",repl.rc);
   }
}
void    leds_radio_toggle() {
   opensim_requ_leds_radio_toggle_t requ;
   opensim_repl_leds_radio_toggle_t repl;
   
   // prepare request
   requ.cmdId = OPENSIM_CMD_leds_radio_toggle;
   
   // send request to server and get reply
   opensim_client_send((void*)&requ,
                       sizeof(opensim_requ_leds_radio_toggle_t),
                       (void*)&repl,
                       sizeof(opensim_repl_leds_radio_toggle_t));
                       
   // make sure server did not return any errors
   if (repl.rc!=OPENSIM_ERR_NONE) {
      printf("ERROR rc=%d for leds_radio_toggle\n",repl.rc);
   }
}
uint8_t leds_radio_isOn() {
   opensim_requ_leds_radio_isOn_t requ;
   opensim_repl_leds_radio_isOn_t repl;
   
   // prepare request
   requ.cmdId = OPENSIM_CMD_leds_radio_isOn;
   
   // send request to server and get reply
   opensim_client_send((void*)&requ,
                       sizeof(opensim_requ_leds_radio_isOn_t),
                       (void*)&repl,
                       sizeof(opensim_repl_leds_radio_isOn_t));
                       
   // make sure server did not return any errors
   if (repl.rc!=OPENSIM_ERR_NONE) {
      printf("ERROR rc=%d for leds_radio_isOn\n",repl.rc);
   }
}

// green
void    leds_sync_on() {
   opensim_requ_leds_sync_on_t requ;
   opensim_repl_leds_sync_on_t repl;
   
   // prepare request
   requ.cmdId = OPENSIM_CMD_leds_sync_on;
   
   // send request to server and get reply
   opensim_client_send((void*)&requ,
                       sizeof(opensim_requ_leds_sync_on_t),
                       (void*)&repl,
                       sizeof(opensim_repl_leds_sync_on_t));
                       
   // make sure server did not return any errors
   if (repl.rc!=OPENSIM_ERR_NONE) {
      printf("ERROR rc=%d for leds_sync_on\n",repl.rc);
   }
}
void    leds_sync_off() {
   opensim_requ_leds_sync_off_t requ;
   opensim_repl_leds_sync_off_t repl;
   
   // prepare request
   requ.cmdId = OPENSIM_CMD_leds_sync_off;
   
   // send request to server and get reply
   opensim_client_send((void*)&requ,
                       sizeof(opensim_requ_leds_sync_off_t),
                       (void*)&repl,
                       sizeof(opensim_repl_leds_sync_off_t));
                       
   // make sure server did not return any errors
   if (repl.rc!=OPENSIM_ERR_NONE) {
      printf("ERROR rc=%d for leds_sync_off\n",repl.rc);
   }
}
void    leds_sync_toggle() {
   opensim_requ_leds_sync_toggle_t requ;
   opensim_repl_leds_sync_toggle_t repl;
   
   // prepare request
   requ.cmdId = OPENSIM_CMD_leds_sync_toggle;
   
   // send request to server and get reply
   opensim_client_send((void*)&requ,
                       sizeof(opensim_requ_leds_sync_toggle_t),
                       (void*)&repl,
                       sizeof(opensim_repl_leds_sync_toggle_t));
                       
   // make sure server did not return any errors
   if (repl.rc!=OPENSIM_ERR_NONE) {
      printf("ERROR rc=%d for leds_sync_toggle\n",repl.rc);
   }
}
uint8_t leds_sync_isOn() {
   opensim_requ_leds_sync_isOn_t requ;
   opensim_repl_leds_sync_isOn_t repl;
   
   // prepare request
   requ.cmdId = OPENSIM_CMD_leds_sync_isOn;
   
   // send request to server and get reply
   opensim_client_send((void*)&requ,
                       sizeof(opensim_requ_leds_sync_isOn_t),
                       (void*)&repl,
                       sizeof(opensim_repl_leds_sync_isOn_t));
                       
   // make sure server did not return any errors
   if (repl.rc!=OPENSIM_ERR_NONE) {
      printf("ERROR rc=%d for leds_sync_isOn\n",repl.rc);
   }
}

// yellow
void    leds_debug_on() {
   opensim_requ_leds_debug_on_t requ;
   opensim_repl_leds_debug_on_t repl;
   
   // prepare request
   requ.cmdId = OPENSIM_CMD_leds_debug_on;
   
   // send request to server and get reply
   opensim_client_send((void*)&requ,
                       sizeof(opensim_requ_leds_debug_on_t),
                       (void*)&repl,
                       sizeof(opensim_repl_leds_debug_on_t));
                       
   // make sure server did not return any errors
   if (repl.rc!=OPENSIM_ERR_NONE) {
      printf("ERROR rc=%d for leds_debug_on\n",repl.rc);
   }
}
void    leds_debug_off() {
   opensim_requ_leds_debug_off_t requ;
   opensim_repl_leds_debug_off_t repl;
   
   // prepare request
   requ.cmdId = OPENSIM_CMD_leds_debug_off;
   
   // send request to server and get reply
   opensim_client_send((void*)&requ,
                       sizeof(opensim_requ_leds_debug_off_t),
                       (void*)&repl,
                       sizeof(opensim_repl_leds_debug_off_t));
                       
   // make sure server did not return any errors
   if (repl.rc!=OPENSIM_ERR_NONE) {
      printf("ERROR rc=%d for leds_debug_off\n",repl.rc);
   }
}
void    leds_debug_toggle() {
   opensim_requ_leds_debug_toggle_t requ;
   opensim_repl_leds_debug_toggle_t repl;
   
   // prepare request
   requ.cmdId = OPENSIM_CMD_leds_debug_toggle;
   
   // send request to server and get reply
   opensim_client_send((void*)&requ,
                       sizeof(opensim_requ_leds_debug_toggle_t),
                       (void*)&repl,
                       sizeof(opensim_repl_leds_debug_toggle_t));
                       
   // make sure server did not return any errors
   if (repl.rc!=OPENSIM_ERR_NONE) {
      printf("ERROR rc=%d for leds_debug_toggle\n",repl.rc);
   }
}
uint8_t leds_debug_isOn() {
   opensim_requ_leds_debug_isOn_t requ;
   opensim_repl_leds_debug_isOn_t repl;
   
   // prepare request
   requ.cmdId = OPENSIM_CMD_leds_debug_isOn;
   
   // send request to server and get reply
   opensim_client_send((void*)&requ,
                       sizeof(opensim_requ_leds_debug_isOn_t),
                       (void*)&repl,
                       sizeof(opensim_repl_leds_debug_isOn_t));
                       
   // make sure server did not return any errors
   if (repl.rc!=OPENSIM_ERR_NONE) {
      printf("ERROR rc=%d for leds_debug_isOn\n",repl.rc);
   }
}

void leds_all_on() {
   opensim_requ_leds_all_on_t requ;
   opensim_repl_leds_all_on_t repl;
   
   // prepare request
   requ.cmdId = OPENSIM_CMD_leds_all_on;
   
   // send request to server and get reply
   opensim_client_send((void*)&requ,
                       sizeof(opensim_requ_leds_all_on_t),
                       (void*)&repl,
                       sizeof(opensim_repl_leds_all_on_t));
                       
   // make sure server did not return any errors
   if (repl.rc!=OPENSIM_ERR_NONE) {
      printf("ERROR rc=%d for leds_all_on\n",repl.rc);
   }
}
void leds_all_off() {
   opensim_requ_leds_all_off_t requ;
   opensim_repl_leds_all_off_t repl;
   
   // prepare request
   requ.cmdId = OPENSIM_CMD_leds_all_off;
   
   // send request to server and get reply
   opensim_client_send((void*)&requ,
                       sizeof(opensim_requ_leds_all_off_t),
                       (void*)&repl,
                       sizeof(opensim_repl_leds_all_off_t));
                       
   // make sure server did not return any errors
   if (repl.rc!=OPENSIM_ERR_NONE) {
      printf("ERROR rc=%d for leds_all_off\n",repl.rc);
   }
}
void leds_all_toggle() {
   opensim_requ_leds_all_toggle_t requ;
   opensim_repl_leds_all_toggle_t repl;
   
   // prepare request
   requ.cmdId = OPENSIM_CMD_leds_all_toggle;
   
   // send request to server and get reply
   opensim_client_send((void*)&requ,
                       sizeof(opensim_requ_leds_all_toggle_t),
                       (void*)&repl,
                       sizeof(opensim_repl_leds_all_toggle_t));
                       
   // make sure server did not return any errors
   if (repl.rc!=OPENSIM_ERR_NONE) {
      printf("ERROR rc=%d for leds_all_toggle\n",repl.rc);
   }
}

void leds_circular_shift() {
   opensim_requ_leds_circular_shift_t requ;
   opensim_repl_leds_circular_shift_t repl;
   
   // prepare request
   requ.cmdId = OPENSIM_CMD_leds_circular_shift;
   
   // send request to server and get reply
   opensim_client_send((void*)&requ,
                       sizeof(opensim_requ_leds_circular_shift_t),
                       (void*)&repl,
                       sizeof(opensim_repl_leds_circular_shift_t));
                       
   // make sure server did not return any errors
   if (repl.rc!=OPENSIM_ERR_NONE) {
      printf("ERROR rc=%d for leds_circular_shift\n",repl.rc);
   }
}

void leds_increment() {
   opensim_requ_leds_increment_t requ;
   opensim_repl_leds_increment_t repl;
   
   // prepare request
   requ.cmdId = OPENSIM_CMD_leds_increment;
   
   // send request to server and get reply
   opensim_client_send((void*)&requ,
                       sizeof(opensim_requ_leds_increment_t),
                       (void*)&repl,
                       sizeof(opensim_repl_leds_increment_t));
                       
   // make sure server did not return any errors
   if (repl.rc!=OPENSIM_ERR_NONE) {
      printf("ERROR rc=%d for leds_increment\n",repl.rc);
   }
}

//=========================== private =========================================