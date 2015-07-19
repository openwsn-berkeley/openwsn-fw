/**
 * \file
 *         Device drivers for ADXL346 acceleration sensor in OpenMote-CC2538.
 * \author
 *         Pere Tuset, OpenMote <peretuset@openmote.com>
 */

#ifndef __ADXL346_H__
#define __ADXL346_H__

void adxl346_init(void);
void adxl346_reset(void);
uint8_t adxl346_is_present(void);
int16_t adxl346_read_x(void);
int16_t adxl346_read_y(void);
int16_t adxl346_read_z(void);
float adxl346_convert_acceleration(int16_t acceleration);

#endif /* ifndef __ADXL346_H__ */
