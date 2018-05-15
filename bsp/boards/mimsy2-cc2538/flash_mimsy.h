#ifndef __FLASH_MIMSY_H__
#define __FLASH_MIMSY_H__

#define IMU_DATA_STRUCT_SIZE 24
#include <stdint.h>

/*IMUData is a union data structure used to store imu data points. Access the struct
type of this union is used for accessing and setting the data. The uint32 array version
of the struct is used for reading and writing to flash*/
typedef union IMUData {
  
  struct {
  uint16_t accelX;//a X data
  uint16_t accelY;//a X data
  uint16_t accelZ;//a X data
  
  uint16_t gyroX;//gyro X data
  uint16_t gyroY;//gyro Y data
  uint16_t gyroZ;//gyro Z data
  
  uint32_t timestamp;
  float servo_state_0;
  float servo_state_1;
  
 

} fields;
  struct {
  int16_t accelX;//a X data
  int16_t accelY;//a X data
  int16_t accelZ;//a X data
  
  int16_t gyroX;//gyro X data
  int16_t gyroY;//gyro Y data
  int16_t gyroZ;//gyro Z data
  
  int32_t timestamp;
  float servo_state_0;
  float servo_state_1;
  
 

} signedfields;
uint32_t bits[6];
}IMUData;


/*This struct is used to keep track of wherer data was written to. This strucut 
must be passed to flashWriteIMU where it is updated to include the flash location 
of the data. A written data card is passed to flashReadIMU inorder to read the 
data from that location*/
typedef struct IMUDataCard{
    uint32_t page;
    uint32_t startTime;
    uint32_t endTime;
} IMUDataCard;


extern void flashWriteIMU(IMUData data[],uint32_t size, uint32_t startPage, int wordsWritten);
extern void flashReadIMU(IMUDataCard card, IMUData *data, uint32_t size);
extern void flashReadIMUSection(IMUDataCard card, IMUData *data, uint32_t size,int wordsRead);


#endif
