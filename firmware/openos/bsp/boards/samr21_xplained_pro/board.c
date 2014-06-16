/* === INCLUDES ============================================================ */
#include "compiler.h"
#include "system.h"
#include "system_interrupt.h"
#include "port.h"
#include "samr21_xplained_pro.h"
#include "cycle_counter.h"
#include "board.h"
#include "uart.h"
#include "opentimers.h"
#include "radiotimer.h"
#include "radio.h"
#include "bsp_timer.h"
#include "delay.h"
#include "spi.h"
#include "debugpins.h"
#include "leds.h"




/* === MACROS ============================================================== */

/* TRX Parameter: t10 */
#define RST_PULSE_WIDTH_US                          (10)

/* TRX Parameter: tTR1 typical value */
#define P_ON_TO_CLKM_AVAILABLE_TYP_US               (330)

/* === GLOBALS ============================================================= */

extern int mote_main();

void rf_interface_init(void);

/*
 * @brief Main entry functions. 
 *
 * @param returns the value of returns by mote_main
 *
 */ 
int main(void)
{
	SystemInit();
	return mote_main();
}

/*
 * @brief board_init will initialize the delay functions,
 *        debug pins in the board, Radio External Interrupt functions,
 *        SPI Related, UART and timers with default value.
 * @param None
 *
 */
void board_init(void)
{
 /* initialize the interrupt vectors */
 irq_initialize_vectors();
 /* Disable the irq before initialization */
 cpu_irq_disable();
 
 /*  Initialize the Clock */
 delay_init();
 system_init(); 
 
 /* Configure the Debug Pins */
 debugpins_init(); 
 
 /* Configure the Radio Interrupt */	
 AT86RFX_INTC_INIT();
 /* Clear the Radio Interrupt */
 CLEAR_TRX_IRQ();

/*  Initialize board hardware
	SPI Init  Configure the Radio Pins and  Like RST, SLP_TR, EXTI(IRQ on Rising Edge)
	Configure the Board related stuffs and GPIO's
	LED's Init 
 */
 rf_interface_init();
 
 /* Radio Init */
 radio_init();
  
 /* UART Init */
 uart_init();  
 
 /* BSP Timer Init  --Init the RTC before BSP Timer*/
 bsp_timer_init(); 
 
 
 /* Radio Timer Init */
 radiotimer_init();
 
 /* Clear the Radio Interrupt */
 ENABLE_TRX_IRQ();
 /* Enable the IRQ */
 cpu_irq_enable();
}

/*
 * @brief rf_interface_init will initialize the SPI and RFCTRL,
 *        This function will also Reset and initialize the Radio 
 *        to default state
 *
 * @param None
 *
 */
void rf_interface_init(void)
{
	/* Get the port default config to structure */
	struct port_config pin_conf;
	port_get_config_defaults(&pin_conf);

	/* Configure LEDs as outputs, turn them off */
	pin_conf.direction  = PORT_PIN_DIR_OUTPUT;
	port_pin_set_config(LED_0_PIN, &pin_conf);
	port_pin_set_output_level(LED_0_PIN, LED_0_INACTIVE);

	/* Set buttons as inputs */
	pin_conf.direction  = PORT_PIN_DIR_INPUT;
	pin_conf.input_pull = PORT_PIN_PULL_UP;
	port_pin_set_config(BUTTON_0_PIN, &pin_conf);
	
	/* Configure the RF233 SPI Interface */
	port_get_config_defaults(&pin_conf);
	pin_conf.direction  = PORT_PIN_DIR_OUTPUT;
	port_pin_set_config(AT86RFX_SPI_SCK, &pin_conf);
	port_pin_set_config(AT86RFX_SPI_MOSI, &pin_conf);
	port_pin_set_config(AT86RFX_SPI_CS, &pin_conf);
	port_pin_set_config(AT86RFX_RST_PIN, &pin_conf);
	port_pin_set_config(AT86RFX_SLP_PIN, &pin_conf);
	port_pin_set_output_level(AT86RFX_SPI_SCK, true);
	port_pin_set_output_level(AT86RFX_SPI_MOSI, true);
	port_pin_set_output_level(AT86RFX_SPI_CS, true);
	port_pin_set_output_level(AT86RFX_RST_PIN, true);
	port_pin_set_output_level(AT86RFX_SLP_PIN, true);
	
	/* Enable the RF233 Internal interface with SAMR21 */
	pin_conf.direction  = PORT_PIN_DIR_INPUT;
	port_pin_set_config(AT86RFX_SPI_MISO, &pin_conf);
	PM->APBCMASK.reg |= (1<<PM_APBCMASK_RFCTRL_Pos);
	REG_RFCTRL_FECFG = RFCTRL_CFG_ANT_DIV;
	struct system_pinmux_config config_pinmux;
	system_pinmux_get_config_defaults(&config_pinmux);
	config_pinmux.mux_position = MUX_PA09F_RFCTRL_FECTRL1 ;
	config_pinmux.direction    = SYSTEM_PINMUX_PIN_DIR_OUTPUT;
	system_pinmux_pin_set_config(PIN_RFCTRL1, &config_pinmux);
	system_pinmux_pin_set_config(PIN_RFCTRL2, &config_pinmux);		

	spi_init();
		
	/* Wait typical time of timer TR1. */
	delay_us(P_ON_TO_CLKM_AVAILABLE_TYP_US);
		
	/* Initialize the transceiver */
	PORT_PIN_RADIO_RESET_HIGH();
	PORT_PIN_RADIO_SLP_TR_CNTL_LOW();

	/* Wait typical time of timer TR1. */
	delay_us(P_ON_TO_CLKM_AVAILABLE_TYP_US);

	/* Apply reset pulse */
	PORT_PIN_RADIO_RESET_LOW();
	delay_us(RST_PULSE_WIDTH_US);
	PORT_PIN_RADIO_RESET_HIGH();
}

/*
 * @brief board_sleep This function will prepare the board to sleep
 *        Before entering to sleep the MCU will enable the wakeup source
 *        and set the appropriate possible power saving mode in the MCU.
 *
 * @param None
 *
 */
void board_sleep(void)
{
 /* Enter into sleep mode and disable the MCU and other peripherals */
 system_interrupt_enable_global(); 
 /* Set sleep mode to STANDBY */
 system_set_sleepmode(SYSTEM_SLEEPMODE_STANDBY);

 /* Stay in STANDBY sleep until low voltage is detected */
 system_sleep();
}

/* 
 * @brief This will reset the MCU and board to default state
 *
 * @param None
 *
 */
void board_reset(void)
{
 /* No Handlers added */
}

/* 
 * @brief TRX END and other Transceiver Interrupt 
 *        Handler for AT86RFX
 */
void AT86RFX_ISR(void)
{
	debugpins_isr_set();
	CLEAR_TRX_IRQ();	
	radio_isr();			
	debugpins_isr_clr();
}


