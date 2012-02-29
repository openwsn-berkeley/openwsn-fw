#ifndef OPENMOTE_PINDEFS_H
#define OPENMOTE_PINDEFS_H
#include "LPC17xx.h"
#include "openal_internal_common.h"

#define NOT_VALID_PIN 0xFF


// Note: const goes after the * for constant pointers, but malleable data

#define OPENMOTE_NUM_GPIO_BANKS 5
extern LPC_GPIO_TypeDef * const LPC17xx_gpio_bank_addresses[OPENMOTE_NUM_GPIO_BANKS];

#define OPENMOTE_NUM_PINS 20
extern const char openmote_pin_gpio_bank[OPENMOTE_NUM_PINS] ;
extern const char openmote_pin_gpio_bit[OPENMOTE_NUM_PINS] ;


#define OPENMOTE_NUM_UARTS 3
extern const char openmote_uart_pins[OPENMOTE_NUM_UARTS][2] ;
extern LPC_UART_TypeDef * const openmote_uart_bank_addresses[OPENMOTE_NUM_UARTS] ;

#define OPENMOTE_NUM_I2C 1
extern const char openmote_i2c_pins[OPENMOTE_NUM_I2C][2] ;
extern LPC_I2C_TypeDef * const openmote_i2c_bank_addresses[OPENMOTE_NUM_I2C] ;

#define OPENMOTE_NUM_SPI 1
extern const char openmote_spi_pins[OPENMOTE_NUM_SPI][3] ;
extern LPC_SSP_TypeDef * const openmote_spi_bank_addresses[OPENMOTE_NUM_SPI]  ;

#define OPENMOTE_NUM_PWM_CHANNELS 7
extern const char openmote_pwm_channels[OPENMOTE_NUM_PINS] ;
extern const char openmote_pwm_pinsel_value[OPENMOTE_NUM_PINS] ;
volatile uint32_t * const openmote_pwm_match_registers[OPENMOTE_NUM_PWM_CHANNELS] ;

extern const char openmote_adc_channels[OPENMOTE_NUM_PINS] ;
extern const char openmote_adc_pinsel_value[OPENMOTE_NUM_PINS] ;

#define DAC_AOUT 19
#define DAC_PINSEL_VALUE 2

#define openmote_pin_function_select(PORT,PIN,VALUE) (&(LPC_PINCON->PINSEL0))[(PORT)/2] = ((&(LPC_PINCON->PINSEL0))[(PORT)/2] & ~(0x3 << (PIN)*2)) | (VALUE << (PIN)*2)
#define openmote_pin_mode_select(PORT,PIN,VALUE) (&(LPC_PINCON->PINMODE0))[(PORT)/2] = ((&(LPC_PINCON->PINMODE0))[(PORT)/2] & ~(0x3 << (PIN)*2)) | (VALUE << (PIN)*2)
#define openmote_pin_open_drain_select(PORT,PIN,VALUE) (&(LPC_PINCON->PINMODE_OD0))[(PORT)] = ((&(LPC_PINCON->PINMODE_OD0))[(PORT)] & ~(0x1 << (PIN))) | (VALUE << (PIN))

#endif
