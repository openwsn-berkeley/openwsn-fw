/**
 * \file
 *         Device drivers for SHT21 temperature and humidity sensor in OpenMote-CC2538.
 * \author
 *         Pere Tuset, OpenMote <peretuset@openmote.com>
 */

#ifndef __SHT21_H__
#define __SHT21_H__

void init(void);
void reset(void);
uint8_t is_present(void);
uint16_t read_temperature(void);
float convert_temperature(uint16_t temperature);
uint16_t read_humidity(void);
float convert_humidity(uint16_t humidity);

#endif /* ifndef __SHT21_H__ */

