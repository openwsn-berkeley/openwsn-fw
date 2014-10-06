/**
* Copyright (c) 2014 Atmel Corporation. All rights reserved. 
*  
* Redistribution and use in source and binary forms, with or without 
* modification, are permitted provided that the following conditions are met:
* 
* 1. Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer.
* 
* 2. Redistributions in binary form must reproduce the above copyright notice, 
* this list of conditions and the following disclaimer in the documentation 
* and/or other materials provided with the distribution.
* 
* 3. The name of Atmel may not be used to endorse or promote products derived 
* from this software without specific prior written permission.  
* 
* 4. This software may only be redistributed and used in connection with an 
* Atmel microcontroller product.
* 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE 
* GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
* 
* 
* 
*/

#ifndef cm0plus_interrupt_h__
#define cm0plus_interrupt_h__

#include "sam.h"

/**
 * \brief Enable interrupt vector
 *
 * Enables execution of the software handler for the requested interrupt vector.
 *
 * \param[in] vector Interrupt vector to enable
 */
static inline void nvic_irq_enable(uint32_t vector)
{
	NVIC->ISER[0] = (uint32_t)(1 << (vector & 0x0000001f));
}

/**
 * \brief Disable interrupt vector
 *
 * Disables execution of the software handler for the requested interrupt vector.
 *
 * \param[in] vector Interrupt vector to disable
 */
static inline void nvic_irq_disable(uint32_t vector)
{
	NVIC->ICER[0] = (uint32_t)(1 << (vector & 0x0000001f));
}

/* Definition for irq to store */
typedef uint32_t irqflags_t;

/* Check whether irq is enabled or not */
#define cpu_irq_is_enabled()    (__get_PRIMASK() == 0)

/* Enable the CPU IRQ and update the status*/
#  define cpu_irq_enable()                 \
do {                                       \
	__DMB();                               \
	__enable_irq();                        \
} while (0)

/* Disable the CPU IRQ and update the status */
#  define cpu_irq_disable()                \
do {                                       \
	__disable_irq();                       \
	__DMB();                               \
} while (0)

/* Disable IRQ and return the interrupt flags */
static inline irqflags_t cpu_irq_save(void)
{
	irqflags_t flags = cpu_irq_is_enabled();
	cpu_irq_disable();
	return flags;
}

/* Restore the IRQ Flags, if the IRQ's are already stored */
static inline void cpu_irq_restore(irqflags_t flags)
{
	if (flags)
	cpu_irq_enable();
}


#endif // cm0plus_interrupt_h__


