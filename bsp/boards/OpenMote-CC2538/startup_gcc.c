/**
 * Author: Xavier Vilajosana (xvilajosana@eecs.berkeley.edu)
 *         Pere Tuset (peretuset@openmote.com)
 * Date:   July 2013
 * Description: CC2538-specific startup code for GCC.
 */

//=========================== include =========================================

#include <stdint.h>

#include <headers/hw_types.h>
#include <headers/hw_nvic.h>

//=========================== defines =========================================

#define STACK_SIZE                      ( 1024 )

#define FLASH_START_ADDR                ( 0x00200000 )
#define BOOTLOADER_BACKDOOR_ENABLED     ( 0xF6FFFFFF ) // ENABLED: GPIO A, PIN 5, LOW
#define BOOTLOADER_BACKDOOR_DISABLED    ( 0xEFFFFFFF ) // DISABLED
#define SYS_CTRL_EMUOVR                 ( 0x400D20B4 )
#define SYS_CTRL_I_MAP                  ( 0x400D2098 )

//=========================== typedef =========================================

typedef struct {
    uint32_t bootloader_cfg;
    uint32_t is_valid_image;
    uint32_t image_address;
} cca_page_t;

//=========================== prototypes ======================================

void reset_handler(void);
void nmi_handler(void);
void fault_handler(void);
void default_handler(void);

void adc_interrupt(void)            __attribute__ ((weak, alias("default_handler")));
void gpio_a_interrupt(void)         __attribute__ ((weak, alias("default_handler")));
void gpio_b_interrupt(void)         __attribute__ ((weak, alias("default_handler")));
void gpio_c_interrupt(void)         __attribute__ ((weak, alias("default_handler")));
void gpio_d_interrupt(void)         __attribute__ ((weak, alias("default_handler")));
void uart0_interrupt(void)          __attribute__ ((weak, alias("default_handler")));
void uart1_interrupt(void)          __attribute__ ((weak, alias("default_handler")));
void ssi0_interrupt(void)           __attribute__ ((weak, alias("default_handler")));
void ssi1_interrupt(void)           __attribute__ ((weak, alias("default_handler")));
void i2c_interrupt(void)            __attribute__ ((weak, alias("default_handler")));
void svc_interrupt(void)            __attribute__ ((weak, alias("default_handler")));
void pendsv_interrupt(void)         __attribute__ ((weak, alias("default_handler")));
void systick_interrupt(void)        __attribute__ ((weak, alias("default_handler")));
void rf_core_interrupt(void)        __attribute__ ((weak, alias("default_handler")));
void rf_error_interrupt(void)       __attribute__ ((weak, alias("default_handler")));
void bsp_timer_interrupt(void)      __attribute__ ((weak, alias("default_handler")));
void radio_timer_interrupt(void)    __attribute__ ((weak, alias("default_handler")));
void watchdog_interrupt(void)       __attribute__ ((weak, alias("default_handler")));
void timer0a_interrupt(void)        __attribute__ ((weak, alias("default_handler")));
void timer0b_interrupt(void)        __attribute__ ((weak, alias("default_handler")));
void timer1a_interrupt(void)        __attribute__ ((weak, alias("default_handler")));
void timer1b_interrupt(void)        __attribute__ ((weak, alias("default_handler")));
void timer2a_interrupt(void)        __attribute__ ((weak, alias("default_handler")));
void timer2b_interrupt(void)        __attribute__ ((weak, alias("default_handler")));
void timer3a_interrupt(void)        __attribute__ ((weak, alias("default_handler")));
void timer3b_interrupt(void)        __attribute__ ((weak, alias("default_handler")));

extern int main(void);

//=========================== variables =======================================

extern uint32_t _text_start;
extern uint32_t _text_end;
extern uint32_t _data_start;
extern uint32_t _data_end;
extern uint32_t _bss_start;
extern uint32_t _bss_end;

__attribute__ ((section(".stack"), used))
static uint8_t _stack[STACK_SIZE];

__attribute__ ((section(".flashcca"), used))
const cca_page_t cca_page = {
  BOOTLOADER_BACKDOOR_ENABLED,                          // Bootloader backdoor enabled
  0,                                                    // Image valid bytes
  FLASH_START_ADDR                                      // Vector table located at flash start address
};

__attribute__ ((section(".vectors"), used))
void (* const vector_table[])(void) = {
   (void (*)(void))((uint32_t)_stack + sizeof(_stack)), // Stack pointer
   reset_handler,                                       // Reset handler
   nmi_handler,                                         // The NMI handler
   fault_handler,                                       // The hard fault handler
   default_handler,                                     // 4 The MPU fault handler
   default_handler,                                     // 5 The bus fault handler
   default_handler,                                     // 6 The usage fault handler
   0, 0, 0, 0,                                          // 7-10 Reserved
   default_handler,                                     // 11 SVCall handler
   default_handler,                                     // 12 Debug monitor handler
   0,                                                   // 13 Reserved
   default_handler,                                     // 14 The PendSV handler
   default_handler,                                     // 15 The SysTick handler
   gpio_a_interrupt,                                    // 16 GPIO Port A
   gpio_b_interrupt,                                    // 17 GPIO Port B
   gpio_c_interrupt,                                    // 18 GPIO Port C
   gpio_d_interrupt,                                    // 19 GPIO Port D
   0,                                                   // 20 None
   uart0_interrupt,                                     // 21 UART0 Rx and Tx
   uart1_interrupt,                                     // 22 UART1 Rx and Tx
   ssi0_interrupt,                                      // 23 SSI0 Rx and Tx
   i2c_interrupt,                                       // 24 I2C Master and Slave
   0, 0, 0, 0, 0,                                       // 25-29 Reserved
   adc_interrupt,                                       // 30 ADC Sequence 0
   0, 0, 0,                                             // 31-33 Reserved
   watchdog_interrupt,                                  // 34 Watchdog timer, timer 0
   timer0a_interrupt,                                   // 35 Timer 0 subtimer A
   timer0b_interrupt,                                   // 36 Timer 0 subtimer B
   timer1a_interrupt,                                   // 37 Timer 1 subtimer A
   timer0b_interrupt,                                   // 38 Timer 1 subtimer B
   timer2a_interrupt,                                   // 39 Timer 2 subtimer A
   timer2b_interrupt,                                   // 40 Timer 2 subtimer B
   default_handler,                                     // 41 Analog Comparator 0
   rf_core_interrupt,                                   // 42 RFCore Rx/Tx
   rf_error_interrupt,                                  // 43 RFCore Error
   default_handler,                                     // 44 IcePick
   default_handler,                                     // 45 FLASH Control
   default_handler,                                     // 46 AES
   default_handler,                                     // 47 PKA
   bsp_timer_interrupt,                                 // 48 Sleep Timer
   radio_timer_interrupt,                               // 49 Radio Timer
   ssi1_interrupt,                                      // 50 SSI1 Rx and Tx
   timer3a_interrupt,                                   // 51 Timer 3 subtimer A
   timer3b_interrupt,                                   // 52 Timer 3 subtimer B
   0, 0, 0, 0, 0, 0, 0,                                 // 53-59 Reserved
   default_handler,                                     // 60 USB
   0,                                                   // 61 Reserved
   default_handler,                                     // 62 uDMA
   default_handler,                                     // 63 uDMA Error
#ifndef CC2538_USE_ALTERNATE_INTERRUPT_MAP
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                        // 64-73 Reserved
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                        // 74-83 Reserved
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                        // 84-93 Reserved
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                        // 94-103 Reserved
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                        // 104-113 Reserved
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                        // 114-123 Reserved
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                        // 124-133 Reserved
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                        // 134-143 Reserved
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                        // 144-143 Reserved
   0, 0,                                                // 154-155 Reserved
   default_handler,                                     // 156 USB Reserved
   rf_core_interrupt,                                   // 157 RFCORE RX/TX
   rf_error_interrupt,                                  // 158 RFCORE Error
   default_handler,                                     // 159 AES
   default_handler,                                     // 160 PKA
   bsp_timer_interrupt,                                 // 161 Sleep Timer
   radio_timer_interrupt,                               // 162 Radio Timer
#endif
};

//=========================== public ==========================================

void nmi_handler(void) {
    reset_handler();
    while(1) {
        __asm("nop");
    }
}

void fault_handler(void) {
    while(1) {
        __asm("nop");
    }
}

void default_handler(void) {
    while(1) {
        __asm("nop");
    }
}

void reset_handler(void) {
    volatile uint32_t *src, *dst;

    /* Workaround for PM debug issue */
    HWREG(SYS_CTRL_EMUOVR) = 0xFF;

    /* Workaround for J-Link debug issue */
    HWREG(NVIC_VTABLE) = (uint32_t) vector_table;

    /* Copy the data segment initializers from flash to SRAM */
    for (src = &_text_end, dst = &_data_start; dst < &_data_end;) {
        *dst++ = *src++;
    }

    /* Initialize the BSS section to zero */
    for (dst = &_bss_start; dst < &_bss_end;) {
        *dst++ = 0;
    }

#ifdef CC2538_USE_ALTERNATE_INTERRUPT_MAP
    /* Enable alternate interrupt mapping */
    HWREG(SYS_CTRL_I_MAP) |= 1;
#endif

   /* Call the application's entry point */
   main();

   /* End here if we return from main() */
   while (1) {
       __asm("nop");
   }
}

//=========================== private =========================================
