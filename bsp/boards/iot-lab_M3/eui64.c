/**
\brief iot-lab_M3 definition of the "eui64" bsp module.

\author Alaeddine Weslati <alaeddine.weslati@inria.fr>, January 2014.
*/

#include "string.h"
#include "eui64.h"
//#include "hash.h"

//=========================== defines =========================================
//see http://www.st.com/st-web-ui/static/active/en/resource/technical/document/reference_manual/CD00171190.pdf p1066

// stm32f103rey, 96-bit unique ID address : pointer to 8 first bytes of the 12: 0->8
#define UNIQUE_ID_BASE_ADDRESS          0x1FFFF7E8
// stm32f103rey, 96-bit unique ID address : pointer to 8 last bytes of the 12: 4->12
#define UNIQUE_ID_LAST_ADDRESS          0x1FFFF7EC


//=========================== variables =======================================

const uint8_t const *uid = (const uint8_t *const) UNIQUE_ID_BASE_ADDRESS;
const uint8_t const *luid = (const uint8_t *const) UNIQUE_ID_LAST_ADDRESS;

//=========================== prototypes ======================================

//=========================== public ==========================================

void eui64_get(uint8_t* addressToWrite)
{
  //Fnv64_t bob=fnv_64a_buf(uid, 12, FNV1A_64_INIT);
   
  addressToWrite[0] = uid[7];
  addressToWrite[1] = uid[6];
  addressToWrite[2] = uid[5];
  addressToWrite[3] = uid[4];
  addressToWrite[4] = luid[3];
  addressToWrite[5] = luid[2];
  addressToWrite[6] = luid[1];
  addressToWrite[7] = luid[0];
}

//=========================== private =========================================

//uint8_t * hash(const uint8_t const *uid){
   
//}
