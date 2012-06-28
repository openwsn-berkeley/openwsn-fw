/*
 * Note: This file is recreated by the project wizard whenever the MCU is
 *       changed and should not be edited by hand
 */

/* Include the derivative-specific header file */
#include <MK20D7.h>
#define __MK_xxx_H__
#define printf printf_kinetis
#define sprintf sprintf_kinetis

#if (defined MCU_MK20D7) || (defined MCU_MK40D7)
	#define MCGOUTCLK_72_MHZ
#endif

#define BSP_LED2 1 << 9
#define BSP_LED3 1 << 10

