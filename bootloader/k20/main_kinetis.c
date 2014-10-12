/**
\brief Main File
 Based on Freescale demo for k60 bootloader. 

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, June 2012.
 */

#include "types.h"
#include "derivative.h" /* include peripheral declarations */
#include "user_config.h"
#include "RealTimerCounter.h"
#include "Wdt_kinetis.h"
#include "hidef.h"


/*****************************************************************************
 * Global Functions Prototypes
 *****************************************************************************/
#if MAX_TIMER_OBJECTS
	extern uint_8 TimerQInitialize(uint_8 ControllerId);
#endif
extern void TestApp_Init(void);
extern void TestApp_Task(void);

#if (defined MCU_MK20D7) || (defined MCU_MK40D7)
	#define MCGOUTCLK_72_MHZ
#endif

#if (defined MCU_MK60N512VMD100) || (defined MCU_MK53N512CMD100)
	#define BSP_CLOCK_SRC                   (50000000ul)       	// crystal, oscillator freq.
#else
	#define BSP_CLOCK_SRC                   (8000000ul)       	// crystal, oscillator freq.
#endif
#define BSP_REF_CLOCK_SRC               (2000000ul)       		// must be 2-4MHz

#ifdef MCGOUTCLK_72_MHZ
	#define BSP_CORE_DIV                    (1)
	#define BSP_BUS_DIV                     (2)
	#define BSP_FLEXBUS_DIV                 (2)
	#define BSP_FLASH_DIV                   (3)

	// BSP_CLOCK_MUL from interval 24 - 55
	#define BSP_CLOCK_MUL                   (36)    // 72MHz
#else
	#define BSP_CORE_DIV                    (1)
	#define BSP_BUS_DIV                     (1)
	#define BSP_FLEXBUS_DIV                 (1)
	#define BSP_FLASH_DIV                   (2)

	// BSP_CLOCK_MUL from interval 24 - 55
	#define BSP_CLOCK_MUL                   (24)    // 48MHz
#endif

#define BSP_REF_CLOCK_DIV               (BSP_CLOCK_SRC / BSP_REF_CLOCK_SRC)

#define BSP_CLOCK                       (BSP_REF_CLOCK_SRC * BSP_CLOCK_MUL)
#define BSP_CORE_CLOCK                  (BSP_CLOCK / BSP_CORE_DIV)          // CORE CLK, max 100MHz
#define BSP_SYSTEM_CLOCK                (BSP_CORE_CLOCK)                    // SYSTEM CLK, max 100MHz
#define BSP_BUS_CLOCK                   (BSP_CLOCK / BSP_BUS_DIV)       // max 50MHz
#define BSP_FLEXBUS_CLOCK               (BSP_CLOCK / BSP_FLEXBUS_DIV)
#define BSP_FLASH_CLOCK                 (BSP_CLOCK / BSP_FLASH_DIV)     // max 25MHz

#ifdef MCU_MK70F12
enum usbhs_clock
{
  MCGPLL0,
  MCGPLL1,
  MCGFLL,
  PLL1,
  CLKIN
};

// Constants for use in pll_init
#define NO_OSCINIT 0
#define OSCINIT 1

#define OSC_0 0
#define OSC_1 1

#define LOW_POWER 0
#define HIGH_GAIN 1

#define CANNED_OSC  0
#define CRYSTAL 1

#define PLL_0 0
#define PLL_1 1

#define PLL_ONLY 0
#define MCGOUT 1

#define BLPI 1
#define FBI  2
#define FEI  3
#define FEE  4
#define FBE  5
#define BLPE 6
#define PBE  7
#define PEE  8

// IRC defines
#define SLOW_IRC 0
#define FAST_IRC 1

/*
 * Input Clock Info
 */
#define CLK0_FREQ_HZ        50000000
#define CLK0_TYPE           CANNED_OSC

#define CLK1_FREQ_HZ        12000000
#define CLK1_TYPE           CRYSTAL

/* Select Clock source */
/* USBHS Fractional Divider value for 120MHz input */
/* USBHS Clock = PLL0 x (USBHSFRAC+1) / (USBHSDIV+1)       */
#define USBHS_FRAC    0
#define USBHS_DIV     SIM_CLKDIV2_USBHSDIV(1)
#define USBHS_CLOCK   MCGPLL0


/* USB Fractional Divider value for 120MHz input */
/** USB Clock = PLL0 x (FRAC +1) / (DIV+1)       */
/** USB Clock = 120MHz x (1+1) / (4+1) = 48 MHz    */
#define USB_FRAC    SIM_CLKDIV2_USBFSFRAC_MASK
#define USB_DIV     SIM_CLKDIV2_USBFSDIV(4)


/* Select Clock source */
#define USB_CLOCK   MCGPLL0
//#define USB_CLOCK   MCGPLL1
//#define USB_CLOCK   MCGFLL
//#define USB_CLOCK   PLL1
//#define USB_CLOCK   CLKIN

/* The expected PLL output frequency is:
 * PLL out = (((CLKIN/PRDIV) x VDIV) / 2)
 * where the CLKIN can be either CLK0_FREQ_HZ or CLK1_FREQ_HZ.
 * 
 * For more info on PLL initialization refer to the mcg driver files.
 */

#define PLL0_PRDIV      5
#define PLL0_VDIV       24

#define PLL1_PRDIV      5
#define PLL1_VDIV       30
#endif

/*****************************************************************************
 * Local Variables
 *****************************************************************************/
#ifdef MCU_MK70F12
	int mcg_clk_hz;
	int mcg_clk_khz;
	int core_clk_khz;
	int periph_clk_khz;
	int pll_0_clk_khz;
	int pll_1_clk_khz;
#endif

/*****************************************************************************
 * Local Functions Prototypes
 *****************************************************************************/
static void Init_Sys(void);
static int pll_init(
	#ifdef MCU_MK70F12
		unsigned char init_osc, 
		unsigned char osc_select, 
		int crystal_val, 
		unsigned char hgo_val, 
		unsigned char erefs_val, 
		unsigned char pll_select, 
		signed char prdiv_val, 
		signed char vdiv_val, 
		unsigned char mcgout_select
	#endif
		);
#ifdef MCU_MK70F12
	void trace_clk_init(void);
	void fb_clk_init(void);
#endif
static void wdog_disable(void);
static void StartKeyPressSimulationTimer(void);
static void KeyPressSimulationTimerCallback(void);
void GPIO_Init();
#ifdef MCU_MK70F12
	void delay(int delayloop);
#endif

/****************************************************************************
 * Global Variables
 ****************************************************************************/
#if ((defined __CWCC__) || (defined __IAR_SYSTEMS_ICC__) || (defined __CC_ARM))
	extern uint_32 ___VECTOR_RAM[];            // Get vector table that was copied to RAM
#elif defined(__GNUC__)
	extern uint_32 __cs3_interrupt_vector[];
#endif
volatile uint_8 kbi_stat;	   /* Status of the Key Pressed */
#ifdef USE_FEEDBACK_ENDPOINT
	extern uint_32 feedback_data;
#endif

/*****************************************************************************
 * Global Functions
 *****************************************************************************/
/******************************************************************************
 * @name        main
 *
 * @brief       This routine is the starting point of the application
 *
 * @param       None
 *
 * @return      None
 *
 *****************************************************************************
 * This function initializes the system, enables the interrupts and calls the
 * application
 *****************************************************************************/
#ifdef __GNUC__
 int main(void)
#else
 void main(void)
#endif
{
	/* initialize the system */
    Init_Sys();

#ifdef MCU_MK70F12
    sci2_init();
#endif

#ifdef MCU_MK70F12
	#if !HIGH_SPEED_DEVICE // full speed
		/* MPU Configuration */
		MPU_CESR=0;	// MPU is disabled. All accesses from all bus masters are allowed
	
		SIM_SOPT2 |= SIM_SOPT2_PLLFLLSEL(1)       /** PLL0 reference */   
							  |  SIM_SOPT2_USBFSRC(0)         /** MCGPLLCLK as CLKC source */
							  |  SIM_SOPT2_USBF_CLKSEL_MASK;  /** USB fractional divider like USB reference clock */  
		SIM_CLKDIV2 = USB_FRAC | USB_DIV;         /** Divide reference clock to obtain 48MHz */
	
		/* Enable USB-OTG IP clocking */
		SIM_SCGC4 |= SIM_SCGC4_USBFS_MASK;
	#else	// High speed	
		// disable cache
		LMEM_PCCCR &= ~LMEM_PCCCR_ENCACHE_MASK;
		
		/* AXBS_PRS1: ??=0,M7=7,??=0,M6=0,??=0,M5=5,??=0,M4=4,??=0,M3=3,??=0,M2=2,??=0,M1=1,??=0,M0=6 */
		AXBS_PRS1 = (uint32_t)0x70543216UL;
	  
		// Disable the MPU so that USB can access RAM
		MPU_CESR &= ~MPU_CESR_VLD_MASK;
	
		// clock init
		SIM_CLKDIV2 |= USBHS_FRAC | 
				USBHS_DIV;			// Divide reference clock to obtain 60MHz
	
		// MCGPLLCLK for the USB 60MHz CLKC source 
		SIM_SOPT2 |= SIM_SOPT2_USBHSRC(1);
	
		// External 60MHz UPLI Clock
		SIM_SOPT2 |= SIM_SOPT2_USBH_CLKSEL_MASK;

		// enable USBHS clock
		SIM_SCGC6 |= SIM_SCGC6_USB2OTG_MASK;
	
		// use external 60MHz ULPI clock
		SIM_SOPT2 |= (SIM_SOPT2_USBH_CLKSEL_MASK);
	
		// select alternate function 2 for ULPI pins
		SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK;
		PORTA_PCR10 = PORT_PCR_MUX(2);  // Data0
		PORTA_PCR6 = PORT_PCR_MUX(2);	// CLK
		PORTA_PCR11 = PORT_PCR_MUX(2);  // Data1
		PORTA_PCR24 = PORT_PCR_MUX(2);	// Data2
		PORTA_PCR29 = PORT_PCR_MUX(2);	// Data7
		PORTA_PCR25 = PORT_PCR_MUX(2);	// Data3
		PORTA_PCR26 = PORT_PCR_MUX(2);	// Data4
		PORTA_PCR27 = PORT_PCR_MUX(2);	// Data5
		PORTA_PCR28 = PORT_PCR_MUX(2);	// Data6
		PORTA_PCR8 = PORT_PCR_MUX(2);	// NXT
		PORTA_PCR7 = PORT_PCR_MUX(2);	// DIR
		PORTA_PCR9 = PORT_PCR_MUX(2);	// STP

		//delay(100);
	
		// reset ULPI module
		USBHS_USBCMD |= USBHS_USBCMD_RST_MASK;
	
		//delay(100);
	
		// check if USBHS module is ok
		USBHS_ULPI_VIEWPORT = 0x40000000;
		while(USBHS_ULPI_VIEWPORT & (0x40000000));
		#ifdef SERIAL_DEBUG
		#endif
	#endif	// HIGH_SPEED_DEVICE
#endif	// MCU_MK70F12
    
#if !HIGH_SPEED_DEVICE	
	NVICICER2 |= (1<<9);	/* Clear any pending interrupts on USB */
	NVICISER2 |= (1<<9);	/* Enable interrupts from USB module */	
#else // HIGH_SPEED_DEVICE
    NVICICER3 |= 1;	//Clear any pending interrupts on USBHS 
    NVICISER3 |= 1;	//Enable interrupts on USBHS
#endif

    /* SIM Configuration */
    GPIO_Init();

#if MAX_TIMER_OBJECTS
    (void)TimerQInitialize(0);
#endif
    (void)TestApp_Init(); /* Initialize the USB Test Application */

    while(TRUE)
    {
    	Watchdog_Reset();
       /* Call the application task */
       TestApp_Task();
    }

#ifdef __GNUC__
    return 0;
#endif
}

/*****************************************************************************
 * Local Functions
 *****************************************************************************/
/*****************************************************************************
 *
 *    @name     GPIO_Init
 *
 *    @brief    This function Initializes LED GPIO
 *
 *    @param    None
 *
 *    @return   None
 *
 ****************************************************************************
 * Intializes the GPIO
 ***************************************************************************/
void GPIO_Init()
{   
#if(defined MCU_MK20D5) || (defined MCU_MK20D7) || (defined MCU_MK40D7) 
	#if(!(defined MCU_MK20D5)) || (!(defined _USB_BATT_CHG_APP_H_))
		/* Enable clock gating to PORTC */
		SIM_SCGC5 |= SIM_SCGC5_PORTC_MASK;
		
		/* LEDs settings */
		PORTC_PCR9|= PORT_PCR_SRE_MASK    /* Slow slew rate */
				  |  PORT_PCR_ODE_MASK    /* Open Drain Enable */
				  |  PORT_PCR_DSE_MASK    /* High drive strength */
				  ;
		PORTC_PCR9 = PORT_PCR_MUX(1);
		PORTC_PCR10|= PORT_PCR_SRE_MASK    /* Slow slew rate */
				  |  PORT_PCR_ODE_MASK    /* Open Drain Enable */
				  |  PORT_PCR_DSE_MASK    /* High drive strength */
				  ;
		PORTC_PCR10 = PORT_PCR_MUX(1);
		GPIOC_PSOR |= 1 << 9 | 1 << 10;
		GPIOC_PDDR |= 1 << 9 | 1 << 10;
		
		/* Switch buttons settings */    	
		/* Set input PORTC1 */
		PORTC_PCR1 = PORT_PCR_MUX(1);
		GPIOC_PDDR &= ~((uint_32)1 << 1);
		/* Pull up */
		PORTC_PCR1 |= PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
		/* GPIO_INT_EDGE_FALLING */
		PORTC_PCR1 |= PORT_PCR_IRQC(0xA);	
		/* Set input PORTC2 */
		PORTC_PCR2 =  PORT_PCR_MUX(1);
		GPIOC_PDDR &= ~((uint_32)1 << 2);
		/* Pull up */
		PORTC_PCR2 |= PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
		/* GPIO_INT_EDGE_FALLING */
		PORTC_PCR2 |= PORT_PCR_IRQC(0xA);
		/* Enable interrupt */
		PORTC_ISFR |= (1<<1);
		PORTC_ISFR |= (1<<2);
		#ifdef MCU_MK20D5
			NVICICPR1 = 1 << ((42)%32);
			NVICISER1 = 1 << ((42)%32);	
		#else
			NVICICPR2 |= (uint32_t)(1 << ((89)%32));		/* Clear any pending interrupts on PORTC */
			NVICISER2 |= (uint32_t)(1 << ((89)%32));		/* Enable interrupts from PORTC module */ 
		#endif
	#endif
#endif
#if (defined MCU_MK40N512VMD100) ||  (defined MCU_MK53N512CMD100)
    /* Enable clock gating to PORTC */
    SIM_SCGC5 |= SIM_SCGC5_PORTC_MASK;	
	
    /* LEDs settings */
    PORTC_PCR7|= PORT_PCR_SRE_MASK    /* Slow slew rate */
              |  PORT_PCR_ODE_MASK    /* Open Drain Enable */
              |  PORT_PCR_DSE_MASK    /* High drive strength */
              ;
    PORTC_PCR7 = PORT_PCR_MUX(1);
    PORTC_PCR8|= PORT_PCR_SRE_MASK    /* Slow slew rate */
              |  PORT_PCR_ODE_MASK    /* Open Drain Enable */
              |  PORT_PCR_DSE_MASK    /* High drive strength */
              ;
    PORTC_PCR8 = PORT_PCR_MUX(1);
    GPIOC_PSOR |= 1 << 7 | 1 << 8;
    GPIOC_PDDR |= 1 << 7 | 1 << 8;
    
	/* Switch buttons settings */
	/* Set in put PORTC5 */
	PORTC_PCR5 =  PORT_PCR_MUX(1);
	GPIOC_PDDR &= ~((uint_32)1 << 5);
	/* Pull up */
	PORTC_PCR5 |= PORT_PCR_PE_MASK|PORT_PCR_PS_MASK;
	/* GPIO_INT_EDGE_HIGH */
	PORTC_PCR5 |= PORT_PCR_IRQC(9);	
	/* Set in put PORTC13*/
	PORTC_PCR13 =  PORT_PCR_MUX(1);
	GPIOC_PDDR &= ~((uint_32)1 << 13);
	/* Pull up */
	PORTC_PCR13 |= PORT_PCR_PE_MASK|PORT_PCR_PS_MASK;
	/* GPIO_INT_EDGE_HIGH */
	PORTC_PCR13 |= PORT_PCR_IRQC(9);
	/* Enable interrupt */
	PORTC_ISFR |=(1<<5);
	PORTC_ISFR |=(1<<13);
	NVICICPR2 = 1 << ((89)%32);
	NVICISER2 = 1 << ((89)%32);
	
	PORTC_PCR8 =  PORT_PCR_MUX(1);
	GPIOC_PDDR |= 1<<8;
#endif	
#if defined(MCU_MK60N512VMD100)  
	/* Enable clock gating to PORTA and PORTE */
	SIM_SCGC5 |= (SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTE_MASK);
	
	/* LEDs settings */
	PORTA_PCR10 =  PORT_PCR_MUX(1);
	PORTA_PCR11 =  PORT_PCR_MUX(1);
	PORTA_PCR28 =  PORT_PCR_MUX(1);
	PORTA_PCR29 =  PORT_PCR_MUX(1);
	
	GPIOA_PDDR |= (1<<10);
	GPIOA_PDDR |= (1<<11);
	GPIOA_PDDR |= (1<<28);
	GPIOA_PDDR |= (1<<29);
	
	/* Switch buttons settings */
	/* Set input on PORTA pin 19 */
#ifndef _USB_BATT_CHG_APP_H_ 	
	PORTA_PCR19 =  PORT_PCR_MUX(1);
	GPIOA_PDDR &= ~((uint_32)1 << 19);	
	/* Pull up enabled */
	PORTA_PCR19 |= PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;	
	/* GPIO_INT_EDGE_HIGH */
	PORTA_PCR19 |= PORT_PCR_IRQC(9);	
	
	/* Set input on PORTE pin 26 */
	PORTE_PCR26 =  PORT_PCR_MUX(1);
	GPIOE_PDDR &= ~((uint_32)1 << 26);	
	/* Pull up */
	PORTE_PCR26 |= PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;	
	/* GPIO_INT_EDGE_HIGH */
	PORTE_PCR26 |= PORT_PCR_IRQC(9);
#endif
	
	/* Clear interrupt flag */
#ifndef _USB_BATT_CHG_APP_H_ 
	PORTA_ISFR |= (1<<19);
#endif
	PORTE_ISFR |= (1<<26);
	
	/* Enable interrupt port A */
#ifndef _USB_BATT_CHG_APP_H_		
	NVICICPR2 = 1 << ((87)%32);
	NVICISER2 = 1 << ((87)%32);	
#endif	
	
	/* Enable interrupt port E */	
	NVICICPR2 = 1 << ((91)%32);
	NVICISER2 = 1 << ((91)%32);	
#endif	
#ifdef MCU_MK70F12
	/* Enable clock gating to PORTD and PORTE */
	SIM_SCGC5 |= SIM_SCGC5_PORTD_MASK | SIM_SCGC5_PORTE_MASK;
	
	/* Set function to GPIO */
	PORTD_PCR0 = PORT_PCR_MUX(1);
	PORTE_PCR26 = PORT_PCR_MUX(1);
	
	/* Set pin direction to in */
	GPIOD_PDDR &= ~(1<<0);
	GPIOE_PDDR &= ~(1<<26);
	
	/* Enable pull up */
	PORTD_PCR0 |= PORT_PCR_PE_MASK|PORT_PCR_PS_MASK;
	PORTE_PCR26 |= PORT_PCR_PE_MASK|PORT_PCR_PS_MASK;

	/* GPIO_INT_EDGE_HIGH */
	PORTD_PCR0 |= PORT_PCR_IRQC(9);
	PORTE_PCR26 |= PORT_PCR_IRQC(9);

	/* Clear interrupt flag */
	PORTD_ISFR |= (1<<0);
	PORTE_ISFR |= (1<<26);

	/* enable interrupt port D */
	NVICICPR2 |= 1 << ((90)%32);
	NVICISER2 |= 1 << ((90)%32);

	/* enable interrupt port E */
	NVICICPR2 |= 1 << ((91)%32);
	NVICISER2 |= 1 << ((91)%32);
#endif	// MCU_MK70F12
}

/******************************************************************************
 * @name       all_led_off
 *
 * @brief      Switch OFF all LEDs on Board
 *
 * @param	   None
 *
 * @return     None
 *
 *****************************************************************************
 * This function switch OFF all LEDs on Board
 *****************************************************************************/
static void all_led_off(void)
{
	#if defined (MCU_MK40N512VMD100)
		GPIOC_PSOR |= 1 << 7 | 1 << 8 | 1 << 9;
	#elif defined (MCU_MK53N512CMD100)
		GPIOC_PSOR |= 1 << 7 | 1 << 8;
	#elif defined (MCU_MK60N512VMD100) 
		GPIOA_PSOR |= 1 << 10 | 1 << 11 | 1 << 28 | 1 << 29;
	#elif (defined MCU_MK20D5) || (defined MCU_MK20D7) || (defined MCU_MK40D7)
		GPIOC_PSOR |= 1 << 9 | 1 << 10;
	#endif
}

/******************************************************************************
 * @name       display_led
 *
 * @brief      Displays 8bit value on Board LEDs
 *
 * @param	   val
 *
 * @return     None
 *
 *****************************************************************************
 * This function displays 8 bit value on Board LED
 *****************************************************************************/
void display_led(uint_8 val)
{
    uint_8 i = 0;
	UNUSED(i);
    all_led_off();

	#if defined (MCU_MK40N512VMD100)
    	val &= 0x07;
        if(val & 0x1)
    		GPIOC_PCOR |= 1 << 7;
    	if(val & 0x2)
    		GPIOC_PCOR |= 1 << 8;
    	if(val & 0x4)
    		GPIOC_PCOR |= 1 << 9; 
	#elif defined (MCU_MK53N512CMD100)
		val &= 0x03;
	    if(val & 0x1)
			GPIOC_PCOR |= 1 << 7;
		if(val & 0x2)
			GPIOC_PCOR |= 1 << 8;
	#elif defined (MCU_MK60N512VMD100) 
		val &= 0x0F;
	    if(val & 0x1)
			GPIOA_PCOR |= 1 << 11;
		if(val & 0x2)
			GPIOA_PCOR |= 1 << 28;		
	    if(val & 0x4)
			GPIOA_PCOR |= 1 << 29;
		if(val & 0x8)
			GPIOA_PCOR |= 1 << 10;		
	#elif (defined MCU_MK20D5) || (defined MCU_MK20D7) || (defined MCU_MK40D7)
		val &= 0x03; 
	    if(val & 0x1)
			GPIOC_PCOR |= 1 << 9;
		if(val & 0x2)
			GPIOC_PCOR |= 1 << 10;		
	#endif
}
/*****************************************************************************
 *
 *    @name     Init_Sys
 *
 *    @brief    This function Initializes the system
 *
 *    @param    None
 *
 *    @return   None
 *
 ****************************************************************************
 * Initializes the MCU, MCG, KBI, RTC modules
 ***************************************************************************/
static void Init_Sys(void)
{
	/* Point the VTOR to the new copy of the vector table */
#if (defined(__CWCC__) || defined(__IAR_SYSTEMS_ICC__) || defined(__CC_ARM))
	SCB_VTOR = (uint_32)___VECTOR_RAM;
#elif defined(__GNUC__)
	SCB_VTOR = (uint_32)__cs3_interrupt_vector;
#endif
#ifndef MCU_MK70F12												
    /* SIM Configuration */
	pll_init();
	#if !(defined MCU_MK20D5) && !(defined MCU_MK20D7) && !(defined MCU_MK40D7) 
		MPU_CESR=0x00;
	#endif
	
    /************* USB Part **********************/
    /*********************************************/   
    SIM_CLKDIV2 &= (uint32_t)(~(SIM_CLKDIV2_USBFRAC_MASK | SIM_CLKDIV2_USBDIV_MASK));
	#ifdef MCGOUTCLK_72_MHZ
		/* Configure USBFRAC = 0, USBDIV = 0 => frq(USBout) = 2 / 3 * frq(PLLin) */
		SIM_CLKDIV2 = SIM_CLKDIV2_USBDIV(2) | SIM_CLKDIV2_USBFRAC_MASK;
	#else
		/* Configure USBFRAC = 0, USBDIV = 0 => frq(USBout) = 1 / 1 * frq(PLLin) */
		SIM_CLKDIV2 = SIM_CLKDIV2_USBDIV(0);
	#endif
          
    /* Enable USB-OTG IP clocking */
    SIM_SCGC4 |= (SIM_SCGC4_USBOTG_MASK);          
    
    /* Configure USB to be clocked from PLL */
    SIM_SOPT2  |= (SIM_SOPT2_USBSRC_MASK | SIM_SOPT2_PLLFLLSEL_MASK);
    
    /* Configure enable USB regulator for device */
    SIM_SOPT1 |= SIM_SOPT1_USBREGEN_MASK;
#else
    // K70 initialization
    /*
     * Enable all of the port clocks. These have to be enabled to configure
     * pin muxing options, so most code will need all of these on anyway.
     */
    SIM_SCGC5 |= (SIM_SCGC5_PORTA_MASK
    		| SIM_SCGC5_PORTB_MASK
    		| SIM_SCGC5_PORTC_MASK
    		| SIM_SCGC5_PORTD_MASK
    		| SIM_SCGC5_PORTE_MASK 
    		| SIM_SCGC5_PORTF_MASK );

    // releases hold with ACKISO:  Only has an effect if recovering from VLLS1, VLLS2, or VLLS3
    // if ACKISO is set you must clear ackiso before calling pll_init 
    //    or pll init hangs waiting for OSC to initialize
    // if osc enabled in low power modes - enable it first before ack
    // if I/O needs to be maintained without glitches enable outputs and modules first before ack.
    if (PMC_REGSC &  PMC_REGSC_ACKISO_MASK)
    	PMC_REGSC |= PMC_REGSC_ACKISO_MASK;

#if defined(NO_PLL_INIT)
    mcg_clk_hz = 21000000; //FEI mode
#elif defined (ASYNCH_MODE)  
    /* Set the system dividers */
    /* NOTE: The PLL init will not configure the system clock dividers,
     * so they must be configured appropriately before calling the PLL
     * init function to ensure that clocks remain in valid ranges.
     */  
    SIM_CLKDIV1 = ( 0
    		| SIM_CLKDIV1_OUTDIV1(0)
    		| SIM_CLKDIV1_OUTDIV2(1)
    		| SIM_CLKDIV1_OUTDIV3(1)
    		| SIM_CLKDIV1_OUTDIV4(5) );

    /* Initialize PLL0 */
    /* PLL0 will be the source for MCG CLKOUT so the core, system, FlexBus, and flash clocks are derived from it */ 
    mcg_clk_hz = pll_init(OSCINIT,   /* Initialize the oscillator circuit */
    		OSC_0,     /* Use CLKIN0 as the input clock */
    		CLK0_FREQ_HZ,  /* CLKIN0 frequency */
    		LOW_POWER,     /* Set the oscillator for low power mode */
    		CLK0_TYPE,     /* Crystal or canned oscillator clock input */
    		PLL_0,         /* PLL to initialize, in this case PLL0 */
    		PLL0_PRDIV,    /* PLL predivider value */
    		PLL0_VDIV,     /* PLL multiplier */
    		MCGOUT);       /* Use the output from this PLL as the MCGOUT */

    /* Check the value returned from pll_init() to make sure there wasn't an error */
    if (mcg_clk_hz < 0x100)
    	while(1);

    /* Initialize PLL1 */
    /* PLL1 will be the source for the DDR controller, but NOT the MCGOUT */   
    pll_1_clk_khz = (pll_init(NO_OSCINIT, /* Don't init the osc circuit, already done */
    		OSC_0,      /* Use CLKIN0 as the input clock */
    		CLK0_FREQ_HZ,  /* CLKIN0 frequency */
    		LOW_POWER,     /* Set the oscillator for low power mode */
    		CLK0_TYPE,     /* Crystal or canned oscillator clock input */
    		PLL_1,         /* PLL to initialize, in this case PLL1 */
    		PLL1_PRDIV,    /* PLL predivider value */
    		PLL1_VDIV,     /* PLL multiplier */
    		PLL_ONLY) / 1000);   /* Don't use the output from this PLL as the MCGOUT */

    /* Check the value returned from pll_init() to make sure there wasn't an error */
    if ((pll_1_clk_khz * 1000) < 0x100)
    	while(1);

    pll_0_clk_khz = mcg_clk_hz / 1000;       

#elif defined (SYNCH_MODE)  
    /* Set the system dividers */
    /* NOTE: The PLL init will not configure the system clock dividers,
     * so they must be configured appropriately before calling the PLL
     * init function to ensure that clocks remain in valid ranges.
     */  
    SIM_CLKDIV1 = ( 0
    		| SIM_CLKDIV1_OUTDIV1(0)
    		| SIM_CLKDIV1_OUTDIV2(2)
    		| SIM_CLKDIV1_OUTDIV3(2)
    		| SIM_CLKDIV1_OUTDIV4(5) );

    /* Initialize PLL1 */
    /* PLL1 will be the source MCGOUT and the DDR controller */   
    mcg_clk_hz = pll_init(OSCINIT, /* Don't init the osc circuit, already done */
    		OSC_0,      /* Use CLKIN0 as the input clock */
    		CLK0_FREQ_HZ,  /* CLKIN0 frequency */
    		LOW_POWER,     /* Set the oscillator for low power mode */
    		CLK0_TYPE,     /* Crystal or canned oscillator clock input */
    		PLL_1,         /* PLL to initialize, in this case PLL1 */
    		PLL1_PRDIV,    /* PLL predivider value */
    		PLL1_VDIV,     /* PLL multiplier */
    		MCGOUT);   /* Don't use the output from this PLL as the MCGOUT */

    /* Check the value returned from pll_init() to make sure there wasn't an error */
    if (mcg_clk_hz < 0x100)
    	while(1);

    /* Initialize PLL0 */
    /* PLL0 is initialized, but not used as the MCGOUT */ 
    pll_0_clk_khz = (pll_init(NO_OSCINIT,   /* Initialize the oscillator circuit */
    		OSC_0,     /* Use CLKIN0 as the input clock */
    		CLK0_FREQ_HZ,  /* CLKIN0 frequency */
    		LOW_POWER,     /* Set the oscillator for low power mode */
    		CLK0_TYPE,     /* Crystal or canned oscillator clock input */
    		PLL_0,         /* PLL to initialize, in this case PLL0 */
    		PLL0_PRDIV,    /* PLL predivider value */
    		PLL0_VDIV,     /* PLL multiplier */
    		PLL_ONLY) / 1000);       /* Use the output from this PLL as the MCGOUT */

    /* Check the value returned from pll_init() to make sure there wasn't an error */
    if ((pll_0_clk_khz * 1000) < 0x100)
    	while(1);

    pll_1_clk_khz = mcg_clk_hz / 1000;       

#else
#error "A PLL configuration for this platform is NOT defined"
#endif


    /*
     * Use the value obtained from the pll_init function to define variables
     * for the core clock in kHz and also the peripheral clock. These
     * variables can be used by other functions that need awareness of the
     * system frequency.
     */
    mcg_clk_khz = mcg_clk_hz / 1000;
    core_clk_khz = mcg_clk_khz / (((SIM_CLKDIV1 & SIM_CLKDIV1_OUTDIV1_MASK) >> 28)+ 1);
    periph_clk_khz = mcg_clk_khz / (((SIM_CLKDIV1 & SIM_CLKDIV1_OUTDIV2_MASK) >> 24)+ 1);

    /* For debugging purposes, enable the trace clock and/or FB_CLK so that
     * we'll be able to monitor clocks and know the PLL is at the frequency
     * that we expect.
     */
    trace_clk_init();
    fb_clk_init();

    /* Initialize the DDR if the project option if defined */
#ifdef DDR_INIT
    twr_ddr2_script_init();
#endif
#endif
}

#ifdef MCU_MK70F12
/********************************************************************/
void trace_clk_init(void)
{
	/* Set the trace clock to the core clock frequency */
	SIM_SOPT2 |= SIM_SOPT2_TRACECLKSEL_MASK;

	/* Enable the TRACE_CLKOUT pin function on PTF23 (alt6 function) */
	PORTF_PCR23 = ( PORT_PCR_MUX(0x6));
}

/********************************************************************/
void fb_clk_init(void)
{
	/* Enable the clock to the FlexBus module */
	SIM_SCGC7 |= SIM_SCGC7_FLEXBUS_MASK;

	/* Enable the FB_CLKOUT function on PTC3 (alt5 function) */
	PORTC_PCR3 = ( PORT_PCR_MUX(0x5));
}
#endif // MCU_MK70F12


/******************************************************************************
*   @name        IRQ_ISR_PORTA
*
*   @brief       Service interrupt routine of IRQ
*
*   @return      None
*
*   @comment	 
*    
*******************************************************************************/
//#ifndef _USB_BATT_CHG_APP_H_
void IRQ_ISR_PORTA(void)
{
	#if defined(MCU_MK20D5)
		NVICICPR1 |= 1 << ((40)%32);
		NVICISER1 |= 1 << ((40)%32);		
	#else
		NVICICPR2 |= 1 << ((87)%32);
		NVICISER2 |= 1 << ((87)%32);
	#endif
	
	DisableInterrupts;
	if(PORTA_ISFR & (1<<19))
	{
		kbi_stat |= 0x02; 				/* Update the kbi state */
		PORTA_ISFR |=(1<<19);			/* Clear the bit by writing a 1 to it */
	}
	EnableInterrupts;
}
//#endif	// _USB_BATT_CHG_APP_H_

/******************************************************************************
*   @name        IRQ_ISR
*
*   @brief       Service interrupt routine of IRQ
*
*   @return      None
*
*   @comment	 
*    
*******************************************************************************/
//#ifndef MCU_MK70F12
	#if (!(defined MCU_MK20D5)) || (!(defined _USB_BATT_CHG_APP_H_))
void IRQ_ISR_PORTC(void)
	{
		#if defined(MCU_MK20D5)
			NVICICPR1 |= (uint32_t)(1 << ((42)%32));
			NVICISER1 |= (uint32_t)(1 << ((42)%32));		
		#else
			NVICICPR2 |= (uint32_t)(1 << ((89)%32));		/* Clear any pending interrupt on PORTC */
			NVICISER2 |= (uint32_t)(1 << ((89)%32));		/* Set interrupt on PORTC */
		#endif
			
		DisableInterrupts;
	#if(defined MCU_MK20D5) || (defined MCU_MK20D7) || (defined MCU_MK40D7)
		if(PORTC_ISFR & (1<<1))
	#else
		if(PORTC_ISFR & (1<<5))
	#endif	
		{
			kbi_stat |= 0x02; 				/* Update the kbi state */
			
			/* Clear the bit by writing a 1 to it */
		#if(defined MCU_MK20D5) || (defined MCU_MK20D7) || (defined MCU_MK40D7)
			PORTC_ISFR |=(1<<1);
		#else
			PORTC_ISFR |=(1<<5);
		#endif							
		}
		
	#if(defined MCU_MK20D5) || (defined MCU_MK20D7) || (defined MCU_MK40D7)
		if(PORTC_ISFR & (1<<2))
	#else
		if(PORTC_ISFR & (1<<13))
	#endif	
		{
			kbi_stat |= 0x08;				/* Update the kbi state */
			
			/* Clear the bit by writing a 1 to it */
			#if(defined MCU_MK20D5) || (defined MCU_MK20D7) || (defined MCU_MK40D7)
				PORTC_ISFR |=(1<<2);
			#else
				PORTC_ISFR |=(1<<13);
			#endif				
		}	
		EnableInterrupts;
	 Switch_mode();
	}
	#endif
//#endif

/******************************************************************************
*   @name        IRQ_ISR
*
*   @brief       Service interrupt routine of IRQ
*
*   @return      None
*
*   @comment	 
*    
*******************************************************************************/
void IRQ_ISR_PORTD(void)
{
	#ifdef MCU_MK20D5
		NVICICPR1 |= 1 << ((43)%32);
		NVICISER1 |= 1 << ((43)%32);	
	#else
		NVICICPR2 |= 1 << ((90)%32);
		NVICISER2 |= 1 << ((90)%32);
	#endif	
	
	DisableInterrupts;
	if(PORTD_ISFR & (1<<0))
	{		
		/* Update the kbi state */
		kbi_stat |= 0x02;	// right click
		PORTD_ISFR |= (1<<0);			/* Clear the bit by writing a 1 to it */
	}
	EnableInterrupts;
}

/******************************************************************************
*   @name        IRQ_ISR_PORTE
*
*   @brief       Service interrupt routine of IRQ
*
*   @return      None
*
*   @comment	 
*    
*******************************************************************************/
void IRQ_ISR_PORTE(void)
{
	#ifdef MCU_MK20D5
		NVICICPR1 |= 1 << ((44)%32);
		NVICISER1 |= 1 << ((44)%32);	
	#else	
		NVICICPR2 |= 1 << ((91)%32);
		NVICISER2 |= 1 << ((91)%32);	
	#endif
	DisableInterrupts;
	if(PORTE_ISFR & (1<<26))
	{
		/* Update the kbi state */
#ifdef MCU_MK70F12
		kbi_stat |= 0x01;	// left click
#else
		kbi_stat |= 0x08;	// move pointer down
#endif
		PORTE_ISFR |= (1<<26);			/* Clear the bit by writing a 1 to it */
	}
	EnableInterrupts;
}

/******************************************************************************
*   @name        IRQ_ISR_PORTF
*
*   @brief       Service interrupt routine of IRQ
*
*   @return      None
*
*   @comment	 
*    
*******************************************************************************/
#ifdef MCU_MK70F12
void IRQ_ISR_PORTF(void)
{
}
#endif

/*****************************************************************************
 * @name     wdog_disable
 *
 * @brief:   Disable watchdog.
 *
 * @param  : None
 *
 * @return : None
 *****************************************************************************
 * It will disable watchdog.
  ****************************************************************************/
static void wdog_disable(void)
{
	/* Write 0xC520 to the unlock register */
	WDOG_UNLOCK = 0xC520;
	
	/* Followed by 0xD928 to complete the unlock */
	WDOG_UNLOCK = 0xD928;
	
	/* Clear the WDOGEN bit to disable the watchdog */
	WDOG_STCTRLH &= ~WDOG_STCTRLH_WDOGEN_MASK;
}

/*****************************************************************************
 * @name     pll_init
 *
 * @brief:   Initialization of the MCU.
 *
 * @param  : None
 *
 * @return : None
 *****************************************************************************
 * It will configure the MCU to disable STOP and COP Modules.
 * It also set the MCG configuration and bus clock frequency.
 ****************************************************************************/
static int pll_init(
#ifdef MCU_MK70F12
	unsigned char init_osc, 
	unsigned char osc_select, 
	int crystal_val, 
	unsigned char hgo_val, 
	unsigned char erefs_val, 
	unsigned char pll_select, 
	signed char prdiv_val, 
	signed char vdiv_val, 
	unsigned char mcgout_select
#endif
		)
{
#ifndef MCU_MK70F12
    /* 
     * First move to FBE mode
     * Enable external oscillator, RANGE=0, HGO=, EREFS=, LP=, IRCS=
    */
	#if ((defined MCU_MK60N512VMD100) || (defined MCU_MK53N512CMD100))
		MCG_C2 = 0;
	#else
		#if(defined MCU_MK20D5) || (defined MCU_MK20D7) || (defined MCU_MK40D7)
			MCG_C2 = MCG_C2_RANGE0(2) | MCG_C2_HGO0_MASK | MCG_C2_EREFS0_MASK | MCG_C2_IRCS_MASK;
		#else
			MCG_C2 = MCG_C2_RANGE(2) | MCG_C2_HGO_MASK | MCG_C2_EREFS_MASK|MCG_C2_IRCS_MASK;
		#endif
	#endif

    /* Select external oscillator and Reference Divider and clear IREFS 
     * to start external oscillator
     * CLKS = 2, FRDIV = 3, IREFS = 0, IRCLKEN = 0, IREFSTEN = 0
     */
    MCG_C1 = MCG_C1_CLKS(2) | MCG_C1_FRDIV(3);
    
	/* Wait for oscillator to initialize */
	#if((defined MCU_MK20D5) || (defined MCU_MK20D7) || (defined MCU_MK40D7))
		while (!(MCG_S & MCG_S_OSCINIT0_MASK)){};
	#elif defined (MCU_MK40N512VMD100)
		while (!(MCG_S & MCG_S_OSCINIT_MASK)){};
	#endif    
      
	#ifndef MCU_MK20D5
    	/* Wait for Reference Clock Status bit to clear */
    	while (MCG_S & MCG_S_IREFST_MASK) {};
	#endif
    
    /* Wait for clock status bits to show clock source 
     * is external reference clock */
    while (((MCG_S & MCG_S_CLKST_MASK) >> MCG_S_CLKST_SHIFT) != 0x2) {};
    
    /* Now in FBE
     * Configure PLL Reference Divider, PLLCLKEN = 0, PLLSTEN = 0, PRDIV = 0x18
     * The crystal frequency is used to select the PRDIV value. 
     * Only even frequency crystals are supported
     * that will produce a 2MHz reference clock to the PLL.
     */
	#if(defined MCU_MK20D5) || (defined MCU_MK20D7) || (defined MCU_MK40D7)
    	MCG_C5 = MCG_C5_PRDIV0(BSP_REF_CLOCK_DIV - 1) | MCG_C5_PLLCLKEN0_MASK;
	#else
    	MCG_C5 = MCG_C5_PRDIV(BSP_REF_CLOCK_DIV - 1);
	#endif
    	
    /* Ensure MCG_C6 is at the reset default of 0. LOLIE disabled, 
     * PLL disabled, clock monitor disabled, PLL VCO divider is clear
     */
    MCG_C6 = 0;

    
    /* Calculate mask for System Clock Divider Register 1 SIM_CLKDIV1 */
	#if (defined MCU_MK20D5) || (defined MCU_MK40D7)
		SIM_CLKDIV1 =   SIM_CLKDIV1_OUTDIV1(BSP_CORE_DIV - 1) | 	/* core/system clock */
						SIM_CLKDIV1_OUTDIV2(BSP_BUS_DIV - 1)  | 	/* peripheral clock; */
						SIM_CLKDIV1_OUTDIV4(BSP_FLASH_DIV - 1);     /* flash clock */
	#else    
		SIM_CLKDIV1 =  	SIM_CLKDIV1_OUTDIV1(BSP_CORE_DIV    - 1) |
						SIM_CLKDIV1_OUTDIV2(BSP_BUS_DIV     - 1) |
						SIM_CLKDIV1_OUTDIV3(BSP_FLEXBUS_DIV - 1) |
						SIM_CLKDIV1_OUTDIV4(BSP_FLASH_DIV   - 1);
	#endif
    
   /* Set the VCO divider and enable the PLL, 
     * LOLIE = 0, PLLS = 1, CME = 0, VDIV = 2MHz * BSP_CLOCK_MUL
     */
	#if(defined MCU_MK20D5) || (defined MCU_MK20D7) || (defined MCU_MK40D7)
		MCG_C6 = MCG_C6_PLLS_MASK | MCG_C6_VDIV0(BSP_CLOCK_MUL - 24);
	#else
		MCG_C6 = MCG_C6_PLLS_MASK | MCG_C6_VDIV(BSP_CLOCK_MUL - 24);
	#endif
		
    /* Wait for PLL status bit to set */
    while (!(MCG_S & MCG_S_PLLST_MASK)) {};
    
    /* Wait for LOCK bit to set */
	#if(defined MCU_MK20D5) || (defined MCU_MK20D7) || (defined MCU_MK40D7)
		while (!(MCG_S & MCG_S_LOCK0_MASK)){}; 	
	#else    
		while (!(MCG_S & MCG_S_LOCK_MASK)) {};
	#endif
    
    /* Now running PBE Mode */

    /* Transition into PEE by setting CLKS to 0
     * CLKS=0, FRDIV=3, IREFS=0, IRCLKEN=0, IREFSTEN=0
     */
    MCG_C1 &= ~MCG_C1_CLKS_MASK;

    /* Wait for clock status bits to update */
    while (((MCG_S & MCG_S_CLKST_MASK) >> MCG_S_CLKST_SHIFT) != 0x3) {};

	#if(defined MCU_MK20D5)
    	/* Enable the ER clock of oscillators */
    	OSC0_CR = OSC_CR_ERCLKEN_MASK | OSC_CR_EREFSTEN_MASK;    
	#else
    	/* Enable the ER clock of oscillators */
    	OSC_CR = OSC_CR_ERCLKEN_MASK | OSC_CR_EREFSTEN_MASK;       
    #endif
    
    /* Now running PEE Mode */
    return 0;
#else
    unsigned char frdiv_val;
    unsigned char temp_reg;
    unsigned char prdiv, vdiv;
    short i;
    int ref_freq;
    int pll_freq;

    // If using the PLL as MCG_OUT must check if the MCG is in FEI mode first
    if (mcgout_select)
    {
    	// check if in FEI mode
    	if (!((((MCG_S & MCG_S_CLKST_MASK) >> MCG_S_CLKST_SHIFT) == 0x0) && // check CLKS mux has selcted FLL output
    			(MCG_S & MCG_S_IREFST_MASK) &&                                  // check FLL ref is internal ref clk
    			(!(MCG_S & MCG_S_PLLST_MASK))))                                 // check PLLS mux has selected FLL
    	{
    		return 0x1;                                                     // return error code
    	}
    } // if (mcgout_select)

    // Check if OSC1 is being used as a reference for the MCGOUT PLL
    // This requires a more complicated MCG configuration.
    // At this time (Sept 8th 2011) this driver does not support this option
    if (osc_select && mcgout_select)
    {
    	return 0x80; // Driver does not support using OSC1 as the PLL reference for the system clock on MCGOUT
    }

    // check external frequency is less than the maximum frequency
    if  (crystal_val > 60000000) {return 0x21;}

    // check crystal frequency is within spec. if crystal osc is being used as PLL ref
    if (erefs_val)
    {
    	if ((crystal_val < 8000000) || (crystal_val > 32000000)) {return 0x22;} // return 1 if one of the available crystal options is not available
    }

    // make sure HGO will never be greater than 1. Could return an error instead if desired.
    if (hgo_val > 0)
    {
    	hgo_val = 1; // force hgo_val to 1 if > 0
    }

    // Check PLL divider settings are within spec.
    if ((prdiv_val < 1) || (prdiv_val > 8)) {return 0x41;}
    if ((vdiv_val < 16) || (vdiv_val > 47)) {return 0x42;}

    // Check PLL reference clock frequency is within spec.
    ref_freq = crystal_val / prdiv_val;
    if ((ref_freq < 8000000) || (ref_freq > 32000000)) {return 0x43;}

    // Check PLL output frequency is within spec.
    pll_freq = (crystal_val / prdiv_val) * vdiv_val;
    if ((pll_freq < 180000000) || (pll_freq > 360000000)) {return 0x45;}

    // Determine if oscillator needs to be set up
    if (init_osc)
    {
    	// Check if the oscillator needs to be configured
    	if (!osc_select)
    	{
    		// configure the MCG_C2 register
    		// the RANGE value is determined by the external frequency. Since the RANGE parameter affects the FRDIV divide value
    		// it still needs to be set correctly even if the oscillator is not being used

    		temp_reg = MCG_C2;
    		temp_reg &= ~(MCG_C2_RANGE_MASK | MCG_C2_HGO_MASK | MCG_C2_EREFS_MASK); // clear fields before writing new values

    		if (crystal_val <= 8000000)
    		{
    			temp_reg |= (MCG_C2_RANGE(1) | (hgo_val << MCG_C2_HGO_SHIFT) | (erefs_val << MCG_C2_EREFS_SHIFT));
    		}
    		else
    		{
    			// On rev. 1.0 of silicon there is an issue where the the input bufferd are enabled when JTAG is connected.
    			// This has the affect of sometimes preventing the oscillator from running. To keep the oscillator amplitude
    			// low, RANGE = 2 should not be used. This should be removed when fixed silicon is available.
    			//temp_reg |= (MCG_C2_RANGE(2) | (hgo_val << MCG_C2_HGO_SHIFT) | (erefs_val << MCG_C2_EREFS_SHIFT));
    			temp_reg |= (MCG_C2_RANGE(1) | (hgo_val << MCG_C2_HGO_SHIFT) | (erefs_val << MCG_C2_EREFS_SHIFT));
    		}
    		MCG_C2 = temp_reg;
    	}
    	else
    	{
    		// configure the MCG_C10 register
    		// the RANGE value is determined by the external frequency. Since the RANGE parameter affects the FRDIV divide value
    		// it still needs to be set correctly even if the oscillator is not being used
    		temp_reg = MCG_C10;
    		temp_reg &= ~(MCG_C10_RANGE2_MASK | MCG_C10_HGO2_MASK | MCG_C10_EREFS2_MASK); // clear fields before writing new values
    		if (crystal_val <= 8000000)
    		{
    			temp_reg |= MCG_C10_RANGE2(1) | (hgo_val << MCG_C10_HGO2_SHIFT) | (erefs_val << MCG_C10_EREFS2_SHIFT);
    		}
    		else
    		{
    			// On rev. 1.0 of silicon there is an issue where the the input bufferd are enabled when JTAG is connected.
    			// This has the affect of sometimes preventing the oscillator from running. To keep the oscillator amplitude
    			// low, RANGE = 2 should not be used. This should be removed when fixed silicon is available.
    			//temp_reg |= MCG_C10_RANGE2(2) | (hgo_val << MCG_C10_HGO2_SHIFT) | (erefs_val << MCG_C10_EREFS2_SHIFT);
    			temp_reg |= MCG_C10_RANGE2(1) | (hgo_val << MCG_C10_HGO2_SHIFT) | (erefs_val << MCG_C10_EREFS2_SHIFT);
    		}
    		MCG_C10 = temp_reg;
    	} // if (!osc_select)
    } // if (init_osc)

    if (mcgout_select)
    {
    	// determine FRDIV based on reference clock frequency
    	// since the external frequency has already been checked only the maximum frequency for each FRDIV value needs to be compared here.
    	if (crystal_val <= 1250000) {frdiv_val = 0;}
    	else if (crystal_val <= 2500000) {frdiv_val = 1;}
    	else if (crystal_val <= 5000000) {frdiv_val = 2;}
    	else if (crystal_val <= 10000000) {frdiv_val = 3;}
    	else if (crystal_val <= 20000000) {frdiv_val = 4;}
    	else {frdiv_val = 5;}

    	// Select external oscillator and Reference Divider and clear IREFS to start ext osc
    	// If IRCLK is required it must be enabled outside of this driver, existing state will be maintained
    	// CLKS=2, FRDIV=frdiv_val, IREFS=0, IRCLKEN=0, IREFSTEN=0
    	temp_reg = MCG_C1;
    	temp_reg &= ~(MCG_C1_CLKS_MASK | MCG_C1_FRDIV_MASK | MCG_C1_IREFS_MASK); // Clear values in these fields
    	temp_reg = MCG_C1_CLKS(2) | MCG_C1_FRDIV(frdiv_val); // Set the required CLKS and FRDIV values
    	MCG_C1 = temp_reg;

    	// if the external oscillator is used need to wait for OSCINIT to set
    	if (erefs_val)
    	{
    		for (i = 0 ; i < 10000 ; i++)
    		{
    			if (MCG_S & MCG_S_OSCINIT_MASK) break; // jump out early if OSCINIT sets before loop finishes
    		}
    		if (!(MCG_S & MCG_S_OSCINIT_MASK)) return 0x23; // check bit is really set and return with error if not set
    	}

    	// wait for Reference clock Status bit to clear
    	for (i = 0 ; i < 2000 ; i++)
    	{
    		if (!(MCG_S & MCG_S_IREFST_MASK)) break; // jump out early if IREFST clears before loop finishes
    	}
    	if (MCG_S & MCG_S_IREFST_MASK) return 0x11; // check bit is really clear and return with error if not set

    	// Wait for clock status bits to show clock source is ext ref clk
    	for (i = 0 ; i < 2000 ; i++)
    	{
    		if (((MCG_S & MCG_S_CLKST_MASK) >> MCG_S_CLKST_SHIFT) == 0x2) break; // jump out early if CLKST shows EXT CLK slected before loop finishes
    	}
    	if (((MCG_S & MCG_S_CLKST_MASK) >> MCG_S_CLKST_SHIFT) != 0x2) return 0x1A; // check EXT CLK is really selected and return with error if not

    	// Now in FBE
    	// It is recommended that the clock monitor is enabled when using an external clock as the clock source/reference.
    	// It is enabled here but can be removed if this is not required.
    	MCG_C6 |= MCG_C6_CME_MASK;

    	// Select which PLL to enable
    	if (!pll_select)
    	{
    		// Configure PLL0
    		// Ensure OSC0 is selected as the reference clock
    		MCG_C5 &= ~MCG_C5_PLLREFSEL_MASK;
    		//Select PLL0 as the source of the PLLS mux
    		MCG_C11 &= ~MCG_C11_PLLCS_MASK;
    		// Configure MCG_C5
    		// If the PLL is to run in STOP mode then the PLLSTEN bit needs to be OR'ed in here or in user code.
    		temp_reg = MCG_C5;
    		temp_reg &= ~MCG_C5_PRDIV_MASK;
    		temp_reg |= MCG_C5_PRDIV(prdiv_val - 1);    //set PLL ref divider
    		MCG_C5 = temp_reg;

    		// Configure MCG_C6
    		// The PLLS bit is set to enable the PLL, MCGOUT still sourced from ext ref clk
    		// The loss of lock interrupt can be enabled by seperately OR'ing in the LOLIE bit in MCG_C6
    		temp_reg = MCG_C6; // store present C6 value
    		temp_reg &= ~MCG_C6_VDIV_MASK; // clear VDIV settings
    		temp_reg |= MCG_C6_PLLS_MASK | MCG_C6_VDIV(vdiv_val - 16); // write new VDIV and enable PLL
    		MCG_C6 = temp_reg; // update MCG_C6

    		// wait for PLLST status bit to set
    		for (i = 0 ; i < 2000 ; i++)
    		{
    			if (MCG_S & MCG_S_PLLST_MASK) break; // jump out early if PLLST sets before loop finishes
    		}
    		if (!(MCG_S & MCG_S_PLLST_MASK)) return 0x16; // check bit is really set and return with error if not set

    		// Wait for LOCK bit to set
    		for (i = 0 ; i < 2000 ; i++)
    		{
    			if (MCG_S & MCG_S_LOCK_MASK) break; // jump out early if LOCK sets before loop finishes
    		}
    		if (!(MCG_S & MCG_S_LOCK_MASK)) return 0x44; // check bit is really set and return with error if not set

    		// Use actual PLL settings to calculate PLL frequency
    		prdiv = ((MCG_C5 & MCG_C5_PRDIV_MASK) + 1);
    		vdiv = ((MCG_C6 & MCG_C6_VDIV_MASK) + 16);
    	}
    	else
    	{
    		// Configure PLL1
    		// Ensure OSC0 is selected as the reference clock
    		MCG_C11 &= ~MCG_C11_PLLREFSEL2_MASK;
    		//Select PLL1 as the source of the PLLS mux
    		MCG_C11 |= MCG_C11_PLLCS_MASK;
    		// Configure MCG_C11
    		// If the PLL is to run in STOP mode then the PLLSTEN2 bit needs to be OR'ed in here or in user code.
    		temp_reg = MCG_C11;
    		temp_reg &= ~MCG_C11_PRDIV2_MASK;
    		temp_reg |= MCG_C11_PRDIV2(prdiv_val - 1);    //set PLL ref divider
    		MCG_C11 = temp_reg;

    		// Configure MCG_C12
    		// The PLLS bit is set to enable the PLL, MCGOUT still sourced from ext ref clk
    		// The loss of lock interrupt can be enabled by seperately OR'ing in the LOLIE2 bit in MCG_C12
    		temp_reg = MCG_C12; // store present C12 value
    		temp_reg &= ~MCG_C12_VDIV2_MASK; // clear VDIV settings
    		temp_reg |=  MCG_C12_VDIV2(vdiv_val - 16); // write new VDIV and enable PLL
    		MCG_C12 = temp_reg; // update MCG_C12
    		// Enable PLL by setting PLLS bit
    		MCG_C6 |= MCG_C6_PLLS_MASK;

    		// wait for PLLCST status bit to set
    		for (i = 0 ; i < 2000 ; i++)
    		{
    			if (MCG_S2 & MCG_S2_PLLCST_MASK) break; // jump out early if PLLST sets before loop finishes
    		}
    		if (!(MCG_S2 & MCG_S2_PLLCST_MASK)) return 0x17; // check bit is really set and return with error if not set

    		// wait for PLLST status bit to set
    		for (i = 0 ; i < 2000 ; i++)
    		{
    			if (MCG_S & MCG_S_PLLST_MASK) break; // jump out early if PLLST sets before loop finishes
    		}
    		if (!(MCG_S & MCG_S_PLLST_MASK)) return 0x16; // check bit is really set and return with error if not set

    		// Wait for LOCK bit to set
    		for (i = 0 ; i < 2000 ; i++)
    		{
    			if (MCG_S2 & MCG_S2_LOCK2_MASK) break; // jump out early if LOCK sets before loop finishes
    		}
    		if (!(MCG_S2 & MCG_S2_LOCK2_MASK)) return 0x44; // check bit is really set and return with error if not set

    		// Use actual PLL settings to calculate PLL frequency
    		prdiv = ((MCG_C11 & MCG_C11_PRDIV2_MASK) + 1);
    		vdiv = ((MCG_C12 & MCG_C12_VDIV2_MASK) + 16);
    	} // if (!pll_select)

    	// now in PBE

    	MCG_C1 &= ~MCG_C1_CLKS_MASK; // clear CLKS to switch CLKS mux to select PLL as MCG_OUT

    	// Wait for clock status bits to update
    	for (i = 0 ; i < 2000 ; i++)
    	{
    		if (((MCG_S & MCG_S_CLKST_MASK) >> MCG_S_CLKST_SHIFT) == 0x3) break; // jump out early if CLKST = 3 before loop finishes
    	}
    	if (((MCG_S & MCG_S_CLKST_MASK) >> MCG_S_CLKST_SHIFT) != 0x3) return 0x1B; // check CLKST is set correctly and return with error if not

    	// Now in PEE
    }
    else
    {
    	// Setup PLL for peripheral only use
    	if (pll_select)
    	{
    		// Setup and enable PLL1
    		// Select ref source
    		if (osc_select)
    		{
    			MCG_C11 |= MCG_C11_PLLREFSEL2_MASK; // Set select bit to choose OSC1
    		}
    		else
    		{
    			MCG_C11 &= ~MCG_C11_PLLREFSEL2_MASK; // Clear select bit to choose OSC0
    		}
    		// Configure MCG_C11
    		// If the PLL is to run in STOP mode then the PLLSTEN2 bit needs to be OR'ed in here or in user code.
    		temp_reg = MCG_C11;
    		temp_reg &= ~MCG_C11_PRDIV2_MASK;
    		temp_reg |= MCG_C11_PRDIV2(prdiv_val - 1);    //set PLL ref divider
    		MCG_C11 = temp_reg;

    		// Configure MCG_C12
    		// The loss of lock interrupt can be enabled by seperately OR'ing in the LOLIE2 bit in MCG_C12
    		temp_reg = MCG_C12; // store present C12 value
    		temp_reg &= ~MCG_C12_VDIV2_MASK; // clear VDIV settings
    		temp_reg |=  MCG_C12_VDIV2(vdiv_val - 16); // write new VDIV and enable PLL
    		MCG_C12 = temp_reg; // update MCG_C12
    		// Now enable the PLL
    		MCG_C11 |= MCG_C11_PLLCLKEN2_MASK; // Set PLLCLKEN2 to enable PLL1

    		// Wait for LOCK bit to set
    		for (i = 0 ; i < 2000 ; i++)
    		{
    			if (MCG_S2 & MCG_S2_LOCK2_MASK) break; // jump out early if LOCK sets before loop finishes
    		}
    		if (!(MCG_S2 & MCG_S2_LOCK2_MASK)) return 0x44; // check bit is really set and return with error if not set

    		// Use actual PLL settings to calculate PLL frequency
    		prdiv = ((MCG_C11 & MCG_C11_PRDIV2_MASK) + 1);
    		vdiv = ((MCG_C12 & MCG_C12_VDIV2_MASK) + 16);
    	}
    	else
    	{
    		// Setup and enable PLL0
    		// Select ref source
    		if (osc_select)
    		{
    			MCG_C5 |= MCG_C5_PLLREFSEL_MASK; // Set select bit to choose OSC1
    		}
    		else
    		{
    			MCG_C5 &= ~MCG_C5_PLLREFSEL_MASK; // Clear select bit to choose OSC0
    		}
    		// Configure MCG_C5
    		// If the PLL is to run in STOP mode then the PLLSTEN bit needs to be OR'ed in here or in user code.
    		temp_reg = MCG_C5;
    		temp_reg &= ~MCG_C5_PRDIV_MASK;
    		temp_reg |= MCG_C5_PRDIV(prdiv_val - 1);    //set PLL ref divider
    		MCG_C5 = temp_reg;

    		// Configure MCG_C6
    		// The loss of lock interrupt can be enabled by seperately OR'ing in the LOLIE bit in MCG_C6
    		temp_reg = MCG_C6; // store present C6 value
    		temp_reg &= ~MCG_C6_VDIV_MASK; // clear VDIV settings
    		temp_reg |=  MCG_C6_VDIV(vdiv_val - 16); // write new VDIV and enable PLL
    		MCG_C6 = temp_reg; // update MCG_C6
    		// Now enable the PLL
    		MCG_C5 |= MCG_C5_PLLCLKEN_MASK; // Set PLLCLKEN to enable PLL0

    		// Wait for LOCK bit to set
    		for (i = 0 ; i < 2000 ; i++)
    		{
    			if (MCG_S & MCG_S_LOCK_MASK) break; // jump out early if LOCK sets before loop finishes
    		}
    		if (!(MCG_S & MCG_S_LOCK_MASK)) return 0x44; // check bit is really set and return with error if not set

    		// Use actual PLL settings to calculate PLL frequency
    		prdiv = ((MCG_C5 & MCG_C5_PRDIV_MASK) + 1);
    		vdiv = ((MCG_C6 & MCG_C6_VDIV_MASK) + 16);
    	} // if (pll_select)

    } // if (mcgout_select)

    return (((crystal_val / prdiv) * vdiv) / 2); //MCGOUT equals PLL output frequency/2
#endif
}

/* EOF */
