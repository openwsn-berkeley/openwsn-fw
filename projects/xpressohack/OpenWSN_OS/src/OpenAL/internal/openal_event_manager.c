
#include "openal_event_manager.h"
#include "openal_app_manager.h"
#include "openal_internal_common.h"
#include "events.h"
#include "string.h"

// All the Listeners we can have. Provides app_no
char openal_GPIO_Listeners[OPENMOTE_NUM_PINS];
char openal_GPIO_Listener_type[OPENMOTE_NUM_PINS]; // rising or falling

char openal_UARTA_Listener;
char openal_UARTB_Listener;
char openal_UARTC_Listener;
char openal_SOCK_Listener;
char openal_UDP_Listener;

// The event queues for each app
xQueueHandle openal_event_queues[OPENAL_NUM_APPS];

xSemaphoreHandle event_listener_config_mutex;

// Mutexes and semaphores used by openAL
xSemaphoreHandle adc_conversion_semaphore;

void openal_event_manager_init() {
	char c;
	vSemaphoreCreateBinary(adc_conversion_semaphore);
	event_listener_config_mutex = xSemaphoreCreateMutex();

	memset(openal_GPIO_Listeners,APP_SLOT_EMPTY,sizeof(openal_GPIO_Listeners));
	openal_UARTA_Listener = openal_UARTB_Listener = openal_UARTC_Listener = openal_SOCK_Listener = openal_UDP_Listener = APP_SLOT_EMPTY;
}


/*
 * ISR routines
 */
void ADC_IRQHandler() {
	static signed portBASE_TYPE xHigherPriorityTaskWoken;
	xSemaphoreGiveFromISR(adc_conversion_semaphore, &xHigherPriorityTaskWoken);
}

// handles GPIO0 and GPIO2 interrupts
void EINT3_IRQHandler() {
	char c;
	for (c = 0; c < OPENMOTE_NUM_PINS; c++) {
		if ( openal_GPIO_Listeners[c] !=  APP_SLOT_EMPTY) {
			if ( openmote_pin_gpio_bank[c] == 0 && (LPC_GPIOINT->IntStatus & 1)){
				if (LPC_GPIOINT->IO0IntStatR & (1<<openmote_pin_gpio_bit[c])) {
					// TODO: Implement

				} else if (LPC_GPIOINT->IO0IntStatF & (1<<openmote_pin_gpio_bit[c])) {
					// TODO: Implement
				}
			}
			else if ( openmote_pin_gpio_bank[c] == 2 && (LPC_GPIOINT->IntStatus & 2)){
				if (LPC_GPIOINT->IO2IntStatR & (1<<openmote_pin_gpio_bit[c])) {
					// TODO: Implement
				} else if (LPC_GPIOINT->IO2IntStatF & (1<<openmote_pin_gpio_bit[c])) {
					// TODO: Implement
				}
			}
		}
	}

}
