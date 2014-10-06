//*****************************************************************************
//
//  这部分程序基于IAR EWARM5.30编译环境
// 
// 
//  芯片型号          : STM32F103VB
//  程序类型          : 应用类
//  晶振频率          : 8.000000 MHz
//  内存模式          : 
//  外部SRAM大小      : 
//  数据堆栈大小      : 
//--------------文件信息--------------------------------------------------------------------------------
//  文   件   名: systick.c
//  创   建   人:喻金钱
//  最后修改日期: 2009年01月21日
//  描        述: 本文件定义节拍定时器时钟。
//                
//--------------当前版本信息----------------------------------------------------------------------------
//   创建人: 喻金钱
//   版  本: v0.01
//   日　期: 2009年01月20日
//   描　述: 当前版本
//
//******************************************************************************

#include "stm32f10x_lib.h"



unsigned char systik_i;
unsigned char rtc_sig=1,rtc_play;
void SysTickHandler(void);
//SysTick设置
void SysTick_Config(void)
{
    //失能SysTick定时器
    SysTick_CounterCmd(SysTick_Counter_Disable);
  
    //失能SysTick中断
    SysTick_ITConfig(DISABLE);
  
    //设置SysTick时钟源
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);

    //设置SysTick重载值，1s重载一次，在72Mhz时钟下
    SysTick_SetReload(9000000);

    //开SysTick中断
    SysTick_ITConfig(ENABLE);
    
    //开SysTick定时器
    SysTick_CounterCmd(SysTick_Counter_Enable);
}