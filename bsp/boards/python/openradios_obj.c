/**
 * Author: Tengfei Chang (tengfei.chang@inria.fr)
 * Date:   December 2017
 * Description: cross-platform definition of the "openradios" driver module.
*/

#include "board_obj.h"

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== public ==========================================

//===== admin

void openradios_getFunctions(OpenMote* self, radio_functions_t** radio_funct){
    *radio_funct = (self->openradios_vars).radio_funct;
}