#include "stm32f10x_lib.h"

unsigned char systik_i;
unsigned char rtc_sig=1,rtc_play;

void SysTickHandler(void);

void OpenWSN_SysTick_Config(void)
{
  SysTick_Config(9000000);
}
