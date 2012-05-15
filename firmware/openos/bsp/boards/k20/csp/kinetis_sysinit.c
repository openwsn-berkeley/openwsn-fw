/*
 *    kinetis_sysinit.c - Default init routines for
 *                     		Kinetis ARM systems
 *    Copyright � 2010 Freescale semiConductor Inc. All Rights Reserved.
 */

#include "kinetis_sysinit.h"
#include "derivative.h"

typedef void (*const tIsrFunc)(void);
typedef struct {
  uint32_t * __ptr;
  tIsrFunc __fun[110];
} tVectorTable;

extern uint32_t __vector_table[];

#pragma overload void __init_hardware();
void __init_hardware()
{
	  SCB_VTOR = (uint32_t)__vector_table; /* Set the interrupt vector table position */
	/*
		Disable the Watchdog because it may reset the core before entering main().
		There are 2 unlock words which shall be provided in sequence before
		accessing the control register.
	*/
	*(volatile unsigned short *)KINETIS_WDOG_UNLOCK_ADDR = KINETIS_WDOG_UNLOCK_SEQ_1;
	*(volatile unsigned short *)KINETIS_WDOG_UNLOCK_ADDR = KINETIS_WDOG_UNLOCK_SEQ_2;
	*(volatile unsigned short *)KINETIS_WDOG_STCTRLH_ADDR = KINETIS_WDOG_DISABLED_CTRL;

}

void isr_default(void)
{
  /* Write your interrupt code here ... */
	while (1);

}
/* end of isr_default */

void isrINT_NMI(void)
{
  /* Write your interrupt code here ... */

}
/* end of isrINT_NMI */

#ifdef __cplusplus
extern "C" {
#endif
extern uint32_t __SP_INIT[];
extern void __thumb_startup( void );
#ifdef __cplusplus
}
#endif

/* Interrupt vector table */
#ifndef UNASSIGNED_ISR
  #define UNASSIGNED_ISR isr_default   /* unassigned interrupt service routine */
#endif

#pragma define_section vectortable ".vectortable" ".vectortable" ".vectortable" far_abs R
static __declspec(vectortable) tVectorTable __vect_table = { /* Interrupt vector table */
   __SP_INIT,                                              /* 0 (0x00000000) (prior: -) */
  {
   (tIsrFunc)__thumb_startup,                              /* 1 (0x00000004) (prior: -) */
   (tIsrFunc)isrINT_NMI,                                   /* 2 (0x00000008) (prior: -2) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 3 (0x0000000C) (prior: -1) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 4 (0x00000010) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 5 (0x00000014) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 6 (0x00000018) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 7 (0x0000001C) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 8 (0x00000020) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 9 (0x00000024) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 10 (0x00000028) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 11 (0x0000002C) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 12 (0x00000030) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 13 (0x00000034) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 14 (0x00000038) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 15 (0x0000003C) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 16 (0x00000040) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 17 (0x00000044) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 18 (0x00000048) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 19 (0x0000004C) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 20 (0x00000050) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 21 (0x00000054) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 22 (0x00000058) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 23 (0x0000005C) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 24 (0x00000060) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 25 (0x00000064) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 26 (0x00000068) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 27 (0x0000006C) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 28 (0x00000070) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 29 (0x00000074) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 30 (0x00000078) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 31 (0x0000007C) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 32 (0x00000080) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 33 (0x00000084) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 34 (0x00000088) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 35 (0x0000008C) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 36 (0x00000090) (prior: -) */
   (tIsrFunc)llwu_isr,                               /* 37 (0x00000094) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 38 (0x00000098) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 39 (0x0000009C) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 40 (0x000000A0) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 41 (0x000000A4) (prior: -) */
   (tIsrFunc)spi_isr,                               /* 42 (0x000000A8) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 43 (0x000000AC) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 44 (0x000000B0) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 45 (0x000000B4) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 46 (0x000000B8) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 47 (0x000000BC) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 48 (0x000000C0) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 49 (0x000000C4) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 50 (0x000000C8) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 51 (0x000000CC) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 52 (0x000000D0) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 53 (0x000000D4) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 54 (0x000000D8) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 55 (0x000000DC) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 56 (0x000000E0) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 57 (0x000000E4) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 58 (0x000000E8) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 59 (0x000000EC) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 60 (0x000000F0) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 61 (0x000000F4) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 62 (0x000000F8) (prior: -) */
   (tIsrFunc)uart_isr,                               /* 63 (0x000000FC) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 64 (0x00000100) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 65 (0x00000104) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 66 (0x00000108) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 67 (0x0000010C) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 68 (0x00000110) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 69 (0x00000114) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 70 (0x00000118) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 71 (0x0000011C) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 72 (0x00000120) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 73 (0x00000124) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 74 (0x00000128) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 75 (0x0000012C) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 76 (0x00000130) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 77 (0x00000134) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 78 (0x00000138) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 79 (0x0000013C) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 80 (0x00000140) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 81 (0x00000144) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 82 (0x00000148) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 83 (0x0000014C) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 84 (0x00000150) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 85 (0x00000154) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 86 (0x00000158) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 87 (0x0000015C) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 88 (0x00000160) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 89 (0x00000164) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 90 (0x00000168) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 91 (0x0000016C) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 92 (0x00000170) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 93 (0x00000174) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 94 (0x00000178) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 95 (0x0000017C) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 96 (0x00000180) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 97 (0x00000184) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 98 (0x00000188) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 99 (0x0000018C) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 100 (0x00000190) (prior: -) */
   (tIsrFunc)lptmr_isr,                               /* 101 (0x00000194) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 102 (0x00000198) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 103 (0x0000019C) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 104 (0x000001A0) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 105 (0x000001A4) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 106 (0x000001A8) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 107 (0x000001AC) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 108 (0x000001B0) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 109 (0x000001B4) (prior: -) */
   (tIsrFunc)UNASSIGNED_ISR,                               /* 110 (0x000001B8) (prior: -) */
  }
};


