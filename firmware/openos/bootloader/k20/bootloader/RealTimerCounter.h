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
 * @file RealTimerCounter.h
 *
 * @author
 *
 * @version
 *
 * @date    
 *
 * @brief   This is a header file for Real Time Counter (RTC)
 *****************************************************************************/

#ifndef _RTC_HEADER_H
#define _RTC_HEADER_H
/******************************************************************************
 * Includes
 *****************************************************************************/
#include "types.h"          /* Contains User Defined Data Types */
#include "user_config.h"    /* User Defined Configuration Parameters */
/*****************************************************************************
 * Constant and Macro's
 *****************************************************************************/
#define ERR_SUCCESS             0
#define ERR_INVALID_PARAM       -1
#define ERR_TIMER_QUEUE_FULL	-2
#define INVALID_TIME_COUNT		0xFFFFFFFF
/******************************************************************************
 * Types
 *****************************************************************************/

/* Timer Callback Function Prototype */
#ifdef TIMER_CALLBACK_ARG
typedef void (*PFNTIMER_CALLBACK)(void* arg);
#else
typedef void (*PFNTIMER_CALLBACK)(void);
#endif
/* Timer Object Structure */
typedef struct _timer_object
{
    unsigned int msCount;				/* msec Timeout Value */
    PFNTIMER_CALLBACK pfnTimerCallback;	/* Callback Function */
#ifdef TIMER_CALLBACK_ARG
	void* arg;							/* Callback Function Argument */
#endif
} TIMER_OBJECT, *PTIMER_OBJECT;

/*****************************************************************************
 * Global Functions Prototypes
 *****************************************************************************/
extern uint_8 TimerQInitialize(uint_8 ControllerId);
extern uint_8 AddTimerQ(PTIMER_OBJECT pTimerObject);
extern uint_8 RemoveTimerQ(uint_8 index);

#endif /* _RTC_HEADER_H */
