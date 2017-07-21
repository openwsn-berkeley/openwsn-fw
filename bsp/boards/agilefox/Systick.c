//*****************************************************************************
//
//  �ⲿ�ֳ������IAR EWARM5.30���뻷��
// 
// 
//  оƬ�ͺ�          : STM32F103VB
//  ��������          : Ӧ����
//  ����Ƶ��          : 8.000000 MHz
//  �ڴ�ģʽ          : 
//  �ⲿSRAM��С      : 
//  ���ݶ�ջ��С      : 
//--------------�ļ���Ϣ--------------------------------------------------------------------------------
//  ��   ��   ��: systick.c
//  ��   ��   ��:����Ǯ
//  ����޸�����: 2009��01��21��
//  ��        ��: ���ļ�������Ķ�ʱ��ʱ�ӡ�
//                
//--------------��ǰ�汾��Ϣ----------------------------------------------------------------------------
//   ������: ����Ǯ
//   ��  ��: v0.01
//   �ա���: 2009��01��20��
//   �衡��: ��ǰ�汾
//
//******************************************************************************

#include "stm32f10x_lib.h"



unsigned char systik_i;
unsigned char rtc_sig=1,rtc_play;
void SysTickHandler(void);
//SysTick����
void SysTick_Config(void)
{
    //ʧ��SysTick��ʱ��
    SysTick_CounterCmd(SysTick_Counter_Disable);
  
    //ʧ��SysTick�ж�
    SysTick_ITConfig(DISABLE);
  
    //����SysTickʱ��Դ
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);

    //����SysTick����ֵ��1s����һ�Σ���72Mhzʱ����
    SysTick_SetReload(9000000);

    //��SysTick�ж�
    SysTick_ITConfig(ENABLE);
    
    //��SysTick��ʱ��
    SysTick_CounterCmd(SysTick_Counter_Enable);
}