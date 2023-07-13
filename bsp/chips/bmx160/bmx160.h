/**
\brief registers address mapping of bmx160 sensor.

\author Tengfei Chang <tengfei.chang@gmail.com>, Nov 2021.
*/

#include "stdint.h"

//=========================== define ==========================================

#define BMX160_ADDR 0x68

//---- register addresses

#define BMX160_REG_ADDR_CHIPID      0x00
#define BMX160_REG_ADDR_ERR_REG     0x02
#define BMX160_REG_ADDR_PWU_STATUS  0x03

// sensor data

#define BMX160_REG_ADDR_DATA        0x04

#define BMX160_REG_ADDR_MAG_X_L     0x04
#define BMX160_REG_ADDR_MAG_X_H     0x05
#define BMX160_REG_ADDR_MAG_Y_L     0x06
#define BMX160_REG_ADDR_MAG_Y_H     0x07
#define BMX160_REG_ADDR_MAG_Z_L     0x08
#define BMX160_REG_ADDR_MAG_Z_H     0x09
#define BMX160_REG_ADDR_RHALL_L     0x0a
#define BMX160_REG_ADDR_RHALL_H     0x0b
#define BMX160_REG_ADDR_GYR_X_L     0x0c
#define BMX160_REG_ADDR_GYR_X_H     0x0d
#define BMX160_REG_ADDR_GYR_Y_L     0x0e
#define BMX160_REG_ADDR_GYR_Y_H     0x0f
#define BMX160_REG_ADDR_GYR_Z_L     0x10
#define BMX160_REG_ADDR_GYR_Z_H     0x11
#define BMX160_REG_ADDR_ACC_X_L     0x12
#define BMX160_REG_ADDR_ACC_X_H     0x13
#define BMX160_REG_ADDR_ACC_Y_L     0x14
#define BMX160_REG_ADDR_ACC_Y_H     0x15 
#define BMX160_REG_ADDR_ACC_Z_L     0x16
#define BMX160_REG_ADDR_ACC_Z_H     0x17

#define BMX160_REG_ADDR_SENSORTIME  0x18
#define BMX160_REG_ADDR_STATUS      0x1B
#define BMX160_REG_ADDR_INT_STATUS  0x1C
#define BMX160_REG_ADDR_TEMPERATURE 0x20
#define BMX160_REG_ADDR_FIFO_LENGTH 0x22
#define BMX160_REG_ADDR_FIFO_DATA   0x24
#define BMX160_REG_ADDR_ACC_CONF    0x40
#define BMX160_REG_ADDR_ACC_RANGE   0x41
#define BMX160_REG_ADDR_GYR_CONF    0x42
#define BMX160_REG_ADDR_GYR_RANGE   0x43
#define BMX160_REG_ADDR_MAG_CONF    0x44
#define BMX160_REG_ADDR_FIFO_DOWNS  0x45
#define BMX160_REG_ADDR_FIFO_CONFIG 0x46
#define BMX160_REG_ADDR_MAG_IF      0x4C
#define BMX160_REG_ADDR_INT_EN      0x50
#define BMX160_REG_ADDR_CMD         0x7E

//---- register values

#define BMX160_CMD_PMU_ACC_SUSPEND        0x10
#define BMX160_CMD_PMU_ACC_NORMAL         0x11
#define BMX160_CMD_PMU_ACC_LOW_POWER      0x12

#define BMX160_CMD_PMU_GYR_SUSPEND        0x14
#define BMX160_CMD_PMU_GYR_NORMAL         0x15
#define BMX160_CMD_PMU_GYR_FSU            0x17

#define BMX160_CMD_PMU_MAG_IF_SUSPEND     0x18
#define BMX160_CMD_PMU_MAG_IF_NORMAL      0x19
#define BMX160_CMD_PMU_MAG_IF_LOW_POWER   0x1A

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

// admin
uint8_t bmx160_who_am_i(void);
uint8_t bmx160_power_down(void);
uint8_t bmx160_get_pmu_status(void);

void    bmx160_set_cmd(uint8_t cmd);
void    bmx160_acc_config(uint8_t config);
void    bmx160_gyr_config(uint8_t config);
void    bmx160_mag_config(uint8_t config);

void    bmx160_acc_range(uint8_t range);
void    bmx160_gyr_range(uint8_t range);
void    bmx160_mag_if(uint8_t interface);

// read
int16_t bmx160_read_temperature(void);

int16_t bmx160_read_acc_x(void);
int16_t bmx160_read_acc_y(void);
int16_t bmx160_read_acc_z(void);

int16_t bmx160_read_mag_x(void);
int16_t bmx160_read_mag_y(void);
int16_t bmx160_read_mag_z(void);

int16_t bmx160_read_gyr_x(void);
int16_t bmx160_read_gyr_y(void);
int16_t bmx160_read_gyr_z(void);

void    bmx160_read_9dof_data(void);

//=========================== private =========================================