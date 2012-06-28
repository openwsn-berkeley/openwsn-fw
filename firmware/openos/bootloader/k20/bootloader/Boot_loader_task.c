/**
\brief Include the OS and BSP dependent files that define IO functions and
 basic types. You may like to change these files for your board and RTOS.
 Based on Freescale bootloader demo for k60. 

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, June 2012.
 */

#include "derivative.h"
#include "hidef.h"
#include <string.h>
#include <stdlib.h>    /* ANSI memory controls */
#include "stdio.h"
//#include "sci.h"
#include "Bootloader.h"
#include "Boot_loader_task.h"

#if (defined MCU_MK60N512VMD100 |defined MCU_MK20D7 )
#include "flash_FTFL.h"
#endif

/********************** Memory protected area *******************/

#if (defined MCU_MK60N512VMD100||defined MCU_MK20D7)
	  // Protect bootloader flash 0x0 - 0xBFFF
	  #pragma define_section cfmconfig ".cfmconfig" ".cfmconfig" ".cfmconfig" far_abs R
	  static __declspec(cfmconfig) uint_8 _cfm[0x10] = {
	   /* NV_BACKKEY3: KEY=0xFF */
	    0xFFU,
	   /* NV_BACKKEY2: KEY=0xFF */
	    0xFFU,
	   /* NV_BACKKEY1: KEY=0xFF */
	    0xFFU,
	   /* NV_BACKKEY0: KEY=0xFF */
	    0xFFU,
	   /* NV_BACKKEY7: KEY=0xFF */
	    0xFFU,
	   /* NV_BACKKEY6: KEY=0xFF */
	    0xFFU,
	   /* NV_BACKKEY5: KEY=0xFF */
	    0xFFU,
	   /* NV_BACKKEY4: KEY=0xFF */
	    0xFFU,
	   /* NV_FPROT3: PROT=0xF8 */
	    PROT_VALUE3,
	   /* NV_FPROT2: PROT=0xFF */
	    PROT_VALUE2,
	   /* NV_FPROT1: PROT=0xFF */
	    PROT_VALUE1,
	   /* NV_FPROT0: PROT=0xFF */
	    PROT_VALUE0,
	   /* NV_FSEC: KEYEN=1,MEEN=3,FSLACC=3,SEC=2 */
	    0x7EU,
	   /* NV_FOPT: ??=1,??=1,??=1,??=1,??=1,??=1,EZPORT_DIS=1,LPBOOT=1 */
	    0xFFU,
	   /* NV_FEPROT: EPROT=0xFF */
	    0xFFU,
	   /* NV_FDPROT: DPROT=0xFF */
	    0xFFU
	  };
#else
	#error Undefined MCU for flash protection
#endif
	
/**************************************************************************
   Global variables
**************************************************************************/
extern uint_8                       filetype;      /* image file type */     
static uint_32                      New_sp,New_pc; /* stack pointer and program counter */
/**************************************************************************
   Funciton prototypes
**************************************************************************/
#if MAX_TIMER_OBJECTS
extern uint_8 TimerQInitialize(uint_8 ControllerId);
#endif

/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : GPIO_Init
* Returned Value : none
* Comments       : Init LEDs and Buttons
*
*END*--------------------------------------------------------------------*/
void GPIO_Bootloader_Init()
{
    /* Body */
#if defined(MCU_MK60N512VMD100)
    SIM_SCGC5   |= SIM_SCGC5_PORTA_MASK;    /* Enable clock gating to PORTA */
    /* Enable LEDs Port A pin 28 & 29 */                                    
    PORTA_PCR28 |= PORT_PCR_SRE_MASK        /* Slow slew rate */
                |  PORT_PCR_ODE_MASK        /* Open Drain Enable */
                |  PORT_PCR_DSE_MASK        /* High drive strength */
            ;
    PORTA_PCR28 = PORT_PCR_MUX(1);
    PORTA_PCR29 |= PORT_PCR_SRE_MASK        /* Slow slew rate */
                |  PORT_PCR_ODE_MASK        /* Open Drain Enable */
                |  PORT_PCR_DSE_MASK        /* High drive strength */
            ;
    PORTA_PCR29 = PORT_PCR_MUX(1);
    GPIOA_PSOR  |= 1 << 28 | 1 << 29;
    GPIOA_PDDR  |= 1 << 28 | 1 << 29;
    /* set in put PORTA pin 19 */
    PORTA_PCR19 =  PORT_PCR_MUX(1);
    GPIOA_PDDR &= ~((uint_32)1 << 19);
    PORTA_PCR19 |= PORT_PCR_PE_MASK|PORT_PCR_PS_MASK;/* pull up*/
#endif
//TODO select what pin will be used in openmote k20 and configure it 
    

} /* EndBody */

/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : Switch_mode
* Returned Value : none
* Comments       : Jump between application and bootloader
*
*END*--------------------------------------------------------------------*/
void Switch_mode(void)
{
    /* Body */
    volatile uint_32  temp = 1;   /* default the button is not pressed */
    /* Get PC and SP of application region */
    New_sp  = ((uint_32_ptr)IMAGE_ADDR)[0];
    New_pc  = ((uint_32_ptr)IMAGE_ADDR)[1];
    /* Check switch is pressed*/
#if defined(MCU_MK60N512VMD100)
    temp =(uint_32) ((1<<19) & GPIOA_PDIR);               /* DES READ SW1 of TWK60 */
#elif defined(MCU_MK20D7)
    temp =(uint_32) ((1<<2) & GPIOC_PDIR);
#endif /* End MCU_MK20D7 */

    if(temp)
    {
        if((New_sp != 0xffffffff)&&(New_pc != 0xffffffff))
        {
            /* Run the application */
#if (!defined __MK_xxx_H__)
            asm
            {
                move.w   #0x2700,sr
                move.l   New_sp,a0
                move.l   New_pc,a1
                move.l   a0,a7
                jmp     (a1)
            }
#elif defined(__CWCC__)
            asm
            {
                ldr   r4,=New_sp
                ldr   sp, [r4]
                ldr   r4,=New_pc
                ldr   r5, [r4]
                blx   r5
            }
#elif defined(__IAR_SYSTEMS_ICC__)
            asm("mov32   r4,New_sp");
            asm("ldr     sp,[r4]");
            asm("mov32   r4,New_pc");
            asm("ldr     r5, [r4]");
            asm("blx     r5");
#endif /* end (!defined __MK_xxx_H__) */
        } /* EndIf */
    }

} /* EndBody */

/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : erase_flash
* Returned Value : none
* Comments       : erase flash memory in application area
*
*END*--------------------------------------------------------------------*/

uint_8 erase_flash(void)
{ 
    /* Body */
    uint_8 error = FALSE;
    uint_8 i;
    Flash_Init(59);
    DisableInterrupts;                      
    for (i=0;i<(MAX_FLASH1_ADDRESS -(uint_32) IMAGE_ADDR)/ERASE_SECTOR_SIZE;i++)
    {
#if (!defined __MK_xxx_H__)    
        error = Flash_SectorErase((uint_32*)((uint_32) IMAGE_ADDR + ERASE_SECTOR_SIZE*i)) ; /* ERASE 4k flash memory */
#else
        error = Flash_SectorErase((uint_32) IMAGE_ADDR + ERASE_SECTOR_SIZE*i) ; /* ERASE 4k flash memory */
#endif
        if(error != Flash_OK)
        {
            break;
        } /* Endif */
    } /* Endfor */
    EnableInterrupts;
    return error;
} /* EndBody */

/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : SetOutput
* Returned Value : None
* Comments       : Set out put of the LEDs
*     
*END*--------------------------------------------------------------------*/

void SetOutput
    (
        /* [IN] the output pin */
        uint_32 output,
        /* [IN] the state to set */
        boolean state
    ) 
{
    /* Body */


/* For TWR-K60 */
#if (defined MCU_MK60N512VMD100)
    if(state)
        GPIOA_PCOR |= output; 
    else
        GPIOA_PSOR |= output; 
#endif
    
    
#if (defined MCU_MK20D7)
    if(state)
        GPIOC_PCOR |= output; 
    else
        GPIOC_PSOR |= output; 
#endif

} /* EndBody */

  /* EOF */
