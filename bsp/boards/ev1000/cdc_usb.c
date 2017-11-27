/**
\brief ev1000 "usb cdc" bsp module.

\author Jean-Michel Rubillon <jmrubillon@theiet.org>, September 2017.
*/
#include "board_info.h"

#ifdef EV1000_USB
#include "cdc_usb.h"
#include "usb_conf.h"
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_pwr.h"

__IO uint8_t Receive_Buffer[64];
__IO uint32_t Receive_length ;
__IO uint32_t length ;
uint8_t Send_Buffer[64];
uint32_t packet_sent=1;
uint32_t packet_receive=1;

//=========================== defines =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================
static void IntToUnicode (uint32_t value , uint8_t *pbuf , uint8_t len);
//=========================== public ==========================================

void usb_init(void){    
	GPIO_InitTypeDef  GPIO_InitStructure;  
	EXTI_InitTypeDef EXTI_InitStructure;
	
// Configure pins for USB port
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

// USB disconnect pin
	GPIO_InitStructure.GPIO_Pin = USB_DISCONNECT_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_Init(USB_DISCONNECT, &GPIO_InitStructure);

	EXTI_ClearITPendingBit(EXTI_Line18);
	EXTI_InitStructure.EXTI_Line = EXTI_Line18;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
}

uint32_t CDC_Send_DATA (uint8_t *ptrBuffer, uint8_t Send_length)
{
  /*if max buffer is Not reached*/
  if(Send_length < VIRTUAL_COM_PORT_DATA_SIZE)     
  {
    /*Sent flag*/
    packet_sent = 0;
    /* send  packet to PMA*/
    UserToPMABufferCopy((unsigned char*)ptrBuffer, ENDP1_TXADDR, Send_length);
    SetEPTxCount(ENDP1, Send_length);
    SetEPTxValid(ENDP1);
  }
  else
  {
    return 0;
  } 
  return 1;
}

uint32_t CDC_Receive_DATA(void)
{ 
  /*Receive flag*/
  packet_receive = 0;
  SetEPRxValid(ENDP3); 
  return 1 ;
}

void USB_SendData( uint8_t *buffer, uint16_t length){
	CDC_Send_DATA (buffer, length);	
}

void USB_ReceiveData( uint8_t *buffer, uint16_t maxlength){
	uint16_t length = maxlength < Receive_length?maxlength:Receive_length;
	uint16_t index;
	CDC_Receive_DATA();
	for(index = 0; index < length; index++){
		buffer[index] = Receive_Buffer[index];
	}
}
void Get_SerialNum(void)
{
  uint32_t Device_Serial0, Device_Serial1, Device_Serial2;

  Device_Serial0 = *(uint32_t*)ID1;
  Device_Serial1 = *(uint32_t*)ID2;
  Device_Serial2 = *(uint32_t*)ID3;
 
  Device_Serial0 += Device_Serial2;

  if (Device_Serial0 != 0)
  {
    IntToUnicode (Device_Serial0, &Virtual_Com_Port_StringSerial[2] , 8);
    IntToUnicode (Device_Serial1, &Virtual_Com_Port_StringSerial[18], 4);
  }
}

static void IntToUnicode (uint32_t value , uint8_t *pbuf , uint8_t len)
{
  uint8_t idx = 0;
  
  for( idx = 0 ; idx < len ; idx ++)
  {
    if( ((value >> 28)) < 0xA )
    {
      pbuf[ 2* idx] = (value >> 28) + '0';
    }
    else
    {
      pbuf[2* idx] = (value >> 28) + 'A' - 10; 
    }
    
    value = value << 4;
    
    pbuf[ 2* idx + 1] = 0;
  }
}

void USB_Cable_Config (FunctionalState NewState)
{
  if (NewState != DISABLE)
  {
    GPIO_ResetBits(USB_DISCONNECT, USB_DISCONNECT_PIN);
  }
  else
  {
    GPIO_SetBits(USB_DISCONNECT, USB_DISCONNECT_PIN);
  }
}


#endif //EV1000_USB
