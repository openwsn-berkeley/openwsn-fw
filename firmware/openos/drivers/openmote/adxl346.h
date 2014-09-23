/**
 * \file
 *         Device drivers for ADXL346 acceleration sensor in OpenMote-CC2538.
 * \author
 *         Pere Tuset, OpenMote <peretuset@openmote.com>
 */

#ifndef __ADXL346_H__
#define __ADXL346_H__

void init(void);
void reset(void);
uint8_t is_present(void);
uint16_t read_x(void);
uint16_t read_y(void);
uint16_t read_z(void);

#endif /* ifndef __ADXL346_H__ */

