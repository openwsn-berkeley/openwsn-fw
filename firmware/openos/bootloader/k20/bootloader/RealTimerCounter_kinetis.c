/******************************************************************************
 *
 * Freescale Semiconductor Inc.
 * (c) Copyright 2004-2010 Freescale Semiconductor, Inc.
 * ALL RIGHTS RESERVED.
 *
 ******************************************************************************
 *
 * THIS SOFTWARE IS PROVIDED BY FREESCALE "AS IS" AND ANY EXPRESSED OR 
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  
 * IN NO EVENT SHALL FREESCALE OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 **************************************************************************//*!
 *
 * @file RealTimerCounter.c
 *
 * @author
 *
 * @version
 *
 * @date    
 *
 * @brief   This file configures Real Time Counter (RTC) for Timer 
 *          Implementation. It doesn't use the Kinetis RTC (Real Time Clock) 
 *          module.
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <string.h>
#include "user_config.h"
#include "derivative.h"     /* include peripheral declarations */
#include "types.h"          /* Contains User Defined Data Types */
#include "RealTimerCounter.h"


#if MAX_TIMER_OBJECTS
/*****************************************************************************
 * Local Functions Prototypes
 *****************************************************************************/
static uint_8 TimerInit(void);
static void EnableTimerInterrupt(void);
static void DisableTimerInterrupt(void);

/****************************************************************************
 * Global Variables
 ****************************************************************************/
/* Array of Timer Objects */
TIMER_OBJECT g_TimerObjectArray[MAX_TIMER_OBJECTS];
/*****************************************************************************
 * Global Functions Prototypes
 *****************************************************************************/
uint_8 TimerQInitialize(uint_8 controller_ID);
uint_8 AddTimerQ(PTIMER_OBJECT pTimerObject);
uint_8 RemoveTimerQ(uint_8 index);
void Timer_ISR(void);
/*****************************************************************************
 * Global Functions
 *****************************************************************************/
/******************************************************************************
 *
 *   @name        TimerQInitialize
 *
 *   @brief       Initializes RTC, Timer Object Queue and System Clock Counter
 *
 *	 @param       controller_ID    : Controller ID
 *
 *   @return      None
 *****************************************************************************
 * This function initializes System Clock Counter, Timer Queue and Initializes
 * System Timer
 *****************************************************************************/
uint_8 TimerQInitialize(uint_8 controller_ID)
{
    UNUSED (controller_ID)
	(void)memset(g_TimerObjectArray, (int)NULL, sizeof(g_TimerObjectArray));
	return TimerInit();
}

/******************************************************************************
 *
 *   @name        AddTimerQ
 *
 *   @brief       Adds Timer Object to Timer Queue
 *
 *	 @param       pTimerObject	: Pointer to Timer Object
 *
 *   @return      None
 *****************************************************************************
 * Adds Timer Object to Timer Queue
 *****************************************************************************/
uint_8 AddTimerQ(PTIMER_OBJECT pTimerObject)
{
	uint_8 index;
	if(pTimerObject == NULL)
		return (uint_8)ERR_INVALID_PARAM;
	if(pTimerObject->msCount == (unsigned int)INVALID_TIME_COUNT)
		return (uint_8)ERR_INVALID_PARAM;
	
	for(index = 0; index < MAX_TIMER_OBJECTS; index++)
	{
	  /* Disable Timer Interrupts */
    DisableTimerInterrupt();
		
		if(g_TimerObjectArray[index].pfnTimerCallback == NULL)
		{
			(void)memcpy(&g_TimerObjectArray[index], pTimerObject, sizeof(TIMER_OBJECT)); 
			/* Enable Timer Interrupts */
			EnableTimerInterrupt();
			break;
		}
	  /* Enable Timer Interrupts */
		EnableTimerInterrupt();
	}
	if(index == MAX_TIMER_OBJECTS)
		return (uint_8)ERR_TIMER_QUEUE_FULL;
	return index;
}

/******************************************************************************
 *
 *   @name        RemoveTimerQ
 *
 *   @brief       Removes Timer Object from Timer Queue
 *
 *	 @param       index	: Index of Timer Object
 *
 *   @return      None
 *****************************************************************************
 * Removes Timer Object from Timer Queue
 *****************************************************************************/
uint_8 RemoveTimerQ(uint_8 index)
{
	if(index >= MAX_TIMER_OBJECTS)
		return (uint_8)ERR_INVALID_PARAM;
	/* Disable Timer Interrupts */
	DisableTimerInterrupt();
	(void)memset(&g_TimerObjectArray[index], (int)NULL, sizeof(TIMER_OBJECT));
	/* Enable Timer Interrupts */
	EnableTimerInterrupt();
	return (uint_8)ERR_SUCCESS;
}

/******************************************************************************
 *   @name        TimerInit
 *
 *   @brief       This is RTC initialization function
 *
 *   @return      None
 *
 ******************************************************************************
 * Initializes the RTC module registers
 *****************************************************************************/
static uint_8 TimerInit(void)
{
	/* Enable PIT0 Module Clock */
	SIM_SCGC6 |= SIM_SCGC6_PIT_MASK;
	
	/* Configure PIT0 */
	PIT_MCR = ~(PIT_MCR_FRZ_MASK | PIT_MCR_MDIS_MASK);
	
	/* Timer 0.1ms */
	#ifndef MCU_MK70F12
		#if (defined MCU_MK20D7) || (defined MCU_MK40D7)
			#ifdef MCGOUTCLK_72_MHZ
				PIT_LDVAL0 = 36000;
			#else
				PIT_LDVAL0 = 48000;
			#endif
		#else
			PIT_LDVAL0 = 48000;
		#endif		
	#else
		PIT_LDVAL0 = 60000;
	#endif
	
	/* Enable PIT interrupt */
	PIT_TCTRL0 = ~PIT_TCTRL_TEN_MASK;
	PIT_TCTRL0 = ~PIT_TCTRL_TIE_MASK;
	
	/* Mask PIT interrupt flag */
	PIT_TFLG0 = PIT_TFLG_TIF_MASK;
	
	#ifdef MCU_MK20D5
		NVICICPR0 |= (uint32_t)(1<<30);   /* Clear any pending interrupts on PIT0 */
		NVICISER0 |= (uint32_t)(1<<30);   /* Enable interrupts from PIT0 module */
	#else
		NVICICPR2 |= (uint32_t)(1<<4);   /* Clear any pending interrupts on PIT0 */
		NVICISER2 |= (uint32_t)(1<<4);   /* Enable interrupts from PIT0 module */
	#endif
	
    return ERR_SUCCESS;
}

/******************************************************************************
 *   @name        EnableTimerInterrupt
 *
 *   @brief       This routine enables Timer Interrupt
 *
 *   @return      None
 *
 ******************************************************************************
 * Enables RTC Timer Interrupt
 *****************************************************************************/
static void EnableTimerInterrupt(void)
{
	/* Enable Timer Interrupt */
	PIT_TCTRL0 |= PIT_TCTRL_TEN_MASK;
	PIT_TCTRL0 |= PIT_TCTRL_TIE_MASK;
	return;
}

/******************************************************************************
 *   @name        DisableTimerInterrupt
 *
 *   @brief       This routine disables Timer Interrupt
 *
 *   @return      None
 *
 ******************************************************************************
 * Disables RTC Timer Interrupt
 *****************************************************************************/
static void DisableTimerInterrupt(void)
{
	/* Disable Timer Interrupt */
	PIT_TCTRL0 &= ~PIT_TCTRL_TEN_MASK;
	PIT_TCTRL0 &= ~PIT_TCTRL_TIE_MASK;
	return;
}

/******************************************************************************
 *   @name        Timer_ISR
 *
 *   @brief       This routine services RTC Interrupt
 *
 *	 @param       None
 *
 *   @return      None
 *
 ******************************************************************************
 * Services RTC Interrupt. If a Timer Object expires, then removes the object 
 * from Timer Queue and Calls the callback function (if registered)
 *****************************************************************************/
void Timer_ISR(void)
{
	uint_8 index;
	if(PIT_TFLG0 & PIT_TFLG_TIF_MASK)
    {
		/* Clear RTC Interrupt */
		PIT_TFLG0 |= PIT_TFLG_TIF_MASK;
		
		DisableTimerInterrupt();
		   	
		#ifdef MCU_MK20D5
			NVICICPR0 |= (1<<30);   /* Clear any pending interrupts on PIT0 */
			NVICISER0 |= (1<<30);   /* Enable interrupts from PIT0 module */
		#else	
			NVICICPR2 |= (1<<4);   /* Clear any pending interrupts on PIT0 */
			NVICISER2 |= (1<<4);   /* Enable interrupts from PIT0 module */
		#endif		
	    
	    /* Call Pending Timer CallBacks */
		for (index = 0; index < MAX_TIMER_OBJECTS; index++)
		{
			PTIMER_OBJECT ptemp = &g_TimerObjectArray[index];
			if(ptemp->pfnTimerCallback == NULL)
			{
				continue;
			}
			ptemp->msCount--;
			if (ptemp->msCount == 0) 
			{
			    PFNTIMER_CALLBACK pfnTimerCallback = ptemp->pfnTimerCallback;
#ifdef TIMER_CALLBACK_ARG
			    void *parg = ptemp->arg;
#endif
			    (void)RemoveTimerQ(index);
#ifdef TIMER_CALLBACK_ARG
				pfnTimerCallback(parg);
#else
				pfnTimerCallback();
#endif
			}
		}
	}
	EnableTimerInterrupt();
}
#endif
