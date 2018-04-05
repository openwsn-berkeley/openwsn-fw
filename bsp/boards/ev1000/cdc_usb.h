/**
\brief ev1000 "usb cdc" bsp module.

\author Jean-Michel Rubillon <jmrubillon@theiet.org>, September 2017.
*/

#ifndef _CDC_USB_H
#define _CDC_USB_H

#include "stm32f10x_conf.h"
#include "usb_type.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define         ID1          (0x1FFFF7E8)
#define         ID2          (0x1FFFF7EC)
#define         ID3          (0x1FFFF7F0)

/* Exported macro ------------------------------------------------------------*/
#define USB_DISCONNECT                      GPIOA  
#define USB_DISCONNECT_PIN                  GPIO_Pin_9

/* Exported define -----------------------------------------------------------*/
#define BULK_MAX_PACKET_SIZE  0x00000040

/* Exported functions ------------------------------------------------------- */
void Set_System(void);
void Set_USBClock(void);
void Enter_LowPowerMode(void);
void Leave_LowPowerMode(void);
void USB_Interrupts_Config(void);
void USB_Cable_Config (FunctionalState NewState);
void Get_SerialNum(void);
void LCD_Control(void);
uint32_t CDC_Send_DATA (uint8_t *ptrBuffer, uint8_t Send_length);
uint32_t CDC_Receive_DATA(void);

void usb_init(void);
void USB_SendData( uint8_t *buffer, uint16_t length);
void USB_ReceiveData( uint8_t *buffer, uint16_t maxlength);
void USB_Cable_Config (FunctionalState NewState);
void Get_SerialNum(void);

#endif //_CDC_USB_H
