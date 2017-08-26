\/*
 * info.h
 *
 *  Created on: May 18, 2016
 *      Author: Diogo
 */

#ifndef INFO_MEMORY_H_
#define INFO_MEMORY_H_

#include "stdint.h"

//	Defines & Data Types
///////////////////////////////////////////

#define INFO_SEGMENT_SIZE	128

#define	INFOMEM				0x01800	/* END=0x19FF, size 512 as 4 128-byte segments */
#define INFOA				0x01980	/* END=0x19FF, size 128 */
#define INFOB				0x01900	/* END=0x197F, size 128 */
#define	INFOC				0x01880	/* END=0x18FF, size 128 */
#define	INFOD				0x01800	/* END=0x187F, size 128 */


/////////////				INFO A				/////////////

typedef struct _mrm_info_table{			//128Byte Info-A
	const char		info[32];			// 32Byte	|
	const uint16_t	clk_config[9];		// 18Byte   6
	const uint8_t	options[10];		// 10Byte   4
	      uint32_t	app_guest_address;	//	4Byte	|
/////////////////////////////////////////////
	const uint64_t	ipv6_upper;			// 	8Byte	|
	const uint64_t	eui_64;				//	8Byte	6
	const uint8_t	more_options[48];	// 48Byte	4
	//												|
} mrm_info_table;

/////////////				INFO B				/////////////

/////////////				INFO C				/////////////

/////////////				INFO D				/////////////

#endif /* INFO_MEMORY_H_ */
