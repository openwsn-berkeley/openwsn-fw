#include "compiler.h"
#include "rtc_count.h"
#include "rtc_count_interrupt.h"
#include "rtc_count.h"
#include "bsp_rtc.h"
#include "radiotimer.h"

//struct rtc_module rtc_instance;
//
//void configure_rtc_count(void)
//{	
	//struct rtc_count_config config_rtc_count;
	//rtc_count_get_config_defaults(&config_rtc_count);
//
	//config_rtc_count.prescaler           = RTC_COUNT_PRESCALER_DIV_1;
	//config_rtc_count.mode                = RTC_COUNT_MODE_16BIT;
	//config_rtc_count.continuously_update = true;
//
	//rtc_count_init(&rtc_instance, RTC, &config_rtc_count);	
	//rtc_count_enable(&rtc_instance);
	//configure_rtc_callbacks();
	//rtc_count_set_period(&rtc_instance, 0xFFFF);	
//}
//
//void configure_rtc_callbacks(void)
//{
	//rtc_count_register_callback(&rtc_instance, rtc_overflow_callback, RTC_COUNT_CALLBACK_OVERFLOW);
	//rtc_count_register_callback(&rtc_instance, rtc_compare0_callback, RTC_COUNT_CALLBACK_COMPARE_0);	
//}
//
//void enable_rtc_overflow_callback(void)
//{
	//rtc_count_enable_callback(&rtc_instance, RTC_COUNT_CALLBACK_OVERFLOW);
//}
//
//void disable_rtc_overflow_callback(void)
//{
	//rtc_count_disable_callback(&rtc_instance, RTC_COUNT_CALLBACK_OVERFLOW);
//}
//
//void enable_rtc_compare0_callback(void)
//{
	//rtc_count_enable_callback(&rtc_instance, RTC_COUNT_CALLBACK_COMPARE_0);
//}
//
//void disable_rtc_compare0_callback(void)
//{
	//rtc_count_disable_callback(&rtc_instance, RTC_COUNT_CALLBACK_COMPARE_0);
//}
//
//void rtc_set_count(uint32_t rtc_count)
//{
	//rtc_count_set_count(&rtc_instance, rtc_count);
//}
//
//uint32_t rtc_get_count(void)
//{
	//return(rtc_count_get_count(&rtc_instance));
//}
//
//void rtc_set_compare(uint32_t compare_value, uint8_t comp_index)
//{
	//rtc_count_set_compare(&rtc_instance, compare_value, comp_index);
//}
//
//void rtc_get_compare(uint8_t comp_index, uint32_t *compare_val)
//{
	//rtc_count_get_compare(&rtc_instance, compare_val, comp_index);
//}
//
//void rtc_overflow_callback(void)
//{
	///* Do something on RTC overflow here */
//}
//
//void rtc_compare0_callback(void)
//{
	///* RTC Compare callback */
	//radiotimer_isr();
//}
