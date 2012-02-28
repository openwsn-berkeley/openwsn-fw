#ifndef OPENMOTE_PINDEFS_H
#define OPENMOTE_PINDEFS_H
#include "LPC17xx.h"


#define NOT_VALID_PIN 0xFF


#define OPENMOTE_NUM_GPIO_BANKS 5
extern const LPC_GPIO_TypeDef * LPC17xx_gpio_bank_addresses[OPENMOTE_NUM_GPIO_BANKS];

#define OPENMOTE_NUM_PINS 20
extern const char openmote_pin_gpio_bank[OPENMOTE_NUM_PINS];
extern const char openmote_pin_gpio_bit[OPENMOTE_NUM_PINS];


#define OPENMOTE_NUM_UARTS 3
extern const char openmote_uart_pins[OPENMOTE_NUM_UARTS][2];
extern const LPC_UART_TypeDef * openmote_uart_bank_addresses[OPENMOTE_NUM_UARTS];

#define OPENMOTE_NUM_I2C 1
extern const char openmote_i2c_pins[OPENMOTE_NUM_I2C][2];
extern const LPC_I2C_TypeDef * openmote_i2c_bank_addresses[OPENMOTE_NUM_I2C];

#define OPENMOTE_NUM_SPI 1
extern const char openmote_spi_pins[OPENMOTE_NUM_SPI][3];
extern const LPC_SSP_TypeDef * openmote_spi_bank_addresses[OPENMOTE_NUM_SPI] ;

extern const char openmote_pwm_channels[OPENMOTE_NUM_PINS] ;
extern const char openmote_adc_channels[OPENMOTE_NUM_PINS] ;

#endif
