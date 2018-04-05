/**
 * Author: Tengfei Chang (tengfei.chang@inria.fr)
 * Date:   December 2017
 * Description: cross-platform definition of the "openradios" driver module.
*/

#include "openradios.h"

//=========================== defines =========================================

//=========================== variables =======================================

openradios_vars_t openradios_vars;

//=========================== public ==========================================

//===== admin

void openradios_getFunctions(radio_functions_t** radio_funct){
    *radio_funct = openradios_vars.radio_funct;
}