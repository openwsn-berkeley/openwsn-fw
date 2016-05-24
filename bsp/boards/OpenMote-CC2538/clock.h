/**
 * Author: Xavier Vilajosana (xvilajosana@eecs.berkeley.edu)
 *         Pere Tuset (peretuset@openmote.com)
 * Date:   July 2013
 * Description: CC2538-specific definition of the "board" bsp module.
 */

//=========================== variables =======================================

//=========================== prototypes ======================================

void clock_init(void);
uint32_t clock_get(void);
bool clock_expired(uint32_t future);

//=========================== main ============================================

//=========================== public ==========================================

//=========================== private =========================================

//=========================== interrupt handlers ==============================
