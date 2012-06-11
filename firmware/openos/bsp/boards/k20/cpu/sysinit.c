/*
 * File:        sysinit.c
 * Purpose:     Kinetis Configuration
 *              Initializes processor to a default state
 *
 * Notes:
 *
 */

#include "common.h"
#include "sysinit.h"
#include "uart.h"
#include "mcg.h"

/********************************************************************/

/* Actual system clock frequency */
int mcg_clk_hz;
int mcg_clk_khz;
int core_clk_khz;
int core_clk_mhz;
int periph_clk_khz;

/********************************************************************/
void sysinit (void)
{    uint8_t mode;
/* Enable all of the port clocks. These have to be enabled to configure
 * pin muxing options, so most code will need all of these on anyway.
 */
SIM_SCGC5 |= (SIM_SCGC5_PORTA_MASK
		| SIM_SCGC5_PORTB_MASK
		| SIM_SCGC5_PORTC_MASK
		| SIM_SCGC5_PORTD_MASK
		| SIM_SCGC5_PORTE_MASK );

//sets the main clock at 71.99Mhz using the RTC and FLL.

SIM_SCGC6 |= (uint32_t)0x20000000UL;                       
if ((RTC_CR & RTC_CR_OSCE_MASK) == 0u) { /* Only if the OSCILLATOR is not already enabled */
	/* RTC_CR: SC2P=0,SC4P=0,SC8P=0,SC16P=0 */
	RTC_CR &= (uint32_t)~0x3C00UL;                      
	/* RTC_CR: OSCE=1 */
	RTC_CR |= (uint32_t)0x0100UL;                       
	/* RTC_CR: CLKO=0 */
	RTC_CR &= (uint32_t)~0x0200UL;                      
}
/* Disable the WDOG module */
/* WDOG_UNLOCK: WDOGUNLOCK=0xC520 */
WDOG_UNLOCK = (uint16_t)0xC520U;     /* Key 1 */
/* WDOG_UNLOCK : WDOGUNLOCK=0xD928 */
WDOG_UNLOCK  = (uint16_t)0xD928U;    /* Key 2 */
/* WDOG_STCTRLH: DISTESTWDOG=0,BYTESEL=0,TESTSEL=0,TESTWDOG=0,WAITEN=1,STOPEN=1,DBGEN=0,ALLOWUPDATE=1,WINEN=0,IRQRSTEN=0,CLKSRC=1,WDOGEN=0 */
WDOG_STCTRLH = (uint16_t)0x01D2U;                  
/* System clock initialization */
/* SIM_SCGC5: PORTC=1,PORTA=1 */
SIM_SCGC5 |= (uint32_t)0x0A00UL;     /* Enable clock gate for ports to enable pin routing */
/* SIM_CLKDIV1: OUTDIV1=0,OUTDIV2=1,OUTDIV3=1,OUTDIV4=3 */
SIM_CLKDIV1 = (uint32_t)0x01130000UL; /* Update system prescalers */
/* SIM_SOPT2: PLLFLLSEL=0 */
SIM_SOPT2 &= (uint32_t)~0x00010000UL; /* Select FLL as a clock source for various peripherals */
/* SIM_SOPT1: OSC32KSEL=0 */
SIM_SOPT1 &= (uint32_t)~0x000C0000UL; /* System oscillator drives 32 kHz clock for various peripherals */
/* Switch to FEE Mode */
/* OSC_CR: ERCLKEN=0,EREFSTEN=0,SC2P=0,SC4P=0,SC8P=0,SC16P=0 */
OSC_CR = (uint8_t)0x00U;                             
/* MCG_C7: OSCSEL=1 */
MCG_C7 |= (uint8_t)0x01U;                            
/* MCG_C2: LOCRE0=0,RANGE0=0,HGO0=0,EREFS0=0,LP=0,IRCS=0 */
MCG_C2 = (uint8_t)0x00U;                             
/* MCG_C1: CLKS=0,FRDIV=0,IREFS=0,IRCLKEN=1,IREFSTEN=0 */
MCG_C1 = (uint8_t)0x02U;                             
/* MCG_C4: DMX32=1,DRST_DRS=2 */
MCG_C4 = (uint8_t)((MCG_C4 & (uint8_t)~(uint8_t)0x20U) | (uint8_t)0xC0U);
/* MCG_C5: PLLCLKEN0=0,PLLSTEN0=0,PRDIV0=0 */
MCG_C5 = (uint8_t)0x00U;                             
/* MCG_C6: LOLIE0=0,PLLS=0,CME0=0,VDIV0=0 */
MCG_C6 = (uint8_t)0x00U;                             
while((MCG_S & MCG_S_IREFST_MASK) != 0x00U) { /* Check that the source of the FLL reference clock is the external reference clock. */
}
while((MCG_S & 0x0CU) != 0x00U) {    /* Wait until output of the FLL is selected */
}

mcg_clk_hz=71991296;//this is the main clock speed in hz.
mcg_clk_khz = mcg_clk_hz / 1000;
core_clk_khz = mcg_clk_khz / (((SIM_CLKDIV1 & SIM_CLKDIV1_OUTDIV1_MASK) >> 28)+ 1);
periph_clk_khz = mcg_clk_khz / (((SIM_CLKDIV1 & SIM_CLKDIV1_OUTDIV2_MASK) >> 24)+ 1);

mode= what_mcg_mode();
}
/********************************************************************/
void trace_clk_init(void)
{
	/* Set the trace clock to the core clock frequency */
	SIM_SOPT2 |= SIM_SOPT2_TRACECLKSEL_MASK;

	/* Enable the TRACE_CLKOUT pin function on PTA6 (alt7 function) */
	PORTA_PCR6 = ( PORT_PCR_MUX(0x7)
			| PORT_PCR_DSE_MASK);	// enable high drive strength to support high toggle rate
}
/********************************************************************/
void fb_clk_init(void)
{
	/* Enable the clock to the FlexBus module */
	SIM_SCGC7 |= SIM_SCGC7_FLEXBUS_MASK;

	/* Enable the FB_CLKOUT function on PTC3 (alt5 function) */
	PORTC_PCR3 = ( PORT_PCR_MUX(0x5)
			| PORT_PCR_DSE_MASK);	// enable high drive strength to support high toggle rate
}
/********************************************************************/

