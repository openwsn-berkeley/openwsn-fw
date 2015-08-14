#include "opendefs.h"
#include "pid.h"
#include "neighbors.h"
#include "sixtop.h"
#include "scheduler.h"
#include "schedule.h"
#include "openqueue.h"

//=========================== define ==========================================

#define PID_ACCURACY                10000 //  each factor = K / PID_ACCURACY   
#define PID_PROPORTIONAL_GAIN_VALUE 10000 //  Kp = PID_PROPORTIONAL_GAIN_VALUE / PID_ACCURACY
#define PID_INTEGRAL_GAIN_VALUE         0 //  Ki = PID_INTEGRAL_GAIN_VALUE / PID_ACCURACY
#define PID_DERIVATIVE_GAIN_VALUE       0 //  Kd = PID_DERIVATIVE_GAIN_VALUE / PID_ACCURACY

#define PID_NUM_INTEGRAL_HISTORY    25

//=========================== variables =======================================

typedef struct {
   int16_t prevError;
   int16_t errorHistory[PID_NUM_INTEGRAL_HISTORY]; // It is already multiplied by dt
   open_addr_t address;    
   int16_t control;
} pid_vars_t;

pid_vars_t pid_vars;

//=========================== prototypes ======================================

int16_t pid_integralOperation();

//=========================== public ==========================================

// admin
void pid_init() {
    memset(&pid_vars,0,sizeof(pid_vars_t));
}

int16_t pid_compute_packetInQueue() {
    uint8_t i;
    int16_t returnVal;
    // what we desired is no packet in the queue buffer (0) 
    pid_vars.prevError = openqueue_getNumOfPakcetToParent()-TARGET_PACKET_IN_QUEUE;
    // udpate the history, elment zero is the newest one
    for (i=0;i<PID_NUM_INTEGRAL_HISTORY-1;i++) {
        pid_vars.errorHistory[PID_NUM_INTEGRAL_HISTORY-i-1] = pid_vars.errorHistory[PID_NUM_INTEGRAL_HISTORY-i-2];
    }
    pid_vars.errorHistory[0] = pid_vars.prevError * schedule_getFrameLength(); // multiple slotframe for integral calculation
    // calculate the pid
    returnVal  = pid_vars.prevError * PID_PROPORTIONAL_GAIN_VALUE / PID_ACCURACY;
    returnVal += pid_integralOperation() * PID_INTEGRAL_GAIN_VALUE / PID_ACCURACY;
    returnVal += ((pid_vars.errorHistory[0]-pid_vars.errorHistory[1])/schedule_getFrameLength()) * PID_DERIVATIVE_GAIN_VALUE / PID_ACCURACY;
    
    return returnVal;
}

int16_t pid_compute_usageOfCell() {
    uint8_t i;
    int16_t returnVal;
    // what we desired is no packet in the queue buffer (0) 
    pid_vars.prevError = (int16_t)schedule_getCellUsage()-(int16_t)TARGET_CELL_USAGE;
    
    // udpate the history, elment zero is the newest one
    for (i=0;i<PID_NUM_INTEGRAL_HISTORY-1;i++) {
        pid_vars.errorHistory[PID_NUM_INTEGRAL_HISTORY-i-1] = pid_vars.errorHistory[PID_NUM_INTEGRAL_HISTORY-i-2];
    }
    pid_vars.errorHistory[0] = pid_vars.prevError * schedule_getFrameLength(); // multiple slotframe for integral calculation
    // calculate the pid
    // Kp = PID_PROPORTIONAL_GAIN_VALUE/PID_ACCURACY
    returnVal  = pid_vars.prevError * PID_PROPORTIONAL_GAIN_VALUE / PID_ACCURACY; 
    // Ki = PID_INTEGRAL_GAIN_VALUE/PID_ACCURACY
    returnVal += pid_integralOperation() * PID_INTEGRAL_GAIN_VALUE / PID_ACCURACY; 
    // Kd = PID_DERIVATIVE_GAIN_VALUE/PID_ACCURACY
    returnVal += ((pid_vars.errorHistory[0]-pid_vars.errorHistory[1])/schedule_getFrameLength()) * PID_DERIVATIVE_GAIN_VALUE / PID_ACCURACY; 
    
    return returnVal;
}
//=========================== private =========================================

int16_t pid_integralOperation() {
    int16_t returnVal = 0;
    uint8_t i;
    for (i=0;i<PID_NUM_INTEGRAL_HISTORY;i++) {
        returnVal += pid_vars.errorHistory[i];
    }
    return returnVal;
}
