/**
 * Author: Tengfei Chang (tengfei.chang@inria.fr)
 * Date:   December 2017
 * Description: cross-platform definition of the "radio" bsp module.
*/

#include <stdio.h>
#include <string.h>

#include "board_info.h"
#include "radio.h"

//=========================== defines =========================================

//=========================== variables =======================================

radio_functions_t radio_funct[MAX_NUM_RADIOS];

//=========================== public ==========================================

//===== admin
    
void radio_getFunctions(radio_functions_t** radio_function){
    *radio_function = radio_funct;
}