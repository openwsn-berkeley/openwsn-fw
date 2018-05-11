/**
 * Author: Tamas Harczos (tamas.harczos@imms.de)
 * Date:   Apr 2018
 * Description: nRF52840-specific nRF52 SDK configuration definitions.
 */ 

#ifndef BSP_NRF52480_SDK_CONFIG_H
#define BSP_NRF52480_SDK_CONFIG_H

// clock 
#define CLOCK_ENABLED 1
#define NRFX_CLOCK_ENABLED 1

// UART 
#define UART_EASY_DMA_SUPPORT 0
#define UART0_ENABLED 1
#define UART0_CONFIG_USE_EASY_DMA 0

// SPI
//#define NRFX_SPI_ENABLED
//#define NRFX_SPI0_ENABLED
//#define NRFX_SPIM0_ENABLED
#define SPI_ENABLED
#define SPI0_ENABLED 1
#define SPI0_USE_EASY_DMA 0

// TIMER
#define TIMER_ENABLED
#define TIMER0_ENABLED 1
#define TIMER0_USE_EASY_DMA 0

// power management 
#define NRF_PWR_MGMT_ENABLED 1


#endif // BSP_NRF52480_SDK_CONFIG_H
