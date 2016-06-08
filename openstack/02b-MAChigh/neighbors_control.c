#include "opendefs.h"
#include "neighbors_control.h"
#include "neighbors.h"
#include "sixtop.h"
#include "scheduler.h"

//=========================== variables =======================================

neighbors_control_vars_t neighbors_control_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

void neighbors_control_init(void) {
    // clear module variables
    memset(&neighbors_control_vars,0,sizeof(neighbors_control_vars_t));
}


//=========================== private =========================================