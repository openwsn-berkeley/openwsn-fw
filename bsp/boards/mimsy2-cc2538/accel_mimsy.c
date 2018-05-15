#include "i2c_mimsy.h"
#include "i2c.h"
#include "headers/MPU9250_RegisterMap.h"
#include "flash_mimsy.h" //TODO: mive imu_data type to a new mimsy.h file
#include "gptimer.h"
#include "hw_gptimer.h"
#include "hw_memmap.h"

//invensense related includes

#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
//#include "invensense.h"
#include "invensense_adv.h"
#include "eMPL_outputs.h"
#include "mltypes.h"
#include "mpu.h"
#include "log.h"
//#include "ml_math_func.h"
#include <math.h>

//defines

#define DEFAULT_MPU_HZ  (20)
#define IMU_ADDRESS 0x69
union IMURaw {
  
  uint16_t words[7];
  uint8_t bytes[14];
  
  
};

//invensense related structures################################################

struct platform_data_s {
    const signed char orientation[9];
};

//orientation for upright rocket

static struct platform_data_s gyro_pdata = {
    .orientation = { 0, 0, -1,
                     -1, 0, 0,
                     0, 1, 0}
};


//orientation for flat rocket
/*static struct platform_data_s gyro_pdata = {
    .orientation = { 1, 0, 0,
                     0, -1, 0,
                     0, 0, -1}
};*/


struct rx_s {
    unsigned char header[3];
    unsigned char cmd;
};
struct hal_s {
    unsigned char lp_accel_mode;
    unsigned char sensors;
    unsigned char dmp_on;
    unsigned char wait_for_tap;
    volatile unsigned char new_gyro;
    unsigned char motion_int_mode;
    unsigned long no_dmp_hz;
    unsigned long next_pedo_ms;
    unsigned long next_temp_ms;
    unsigned long next_compass_ms;
    unsigned int report;
    unsigned short dmp_features;
    struct rx_s rx;
};
static struct hal_s hal = {0};
static long alt_inv_q29_mult(long a, long b);
static long alt_inv_q30_mult(long a, long b);
static void alt_inv_q_mult(const long *q1, const long *q2, long *qProd);
void alt_inv_q_invert(const long *q, long *qInverted);

//unsigned short alt_inv_orientation_matrix_to_scalar(const signed char *mtx);


//functions ####################################################################
//TODO: add an imu init function
//mimsy init function; inits board timers, resets imu, wakes imu, sets clock source
//enables sensors

//phony callback function for dmp
static void tap_cb(unsigned char direction, unsigned char count)
{

    return;
}

//phony callback function for dmp
static void android_orient_cb(unsigned char orientation)
{
	switch (orientation) {

	default:
		return;
	}
}




void mimsyIMUInit(void){
    //board_timer_init();
    uint8_t readbyte;
    
    
    //     i2c_init();
    uint8_t address;
    address=0x69;
    
     i2c_write_byte(address,MPU9250_PWR_MGMT_1); //reset
     i2c_write_byte(address,0x80);
    
      i2c_write_byte(address,MPU9250_PWR_MGMT_1); //wake
     i2c_write_byte(address,0x00);
    
    uint8_t bytes[2]={MPU9250_PWR_MGMT_1,0x01}  ; 
     i2c_write_bytes(address,bytes,2); //set gyro clock source
   

     bytes[0]=0x6C;
     bytes[1]=0x03;
        uint8_t *byteptr=&readbyte;
      
        i2c_write_byte(address,MPU9250_PWR_MGMT_2);
     i2c_read_byte(address,byteptr);
     
     i2c_write_byte(address,MPU9250_PWR_MGMT_2); //sens enable
     i2c_write_byte(address,0x00);
     
     i2c_write_byte(address,MPU9250_PWR_MGMT_2);
     i2c_read_byte(address,byteptr);
  
  
}

void mimsySetAccelFsr(int fsr){
   mpu_set_accel_fsr(fsr);
}

void mimsySetGyroFsr(int fsr){
   mpu_set_gyro_fsr(fsr);
}

//gyro reads based on invensense drivers
void mimsyIMURead6DofInv(IMUData *data){
  
}
//TODO: start adding invensense driver-based functions

//reads IMU data from Mimsy's MPU9250 
void mimsyIMURead6Dof( IMUData *data){
  uint8_t address=IMU_ADDRESS;
  uint8_t readbyte;  
  uint8_t *byteptr=&readbyte;

  //Accel X
      i2c_read_register(address,MPU9250_ACCEL_XOUT_H,byteptr);
     (*data).fields.accelX=((uint16_t) readbyte)<<8;
    
    i2c_read_register(address,MPU9250_ACCEL_XOUT_L,byteptr);
      (*data).fields.accelX=((uint16_t) readbyte)| (*data).fields.accelX;
      
        //Accel Y
      i2c_read_register(address,MPU9250_ACCEL_YOUT_H,byteptr);
     (*data).fields.accelY=((uint16_t) readbyte)<<8;
    
    i2c_read_register(address,MPU9250_ACCEL_YOUT_L,byteptr);
      (*data).fields.accelY=((uint16_t) readbyte)| (*data).fields.accelY;
      
        //Accel Z
      i2c_read_register(address,MPU9250_ACCEL_ZOUT_H,byteptr);
     (*data).fields.accelZ=((uint16_t) readbyte)<<8;
    
    i2c_read_register(address,MPU9250_ACCEL_ZOUT_L,byteptr);
      (*data).fields.accelZ=((uint16_t) readbyte)| (*data).fields.accelZ;
      
        //Gyro X
      i2c_read_register(address,MPU9250_GYRO_XOUT_H,byteptr);
     (*data).fields.gyroX=((uint16_t) readbyte)<<8;
    
    i2c_read_register(address,MPU9250_GYRO_XOUT_L,byteptr);
      (*data).fields.gyroX=((uint16_t) readbyte)| (*data).fields.gyroX;
      
        //Gyro Y
      i2c_read_register(address,MPU9250_GYRO_YOUT_H,byteptr);
    (*data).fields.gyroY=((uint16_t) readbyte)<<8;
    
    i2c_read_register(address,MPU9250_GYRO_YOUT_L,byteptr);
      (*data).fields.gyroY=((uint16_t) readbyte)| (*data).fields.gyroY;
      
        //Gyro Z
      i2c_read_register(address,MPU9250_GYRO_ZOUT_H,byteptr);
     (*data).fields.gyroZ=((uint16_t) readbyte)<<8;
    
    i2c_read_register(address,MPU9250_GYRO_ZOUT_L,byteptr);
      (*data).fields.gyroZ=((uint16_t) readbyte)| (*data).fields.gyroZ;
       
      
      (*data).fields.timestamp= TimerValueGet(GPTIMER2_BASE, GPTIMER_A);
}   

static unsigned short alt_inv_row_2_scale(const signed char *row)
{
    unsigned short b;

    if (row[0] > 0)
        b = 0;
    else if (row[0] < 0)
        b = 4;
    else if (row[1] > 0)
        b = 1;
    else if (row[1] < 0)
        b = 5;
    else if (row[2] > 0)
        b = 2;
    else if (row[2] < 0)
        b = 6;
    else
        b = 7;		// error
    return b;
}

/** Converts an orientation matrix made up of 0,+1,and -1 to a scalar representation.
* @param[in] mtx Orientation matrix to convert to a scalar.
* @return Description of orientation matrix. The lowest 2 bits (0 and 1) represent the column the one is on for the
* first row, with the bit number 2 being the sign. The next 2 bits (3 and 4) represent
* the column the one is on for the second row with bit number 5 being the sign.
* The next 2 bits (6 and 7) represent the column the one is on for the third row with
* bit number 8 being the sign. In binary the identity matrix would therefor be:
* 010_001_000 or 0x88 in hex.
*/
unsigned short alt_inv_orientation_matrix_to_scalar(const signed char *mtx)
{

    unsigned short scalar;

    /*
       XYZ  010_001_000 Identity Matrix
       XZY  001_010_000
       YXZ  010_000_001
       YZX  000_010_001
       ZXY  001_000_010
       ZYX  000_001_010
     */

    scalar = alt_inv_row_2_scale(mtx);
    scalar |= alt_inv_row_2_scale(mtx + 3) << 3;
    scalar |= alt_inv_row_2_scale(mtx + 6) << 6;


    return scalar;
}

void mimsyDmpBegin(){
      dmp_load_motion_driver_firmware();
    dmp_set_orientation(
       alt_inv_orientation_matrix_to_scalar(gyro_pdata.orientation));
          dmp_register_tap_cb(tap_cb);
    dmp_register_android_orient_cb(android_orient_cb);
    
        hal.dmp_features = DMP_FEATURE_6X_LP_QUAT
        		| DMP_FEATURE_SEND_RAW_ACCEL  | DMP_FEATURE_GYRO_CAL |DMP_FEATURE_SEND_CAL_GYRO;
    dmp_enable_feature(hal.dmp_features);
    dmp_set_fifo_rate(100);
    //dmp_enable_6x_lp_quat(1);
    mpu_set_dmp_state(1);
    hal.dmp_on = 1;
}

/** Performs a multiply and shift by 29. These are good functions to write in assembly on
 * with devices with small memory where you want to get rid of the long long which some
 * assemblers don't handle well
 * @param[in] a
 * @param[in] b
 * @return ((long long)a*b)>>29
*/
static long alt_inv_q29_mult(long a, long b)
{
#ifdef EMPL_NO_64BIT
    long result;
    result = (long)((float)a * b / (1L << 29));
    return result;
#else
    long long temp;
    long result;
    temp = (long long)a * b;
    result = (long)(temp >> 29);
    return result;
#endif
}

/** Performs a multiply and shift by 30. These are good functions to write in assembly on
 * with devices with small memory where you want to get rid of the long long which some
 * assemblers don't handle well
 * @param[in] a
 * @param[in] b
 * @return ((long long)a*b)>>30
*/
static long alt_inv_q30_mult(long a, long b)
{
#ifdef EMPL_NO_64BIT
    long result;
    result = (long)((float)a * b / (1L << 30));
    return result;
#else
    long long temp;
    long result;
    temp = (long long)a * b;
    result = (long)(temp >> 30);
    return result;
#endif
}

/** Performs a fixed point quaternion multiply.
* @param[in] q1 First Quaternion Multicand, length 4. 1.0 scaled
*            to 2^30
* @param[in] q2 Second Quaternion Multicand, length 4. 1.0 scaled
*            to 2^30
* @param[out] qProd Product after quaternion multiply. Length 4.
*             1.0 scaled to 2^30.
*/
static void alt_inv_q_mult(const long *q1, const long *q2, long *qProd)
{
    //INVENSENSE_FUNC_START;
    qProd[0] = alt_inv_q30_mult(q1[0], q2[0]) - alt_inv_q30_mult(q1[1], q2[1]) -
               alt_inv_q30_mult(q1[2], q2[2]) - alt_inv_q30_mult(q1[3], q2[3]);

    qProd[1] = alt_inv_q30_mult(q1[0], q2[1]) + alt_inv_q30_mult(q1[1], q2[0]) +
               alt_inv_q30_mult(q1[2], q2[3]) - alt_inv_q30_mult(q1[3], q2[2]);

    qProd[2] = alt_inv_q30_mult(q1[0], q2[2]) - alt_inv_q30_mult(q1[1], q2[3]) +
               alt_inv_q30_mult(q1[2], q2[0]) + alt_inv_q30_mult(q1[3], q2[1]);

    qProd[3] = alt_inv_q30_mult(q1[0], q2[3]) + alt_inv_q30_mult(q1[1], q2[2]) -
               alt_inv_q30_mult(q1[2], q2[1]) + alt_inv_q30_mult(q1[3], q2[0]);
}

/** Rotates a 3-element vector by Rotation defined by Q
*/
void alt_inv_q_rotate(const long *q, const long *in, long *out)
{
    long q_temp1[4], q_temp2[4];
    long in4[4], out4[4];

    // Fixme optimize
    in4[0] = 0;
    memcpy(&in4[1], in, 3 * sizeof(long));
    alt_inv_q_mult(q, in4, q_temp1);
    alt_inv_q_invert(q, q_temp2);
    alt_inv_q_mult(q_temp1, q_temp2, out4);
    memcpy(out, &out4[1], 3 * sizeof(long));
}

void alt_inv_q_invert(const long *q, long *qInverted)
{
    //INVENSENSE_FUNC_START;
    qInverted[0] = q[0];
    qInverted[1] = -q[1];
    qInverted[2] = -q[2];
    qInverted[3] = -q[3];
}

/**
 * Converts a quaternion to a rotation matrix.
 * @param[in] quat 4-element quaternion in fixed point. One is 2^30.
 * @param[out] rot Rotation matrix in fixed point. One is 2^30. The
 *             First 3 elements of the rotation matrix, represent
 *             the first row of the matrix. Rotation matrix multiplied
 *             by a 3 element column vector transform a vector from Body
 *             to World.
 */
void alt_inv_quaternion_to_rotation(const long *quat, long *rot)
{
    rot[0] =
        alt_inv_q29_mult(quat[1], quat[1]) + alt_inv_q29_mult(quat[0],
                quat[0]) -
        1073741824L;
    rot[1] =
        alt_inv_q29_mult(quat[1], quat[2]) - alt_inv_q29_mult(quat[3], quat[0]);
    rot[2] =
        alt_inv_q29_mult(quat[1], quat[3]) + alt_inv_q29_mult(quat[2], quat[0]);
    rot[3] =
        alt_inv_q29_mult(quat[1], quat[2]) + alt_inv_q29_mult(quat[3], quat[0]);
    rot[4] =
        alt_inv_q29_mult(quat[2], quat[2]) + alt_inv_q29_mult(quat[0],
                quat[0]) -
        1073741824L;
    rot[5] =
        alt_inv_q29_mult(quat[2], quat[3]) - alt_inv_q29_mult(quat[1], quat[0]);
    rot[6] =
        alt_inv_q29_mult(quat[1], quat[3]) - alt_inv_q29_mult(quat[2], quat[0]);
    rot[7] =
        alt_inv_q29_mult(quat[2], quat[3]) + alt_inv_q29_mult(quat[1], quat[0]);
    rot[8] =
        alt_inv_q29_mult(quat[3], quat[3]) + alt_inv_q29_mult(quat[0],
                quat[0]) -
        1073741824L;
}

void alt_mlMatrixVectorMult(long matrix[9], const long vecIn[3], long *vecOut)  {
        // matrix format
        //  [ 0  3  6;
        //    1  4  7;
        //    2  5  8]

        // vector format:  [0  1  2]^T;
        int i, j;
        long temp;

        for (i=0; i<3; i++)	{
                temp = 0;
                for (j=0; j<3; j++)  {
                        temp += alt_inv_q30_mult(matrix[i+j*3], vecIn[j]);
                }
                vecOut[i] = temp;
        }
}

void alt_inv_q_normalizef(float *q)
{
    //INVENSENSE_FUNC_START;
    float normSF = 0;
    float xHalf = 0;
    normSF = (q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);
    if (normSF < 2) {
        xHalf = 0.5f * normSF;
        normSF = normSF * (1.5f - xHalf * normSF * normSF);
        normSF = normSF * (1.5f - xHalf * normSF * normSF);
        normSF = normSF * (1.5f - xHalf * normSF * normSF);
        normSF = normSF * (1.5f - xHalf * normSF * normSF);
        q[0] *= normSF;
        q[1] *= normSF;
        q[2] *= normSF;
        q[3] *= normSF;
    } else {
        q[0] = 1.0;
        q[1] = 0.0;
        q[2] = 0.0;
        q[3] = 0.0;
    }
    normSF = (q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);
}



