#include "uarthal.h"
#include "uartstdio.h"
#include "hw_memmap.h"
#include "hw_gpio.h"
#include "ioc.h"
#include "string.h"
#include "hw_ioc.h"
#include <hw_sys_ctrl.h>
#include "sys_ctrl.h"
#include "gpio.h"

#define EXAMPLE_PIN_UART_RXD            GPIO_PIN_1
#define EXAMPLE_PIN_UART_TXD            GPIO_PIN_2
#define EXAMPLE_GPIO_BASE               GPIO_A_BASE


void uartMimsyInit(){
  
       char cThisChar;

    //
    // Set the clocking to run directly from the external crystal/oscillator.
    // (no ext 32k osc, no internal osc)
    //
   // SysCtrlClockSet(false, false, SYS_CTRL_SYSDIV_32MHZ);


    //
    // Enable UART peripheral module
    //
    SysCtrlPeripheralEnable(SYS_CTRL_PERIPH_UART0);

    //
    // Disable UART function
    //
    UARTDisable(UART0_BASE);

    //
    // Disable all UART module interrupts
    //
    UARTIntDisable(UART0_BASE, 0x1FFF);

    //
    // Set IO clock as UART clock source
    //
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    //
    // Map UART signals to the correct GPIO pins and configure them as
    // hardware controlled.
    //
    IOCPinConfigPeriphOutput(EXAMPLE_GPIO_BASE, EXAMPLE_PIN_UART_TXD, IOC_MUX_OUT_SEL_UART0_TXD);
    GPIOPinTypeUARTOutput(EXAMPLE_GPIO_BASE, EXAMPLE_PIN_UART_TXD); 
    IOCPinConfigPeriphInput(EXAMPLE_GPIO_BASE, EXAMPLE_PIN_UART_RXD, IOC_UARTRXD_UART0);
    GPIOPinTypeUARTInput(EXAMPLE_GPIO_BASE, EXAMPLE_PIN_UART_RXD);
     
    //
    // Configure the UART for 115,200, 8-N-1 operation.
    // This function uses SysCtrlClockGet() to get the system clock
    // frequency.  This could be also be a variable or hard coded value
    // instead of a function call.
    //
   // UARTConfigSetExpClk(UART0_BASE, SysCtrlClockGet(), 115200,
                     //   (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                     //    UART_CONFIG_PAR_NONE));
  //  UARTEnable(UART0_BASE);
    UARTStdioInitExpClk(0,115200*2); //adjusted to account for clock difference caused by openwsn clock scheme
    //
    // Put a character to show start of example.  This will display on the
    // terminal.
    //  
  

}

void mimsyPrintf(const char *pcString, ...){
      va_list vaArgP;

    //
    // Start the varargs processing.
    //
    va_start(vaArgP, pcString); 

    UARTvprintf(pcString, vaArgP); 

    //
    // We're finished with the varargs now.
    //
    va_end(vaArgP);
  
}
