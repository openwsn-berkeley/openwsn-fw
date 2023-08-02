/**
\brief bmx160 driver.

\author Tengfei Chang <tengfei.chang@gmail.com>, Nov 2021.
*/

#include "i2c.h"
#include "bmx160.h"

//=========================== define ==========================================

typedef struct{

    int16_t mag_x;
    int16_t mag_y;
    int16_t mag_z;

    int16_t rhall;

    int16_t gyr_x;
    int16_t gyr_y;
    int16_t gyr_z;

    int16_t acc_x;
    int16_t acc_y;
    int16_t acc_z;

}bmx160x_data_t;

typedef struct {
    
    bmx160x_data_t bmx160x_data;

}bmx160x_var_t;

//=========================== variables =======================================

bmx160x_var_t bmx160x_var;

//=========================== prototypes ======================================

//=========================== public ==========================================

// admin
uint8_t bmx160_who_am_i(void) {

    uint8_t chipid;
    i2c_read_bytes(BMX160_REG_ADDR_CHIPID, &chipid, 1);
    return chipid;
}

void bmx160_config_wakeup(void) {
    
}

uint8_t bmx160_power_down(void) {

}

uint8_t bmx160_get_pmu_status(void) {

    uint8_t pmu_status;
    i2c_read_bytes(BMX160_REG_ADDR_PMU_STATUS, &pmu_status, 1);
    return pmu_status;
}


void bmx160_set_cmd(uint8_t cmd) {
    i2c_write_bytes(BMX160_REG_ADDR_CMD, &cmd, 1);
}

// configuration

void    bmx160_acc_config(uint8_t config) {
    i2c_write_bytes(BMX160_REG_ADDR_ACC_CONF, &config, 1);
}

void    bmx160_gyr_config(uint8_t config) {
    i2c_write_bytes(BMX160_REG_ADDR_GYR_CONF, &config, 1);
}

void    bmx160_mag_config(uint8_t config) {
    i2c_write_bytes(BMX160_REG_ADDR_MAG_CONF, &config, 1);
}

// range & interface

void    bmx160_acc_range(uint8_t range) {
    i2c_write_bytes(BMX160_REG_ADDR_ACC_RANGE, &range, 1);
}

void    bmx160_gyr_range(uint8_t range) {
    i2c_write_bytes(BMX160_REG_ADDR_GYR_RANGE, &range, 1);
}

void    bmx160_mag_if(uint8_t interface) {
    i2c_write_bytes(BMX160_REG_ADDR_MAG_IF, &interface, 1);
}


// read
void bmx160_read_9dof_data(void) {

    uint8_t pmu_status;

    // enable  accelarometer
    do {
        bmx160_set_cmd(BMX160_CMD_PMU_ACC_NORMAL);
        i2c_read_bytes(BMX160_REG_ADDR_PMU_STATUS, &pmu_status, 1);
    } while ((pmu_status & 0x10) != 0x10); 

    // enable gyroscope
    do {
        bmx160_set_cmd(BMX160_CMD_PMU_GYR_NORMAL);
        i2c_read_bytes(BMX160_REG_ADDR_PMU_STATUS, &pmu_status, 1);
    } while ((pmu_status & 0x04) != 0x04); 

    // enabel mag
    do {
        bmx160_set_cmd(BMX160_CMD_PMU_MAG_IF_NORMAL);
        i2c_read_bytes(BMX160_REG_ADDR_PMU_STATUS, &pmu_status, 1);
    } while ((pmu_status & 0x01) != 0x01); 

    // wait until pmu status are correct
    //       acc_normal gyr_normal mag_if_normal
    // 0xb10 01         01         01 = 0x95
    
    i2c_read_bytes(BMX160_REG_ADDR_DATA, (uint8_t*)(&bmx160x_var.bmx160x_data), sizeof(bmx160x_data_t));
}


int16_t bmx160_read_acc_x(void) {

    return bmx160x_var.bmx160x_data.acc_x;
}
int16_t bmx160_read_acc_y(void) {
    
    return bmx160x_var.bmx160x_data.acc_y;
}
int16_t bmx160_read_acc_z(void) {
    
    return bmx160x_var.bmx160x_data.acc_z;
}

int16_t bmx160_read_mag_x(void) {
    
    return bmx160x_var.bmx160x_data.mag_x;
}
int16_t bmx160_read_mag_y(void) {
    
    return bmx160x_var.bmx160x_data.mag_y;
}
int16_t bmx160_read_mag_z(void) {
    
    return bmx160x_var.bmx160x_data.mag_z;
}

int16_t bmx160_read_gyr_x(void) {
    
    return bmx160x_var.bmx160x_data.gyr_x;
}
int16_t bmx160_read_gyr_y(void) {
    
    return bmx160x_var.bmx160x_data.gyr_y;
}
int16_t bmx160_read_gyr_z(void) {
    
    return bmx160x_var.bmx160x_data.gyr_z;
}

//=========================== helper ==========================================

float bmx160_from_lsb_to_celsius(int16_t lsb) {

}

float bmx160_from_fs8_lp1_to_mg(int16_t lsb) {

}

//=========================== private =========================================