/**
 * \file
 *         Device drivers for SI70X temperature and humidity sensor in OpenMote-B.
 * \author
 *         Pere Tuset, OpenMote <peretuset@openmote.com>
 */

#ifndef __SI70X_H__
#define __SI70X_H__

void si70x_init(void);
void si70x_reset(void);
uint8_t si70x_is_present(void);
uint16_t si70x_read_temperature(void);
float si70x_convert_temperature(uint16_t temperature);
uint16_t si70x_read_humidity(void);
float si70x_convert_humidity(uint16_t humidity);

#endif /* ifndef __SI70X_H__ */

