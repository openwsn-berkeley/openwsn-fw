/**
 * \file
 *         Device drivers for SHT11 temperature and humidity sensor in Telosb.
 * \author
 *         Pedro Henrique Gomes <pedrohenriquegomes@gmail.com>
 */

#ifndef __SHT11_H__
#define __SHT11_H__

void sht11_init(void);
void sht11_reset(void);
uint8_t sht11_is_present(void);
uint16_t sht11_read_temperature(void);
float sht11_convert_temperature(uint16_t temperature);
uint16_t sht11_read_humidity(void);
float sht11_convert_humidity(uint16_t humidity);

#endif /* ifndef __SHT11_H__ */

