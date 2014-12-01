/*
 * File:        common.h
 * Purpose:     File to be included by all project files
 *
 * Notes:
 */

#ifndef _COMMON_H_
#define _COMMON_H_

/********************************************************************/

/*
 * Debug prints ON (#define) or OFF (#undef)
 */
#define DEBUG
#define DEBUG_PRINT

/* 
 * Include the generic CPU header file 
 */
#include "arm_cm4.h"
#include <stdint.h>

/* 
 * Include the platform specific header file 
 */

#define OPENMOTE_K20 1
//#define TOWER_K20 1


#if (defined(TWR_K20D72M))
  #include "tower.h"
#else
  #error "No valid platform defined"
#endif

/* 
 * Include the cpu specific header file 
 */
#if (defined(MCU_MK20DZ72))
	#if (defined(TOWER_K20))
		#include "MK20D7.h"
    #elif (defined (OPENMOTE_K20))
        #include "MK20DZ10.h"
    #endif 
#elif (defined(MCU_MK40DZ72))
  #include "MK51D7.h"
#elif (defined(MCU_MK50DZ72))
  #include "MK51D7.h"
#elif (defined(MCU_MK51DZ72))
  #include "MK51D7.h"
#else
  #error "No valid CPU defined"
#endif


/* 
 * Include any toolchain specfic header files 
 */
#if (defined(CW))
  #include "cw.h"
#elif (defined(IAR))
  #include "iar.h"
#else
#warning "No toolchain specific header included"
#endif

/* 
 * Include common utilities
 */
#include "startup.h"
#include <stdlib.h> 

#if (defined(IAR))
	#include "intrinsics.h"
#endif

/********************************************************************/

#endif /* _COMMON_H_ */
