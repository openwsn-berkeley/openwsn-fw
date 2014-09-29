#include "derivative.h" /* include peripheral declarations */
#include "user_config.h"

/*--------------------------------------------------------------*/
typedef void (*const tIsrFunc)(void);
typedef struct {
	uint32_t * __ptr;
#ifdef MCU_MK20D5
	tIsrFunc __func[0x3D];
#elif (defined MCU_MK20D7) || (defined MCU_MK40D7)
	tIsrFunc __func[0x6E];
#else
	tIsrFunc __func[0x77];
#endif	
} tVectorTable;

#if (!defined _SERIAL_AGENT_) && !HIGH_SPEED_DEVICE
extern void USB_ISR();
#else
extern void USBHS_ISR();
#endif
#ifdef _USB_BATT_CHG_APP_H_  
	extern void USBDCD_ISR(void);
	extern void VbusDetect_ISR(void);
#endif
#if (defined(_SERIAL_BRIDGE_)|defined(_SERIAL_AGENT_))
	extern void UART3_RTx_ISR(void);
	extern void UART3_Err_ISR(void);
#endif

#ifdef USED_PIT0
	extern void  Timer_ISR(void);
#endif
#ifdef USED_PIT1
	extern void  pit1_isr(void);
#endif
#ifndef _USB_BATT_CHG_APP_H_
	extern void IRQ_ISR_PORTA();
#endif 
#if (defined MCU_MK20D5) || (defined MCU_MK20D7) || (defined MCU_MK40D7) || (defined MCU_MK40N512VMD100) ||  (defined MCU_MK53N512CMD100)
	#if (!(defined MCU_MK20D5)) || (!(defined _USB_BATT_CHG_APP_H_))
		extern void IRQ_ISR_PORTC();
	#endif
#endif
extern void IRQ_ISR_PORTD();
extern void IRQ_ISR_PORTE();
extern void __thumb_startup( void );
extern uint32_t __SP_INIT[];

#pragma define_section vectortable ".vectortable" ".vectortable" ".vectortable" far_abs R

void Cpu_INT_NMIInterrupt(void)
{
	
}

void Cpu_Interrupt(void)
{
	uint8_t vector_num;
	
	vector_num = *(volatile uint8_t*)(0xE000ED04);
	
}

void Cpu_INT_HardFaultInterrupt()
{
	
}

/*lint -save  -e926 -e927 -e928 -e929 Disable MISRA rule (11.4) checking. Need to explicitly cast pointers to the general ISR for Interrupt vector table */

static __declspec(vectortable) tVectorTable __vector_table = { /* Interrupt vector table */
	/* ISR name                             No. Address      Pri Name                           Description */
	(uint32_t *)__SP_INIT,						/* 0x00  0x00000000   -   ivINT_Initial_Stack_Pointer    used by PE */
	{	
	#ifdef MCU_MK20D5
		   (tIsrFunc)__thumb_startup,         /* 0x01  0x00000004   -   ivINT_Initial_Program_Counter  used by PE */
		   (tIsrFunc)Cpu_INT_NMIInterrupt,    /* 0x02  0x00000008   -2   ivINT_NMI                      used by PE */
		   (tIsrFunc)Cpu_INT_HardFaultInterrupt, /* 0x03  0x0000000C   -1   ivINT_Hard_Fault               unused by PE */
		   (tIsrFunc)Cpu_Interrupt,           /* 0x04  0x00000010   -   ivINT_Reserved4                unused by PE */
		   (tIsrFunc)Cpu_Interrupt,           /* 0x05  0x00000014   -   ivINT_Bus_Fault                unused by PE */
		   (tIsrFunc)Cpu_Interrupt,           /* 0x06  0x00000018   -   ivINT_Usage_Fault              unused by PE */
		   (tIsrFunc)Cpu_Interrupt,           /* 0x07  0x0000001C   -   ivINT_Reserved7                unused by PE */
		   (tIsrFunc)Cpu_Interrupt,           /* 0x08  0x00000020   -   ivINT_Reserved8                unused by PE */
		   (tIsrFunc)Cpu_Interrupt,           /* 0x09  0x00000024   -   ivINT_Reserved9                unused by PE */
		   (tIsrFunc)Cpu_Interrupt,           /* 0x0A  0x00000028   -   ivINT_Reserved10               unused by PE */
		   (tIsrFunc)Cpu_Interrupt,           /* 0x0B  0x0000002C   -   ivINT_SVCall                   unused by PE */
		   (tIsrFunc)Cpu_Interrupt,           /* 0x0C  0x00000030   -   ivINT_DebugMonitor             unused by PE */
		   (tIsrFunc)Cpu_Interrupt,           /* 0x0D  0x00000034   -   ivINT_Reserved13               unused by PE */
		   (tIsrFunc)Cpu_Interrupt,           /* 0x0E  0x00000038   -   ivINT_PendableSrvReq           unused by PE */
		   (tIsrFunc)Cpu_Interrupt,           /* 0x0F  0x0000003C   -   ivINT_SysTick                  unused by PE */
		   (tIsrFunc)Cpu_Interrupt,           /* 0x10  0x00000040   -   ivINT_DMA0                     unused by PE */
		   (tIsrFunc)Cpu_Interrupt,           /* 0x11  0x00000044   -   ivINT_DMA1                     unused by PE */
		   (tIsrFunc)Cpu_Interrupt,           /* 0x12  0x00000048   -   ivINT_DMA2                     unused by PE */
		   (tIsrFunc)Cpu_Interrupt,           /* 0x13  0x0000004C   -   ivINT_DMA3                     unused by PE */
		   (tIsrFunc)Cpu_Interrupt,           /* 0x14  0x00000050   -   ivINT_DMA_Error                unused by PE */
		   (tIsrFunc)Cpu_Interrupt,           /* 0x15  0x00000054   -   ivINT_MCM                      unused by PE */
		   (tIsrFunc)Cpu_Interrupt,           /* 0x16  0x00000058   -   ivINT_FTFL                     unused by PE */
		   (tIsrFunc)Cpu_Interrupt,           /* 0x17  0x0000005C   -   ivINT_Read_Collision           unused by PE */
		   (tIsrFunc)Cpu_Interrupt,           /* 0x18  0x00000060   -   ivINT_LVD_LVW                  unused by PE */
		   (tIsrFunc)Cpu_Interrupt,           /* 0x19  0x00000064   -   ivINT_LLW                      unused by PE */
		   (tIsrFunc)Cpu_Interrupt,           /* 0x1A  0x00000068   -   ivINT_Watchdog                 unused by PE */
		   (tIsrFunc)Cpu_Interrupt,           /* 0x1B  0x0000006C   -   ivINT_I2C0                     unused by PE */
		   (tIsrFunc)Cpu_Interrupt,           /* 0x1C  0x00000070   -   ivINT_SPI0                     unused by PE */
		   (tIsrFunc)Cpu_Interrupt,           /* 0x1D  0x00000074   -   ivINT_I2S0                     unused by PE */
		   (tIsrFunc)Cpu_Interrupt,           /* 0x1E  0x00000078   -   ivINT_I2S1                     unused by PE */
		   (tIsrFunc)Cpu_Interrupt,           /* 0x1F  0x0000007C   -   ivINT_UART0_LON                unused by PE */
		   (tIsrFunc)Cpu_Interrupt,           /* 0x20  0x00000080   -   ivINT_UART0_RX_TX              unused by PE */
		   (tIsrFunc)Cpu_Interrupt,           /* 0x21  0x00000084   -   ivINT_UART0_ERR                unused by PE */
		   (tIsrFunc)Cpu_Interrupt,           /* 0x22  0x00000088   -   ivINT_UART1_RX_TX              unused by PE */
		   (tIsrFunc)Cpu_Interrupt,           /* 0x23  0x0000008C   -   ivINT_UART1_ERR                unused by PE */
		   (tIsrFunc)Cpu_Interrupt,           /* 0x24  0x00000090   -   ivINT_UART2_RX_TX              unused by PE */
		   (tIsrFunc)Cpu_Interrupt,           /* 0x25  0x00000094   -   ivINT_UART2_ERR                unused by PE */
		   (tIsrFunc)Cpu_Interrupt,           /* 0x26  0x00000098   -   ivINT_ADC0                     unused by PE */
		   (tIsrFunc)Cpu_Interrupt,           /* 0x27  0x0000009C   -   ivINT_HSCMP0                   unused by PE */
		   (tIsrFunc)Cpu_Interrupt,           /* 0x28  0x000000A0   -   ivINT_HSCMP1                   unused by PE */
		   (tIsrFunc)Cpu_Interrupt,           /* 0x29  0x000000A4   -   ivINT_FTM0                     unused by PE */
		   (tIsrFunc)Cpu_Interrupt,           /* 0x2A  0x000000A8   -   ivINT_FTM1                     unused by PE */
		   (tIsrFunc)Cpu_Interrupt,           /* 0x2B  0x000000AC   -   ivINT_CMT                      unused by PE */
		   (tIsrFunc)Cpu_Interrupt,           /* 0x2C  0x000000B0   -   ivINT_RTC                      unused by PE */
		   (tIsrFunc)Cpu_Interrupt,           /* 0x2D  0x000000B4   -   ivINT_RTC_SecontInt            unused by PE */
		#ifdef USED_PIT0
		   (tIsrFunc)Timer_ISR,           	  /* 0x2E  0x000000B8   -   ivINT_PIT0                     Timer_ISR    */
		#else 
		   (tIsrFunc)Cpu_Interrupt,           /* 0x2E  0x000000B8   -   ivINT_PIT0                     unused by PE */
		#endif
		#ifdef USED_PIT1
		   (tIsrFunc)pit1_isr,           	  /* 0x2F  0x000000BC   -   ivINT_PIT1                     pit1_isr     */
		#else 
		   (tIsrFunc)Cpu_Interrupt,           /* 0x2F  0x000000BC   -   ivINT_PIT1                     unused by PE */
		#endif
		   (tIsrFunc)Cpu_Interrupt,           /* 0x30  0x000000C0   -   ivINT_PIT2                     unused by PE */
		   (tIsrFunc)Cpu_Interrupt,           /* 0x31  0x000000C4   -   ivINT_PIT3                     unused by PE */
		   (tIsrFunc)Cpu_Interrupt,           /* 0x32  0x000000C8   -   ivINT_PDB0                     unused by PE */
		   (tIsrFunc)USB_ISR,           	  /* 0x33  0x000000CC   -   ivINT_USB0                     USB_ISR      */
		#ifdef _USB_BATT_CHG_APP_H_  
			(tIsrFunc)USBDCD_ISR,		   	  /* 0x34  0x000000D0   -   ivINT_USBDCD                   USBDCD_ISR   */
		#else		   
		   (tIsrFunc)Cpu_Interrupt,           /* 0x34  0x000000D0   -   ivINT_USBDCD                   unused by PE */
		#endif   
		   (tIsrFunc)Cpu_Interrupt,           /* 0x35  0x000000D4   -   ivINT_TSI                      unused by PE */
		   (tIsrFunc)Cpu_Interrupt,           /* 0x36  0x000000D8   -   ivINT_MCG                      unused by PE */
		#ifdef USE_LPT
		   (tIsrFunc)Timer_ISR,               /* 0x37  0x000000DC   -   ivINT_LPTimer                  Timer_ISR    */
		#else
		   (tIsrFunc)Cpu_Interrupt,           /* 0x37  0x000000DC   -   ivINT_LPTimer                  unused by PE */
		#endif
		#ifdef USE_IRQ
		   (tIsrFunc)IRQ_ISR_PORTA,           /* 0x38  0x000000E0   -   ivINT_PORTA                    IRQ_ISR_PORTA */
		#else
		   (tIsrFunc)Cpu_Interrupt,           /* 0x38  0x000000E0   -   ivINT_PORTA                    unused by PE */
		#endif
		   (tIsrFunc)Cpu_Interrupt,           /* 0x39  0x000000E4   -   ivINT_PORTB                    unused by PE */		
		#ifdef _USB_BATT_CHG_APP_H_ 
		   (tIsrFunc)VbusDetect_ISR,           /* 0x3A  0x000000E8   -   ivINT_PORTC                   VbusDetect_ISR */
		#else		   
		   (tIsrFunc)IRQ_ISR_PORTC,           /* 0x3A  0x000000E8   -   ivINT_PORTC                	   IRQ_ISR_PORTC */
		#endif
		   (tIsrFunc)Cpu_Interrupt,           /* 0x3B  0x000000EC   -   ivINT_PORTD                    unused by PE */
		   (tIsrFunc)IRQ_ISR_PORTE,           /* 0x3C  0x000000F0   -   ivINT_PORTE                    IRQ_ISR_PORTE */
		   (tIsrFunc)Cpu_Interrupt,           /* 0x3D  0x000000F4   -   ivINT_Reserved61               unused by PE */			
	#else			
			(tIsrFunc)__thumb_startup,				/* 0x01  0x00000004   -   ivINT_Initial_Program_Counter  used by PE */
			(tIsrFunc)Cpu_INT_NMIInterrupt,			/* 0x02  0x00000008   -2   ivINT_NMI                     used by PE */
			(tIsrFunc)Cpu_INT_HardFaultInterrupt,	/* 0x03  0x0000000C   -1   ivINT_Hard_Fault              unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x04  0x00000010   -   ivINT_Reserved4                unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x05  0x00000014   -   ivINT_Bus_Fault                unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x06  0x00000018   -   ivINT_Usage_Fault              unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x07  0x0000001C   -   ivINT_Reserved7                unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x08  0x00000020   -   ivINT_Reserved8                unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x09  0x00000024   -   ivINT_Reserved9                unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x0A  0x00000028   -   ivINT_Reserved10               unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x0B  0x0000002C   -   ivINT_SVCall                   unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x0C  0x00000030   -   ivINT_DebugMonitor             unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x0D  0x00000034   -   ivINT_Reserved13               unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x0E  0x00000038   -   ivINT_PendableSrvReq           unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x0F  0x0000003C   -   ivINT_SysTick                  unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x10  0x00000040   -   ivINT_DMA0                     unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x11  0x00000044   -   ivINT_DMA1                     unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x12  0x00000048   -   ivINT_DMA2                     unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x13  0x0000004C   -   ivINT_DMA3                     unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x14  0x00000050   -   ivINT_DMA4                     unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x15  0x00000054   -   ivINT_DMA5                     unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x16  0x00000058   -   ivINT_DMA6                     unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x17  0x0000005C   -   ivINT_DMA7                     unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x18  0x00000060   -   ivINT_DMA8                     unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x19  0x00000064   -   ivINT_DMA9                     unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x1A  0x00000068   -   ivINT_DMA10                    unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x1B  0x0000006C   -   ivINT_DMA11                    unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x1C  0x00000070   -   ivINT_DMA12                    unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x1D  0x00000074   -   ivINT_DMA13                    unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x1E  0x00000078   -   ivINT_DMA14                    unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x1F  0x0000007C   -   ivINT_DMA15                    unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x20  0x00000080   -   ivINT_DMA_Error                unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x21  0x00000084   -   ivINT_MCM                      unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x22  0x00000088   -   ivINT_FTFL                     unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x23  0x0000008C   -   ivINT_Read_Collision           unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x24  0x00000090   -   ivINT_LVD_LVW                  unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x25  0x00000094   -   ivINT_LLW                      unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x26  0x00000098   -   ivINT_Watchdog                 unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x27  0x0000009C   -   ivINT_Reserved39               unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x28  0x000000A0   -   ivINT_I2C0                     unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x29  0x000000A4   -   ivINT_I2C1                     unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x2A  0x000000A8   -   ivINT_SPI0                     unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x2B  0x000000AC   -   ivINT_SPI1                     unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x2C  0x000000B0   -   ivINT_SPI2                     unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x2D  0x000000B4   -   ivINT_CAN0_ORed_Message_buffer unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x2E  0x000000B8   -   ivINT_CAN0_Bus_Off             unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x2F  0x000000BC   -   ivINT_CAN0_Error               unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x30  0x000000C0   -   ivINT_CAN0_Tx_Warning          unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x31  0x000000C4   -   ivINT_CAN0_Rx_Warning          unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x32  0x000000C8   -   ivINT_CAN0_Wake_Up             unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x33  0x000000CC   -   ivINT_CAN0_IMEU                unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x34  0x000000D0   -   ivINT_CAN0_Lost_Rx             unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x35  0x000000D4   -   ivINT_CAN1_ORed_Message_buffer unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x36  0x000000D8   -   ivINT_CAN1_Bus_Off             unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x37  0x000000DC   -   ivINT_CAN1_Error               unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x38  0x000000E0   -   ivINT_CAN1_Tx_Warning          unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x39  0x000000E4   -   ivINT_CAN1_Rx_Warning          unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x3A  0x000000E8   -   ivINT_CAN1_Wake_Up             unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x3B  0x000000EC   -   ivINT_CAN1_IMEU                unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x3C  0x000000F0   -   ivINT_CAN1_Lost_Rx             unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x3D  0x000000F4   -   ivINT_UART0_RX_TX              unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x3E  0x000000F8   -   ivINT_UART0_ERR                unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x3F  0x000000FC   -   ivINT_UART1_RX_TX              unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x40  0x00000100   -   ivINT_UART1_ERR                unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x41  0x00000104   -   ivINT_UART2_RX_TX              unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x42  0x00000108   -   ivINT_UART2_ERR                unused by PE */
		#if (defined(_SERIAL_BRIDGE_)|defined(_SERIAL_AGENT_))  
			(tIsrFunc)UART3_RTx_ISR,				/* 0x43  0x0000010C   -   ivINT_UART3_RX_TX              unused by PE */
			(tIsrFunc)UART3_Err_ISR,				/* 0x44  0x00000110   -   ivINT_UART3_ERR                unused by PE */
		#else
			(tIsrFunc)Cpu_Interrupt,				/* 0x43  0x0000010C   -   ivINT_UART3_RX_TX              unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x44  0x00000110   -   ivINT_UART3_ERR                unused by PE */
		#endif      
			(tIsrFunc)Cpu_Interrupt,				/* 0x45  0x00000114   -   ivINT_UART4_RX_TX              unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x46  0x00000118   -   ivINT_UART4_ERR                unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x47  0x0000011C   -   ivINT_UART5_RX_TX              unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x48  0x00000120   -   ivINT_UART5_ERR                unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x49  0x00000124   -   ivINT_ADC0                     unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x4A  0x00000128   -   ivINT_ADC1                     unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x4B  0x0000012C   -   ivINT_HSCMP0                   unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x4C  0x00000130   -   ivINT_HSCMP1                   unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x4D  0x00000134   -   ivINT_HSCMP2                   unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x4E  0x00000138   -   ivINT_FTM0                     unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x4F  0x0000013C   -   ivINT_FTM1                     unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x50  0x00000140   -   ivINT_FTM2                     unused by PE */
		#ifdef _CMT_H
			(tIsrFunc)cmt_isr,						/* 0x51  0x00000144   -   ivINT_CMT                      unused by PE */
		#else
			(tIsrFunc)Cpu_Interrupt,				/* 0x51  0x00000144   -   ivINT_CMT                      unused by PE */
		#endif
			(tIsrFunc)Cpu_Interrupt,				/* 0x52  0x00000148   -   ivINT_RTC                      unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x53  0x0000014C   -   ivINT_Reserved83               unused by PE */
		#ifdef USED_PIT0
			(tIsrFunc)Timer_ISR,					/* 0x54  0x00000150   -   ivINT_PIT0                     unused by PE */
		#else
			(tIsrFunc)Cpu_Interrupt,				/* 0x54  0x00000150   -   ivINT_PIT0                     unused by PE */
		#endif
		#ifdef USED_PIT1
			(tIsrFunc)pit1_isr,						/* 0x55  0x00000154   -   ivINT_PIT1                     unused by PE */
		#else
			(tIsrFunc)Cpu_Interrupt,				/* 0x55  0x00000154   -   ivINT_PIT1                     unused by PE */
		#endif
			(tIsrFunc)Cpu_Interrupt,				/* 0x56  0x00000158   -   ivINT_PIT2                     unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x57  0x0000015C   -   ivINT_PIT3                     unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x58  0x00000160   -   ivINT_PDB0                     unused by PE */
		#if !defined _SERIAL_AGENT_ && !HIGH_SPEED_DEVICE
			(tIsrFunc)USB_ISR,						/* 0x59  0x00000164   -   ivINT_USB0                     unused by PE */
		#else
			(tIsrFunc)Cpu_Interrupt,				/* 0x59  0x00000164   -   ivINT_USB0                     unused by PE */
		#endif
		#ifdef _USB_BATT_CHG_APP_H_  
			(tIsrFunc)USBDCD_ISR,					/* 0x5A  0x00000168   -   ivINT_USBDCD                   USBDCD_ISR */
		#else
			(tIsrFunc)Cpu_Interrupt,				/* 0x5A  0x00000168   -   ivINT_USBDCD                   unused by PE */   
		#endif 
			(tIsrFunc)Cpu_Interrupt,				/* 0x5B  0x0000016C   -   ivINT_Reserved91               unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x5C  0x00000170   -   ivINT_Reserved92               unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x5D  0x00000174   -   ivINT_Reserved93               unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x5E  0x00000178   -   ivINT_Reserved94               unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x5F  0x0000017C   -   ivINT_I2S0                     unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x60  0x00000180   -   ivINT_SDHC                     unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x61  0x00000184   -   ivINT_DAC0                     unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x62  0x00000188   -   ivINT_DAC1                     unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x63  0x0000018C   -   ivINT_TSI0                     unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x64  0x00000190   -   ivINT_MCG                      unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x65  0x00000194   -   ivINT_LPTimer                  unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x66  0x00000198   -   ivINT_LCD                      unused by PE */
		#ifndef _USB_BATT_CHG_APP_H_   
			(tIsrFunc)IRQ_ISR_PORTA,				/* 0x67  0x0000019C   -   ivINT_PORTA                    unused by PE */
		#elif (defined MCU_MK20D7) || (defined MCU_MK40D7) || (defined MCU_MK40D7) || (defined MCU_MK60N512VMD100)
			(tIsrFunc)VbusDetect_ISR,				/* 0x67  0x0000019C   -   ivINT_PORTA                    unused by PE */
		#else
			(tIsrFunc)Cpu_Interrupt,				/* 0x67  0x0000019C   -   ivINT_PORTA                    unused by PE */
		#endif
		#if (defined _USB_BATT_CHG_APP_H_) && ((defined MCU_MK40N512VMD100) ||  (defined MCU_MK53N512CMD100) || (defined MCU_MK70F12)) 
			(tIsrFunc)VbusDetect_ISR,				/* 0x68  0x000001A0   -   ivINT_PORTB                    unused by PE */   
		#else
			(tIsrFunc)Cpu_Interrupt,				/* 0x68  0x000001A0   -   ivINT_PORTB                    unused by PE */
		#endif
		#if (defined MCU_MK20D7) || (defined MCU_MK40D7) || (defined MCU_MK40N512VMD100) || (defined MCU_MK53N512CMD100)
			(tIsrFunc)IRQ_ISR_PORTC,				/* 0x69  0x000001A4   -   ivINT_PORTC                    unused by PE */
		#else
			(tIsrFunc)Cpu_Interrupt,				/* 0x69  0x000001A4   -   ivINT_PORTC                    unused by PE */
		#endif
			(tIsrFunc)IRQ_ISR_PORTD,				/* 0x6A  0x000001A8   -   ivINT_PORTD                    unused by PE */
			(tIsrFunc)IRQ_ISR_PORTE,				/* 0x6B  0x000001AC   -   ivINT_PORTE                    unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x6C  0x000001B0   -   ivINT_Reserved108              unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x6D  0x000001B4   -   ivINT_Reserved109              unused by PE */
			(tIsrFunc)Cpu_Interrupt,				/* 0x6E  0x000001B8   -   ivINT_Reserved110              unused by PE */
		#if (!(defined MCU_MK20D7)) && (!(defined MCU_MK40D7))
				(tIsrFunc)Cpu_Interrupt,				/* 0x6F  0x000001BC   -   ivINT_Reserved111              unused by PE */
		#if HIGH_SPEED_DEVICE
				(tIsrFunc)USBHS_ISR,					/* 0x70  0x000001C0   -   USBHS */
			#else
				(tIsrFunc)Cpu_Interrupt,				/* 0x70  0x000001C0   -   ivINT_Reserved112              unused by PE */
			#endif
				(tIsrFunc)Cpu_Interrupt,				/* 0x71  0x000001C4   -   ivINT_Reserved113              unused by PE */
				(tIsrFunc)Cpu_Interrupt,				/* 0x72  0x000001C8   -   ivINT_Reserved114              unused by PE */
				(tIsrFunc)Cpu_Interrupt,				/* 0x73  0x000001CC   -   ivINT_Reserved115              unused by PE */
				(tIsrFunc)Cpu_Interrupt,				/* 0x74  0x000001D0   -   ivINT_Reserved116              unused by PE */
				(tIsrFunc)Cpu_Interrupt,				/* 0x75  0x000001D4   -   ivINT_Reserved117              unused by PE */
				(tIsrFunc)Cpu_Interrupt,				/* 0x76  0x000001D8   -   ivINT_Reserved118              unused by PE */
				(tIsrFunc)Cpu_Interrupt					/* 0x77  0x000001DC   -   ivINT_Reserved119              unused by PE */
		#endif
	#endif
	}
};
