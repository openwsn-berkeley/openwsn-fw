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
#define UART_ENABLED 1
#define UART_EASY_DMA_SUPPORT 0
#define UART0_ENABLED 1
#define UART0_CONFIG_USE_EASY_DMA 0
#define APP_UART_ENABLED 1
#define APP_UART_DRIVER_INSTANCE 0

// SPI
#define SPI_ENABLED 1         // solely for legacy driver calls (if any)
#define SPI0_ENABLED 1        // solely for legacy driver calls (if any)
#define SPI0_USE_EASY_DMA 0   // solely for legacy driver calls (if any)
#define NRFX_SPI_ENABLED
#define NRFX_SPI0_ENABLED
#define NRFX_SPIM0_ENABLED

// TIMER
// #define TIMER_ENABLED 1
// #define TIMER0_ENABLED 1
// #define TIMER0_USE_EASY_DMA 0

// enable the app_timer library (based on RTC1 [Real Time Counter]) to be used as TIMER
// #define APP_TIMER_ENABLED 1
#define RTC_ENABLED 1         // solely for legacy driver calls (if any)
#define RTC0_ENABLED 1        // solely for legacy driver calls (if any)
#define NRFX_RTC_ENABLED 1
#define NRFX_RTC0_ENABLED 1

// power management 
#define NRF_PWR_MGMT_ENABLED 1

// radio
//#define NRF52

#endif // BSP_NRF52480_SDK_CONFIG_H
