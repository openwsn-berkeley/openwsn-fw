/**
 * Author: Brian Kilberg (bkilberg@berkeley.edu)
            Xavier Vilajosana (xvilajosana@eecs.berkeley.edu)
 *         Pere Tuset (peretuset@openmote.com)
 * Date:   October 2017
 * Description:Mimsy-CC2538-specific definition of the "i2c" bsp module.
 */

#include <hw_gpio.h>
#include <hw_i2cm.h>
#include <hw_ioc.h>
#include <hw_memmap.h>
#include <hw_sys_ctrl.h>
#include <hw_types.h>
#include <gptimer.h>
#include <gpio.h>
#include <i2c_lib.h>
#include <ioc.h>
#include <sys_ctrl.h>

//=========================== define ==========================================

#define I2C_PERIPHERAL          ( SYS_CTRL_PERIPH_I2C )
#define I2C_BASE                ( GPIO_D_BASE )
#define I2C_SCL                 ( GPIO_PIN_4 )
#define I2C_SDA                 ( GPIO_PIN_5 )
#define I2C_BAUDRATE            ( 400000 )
#define I2C_MAX_DELAY_US        ( 1000000 )

//=========================== variables =======================================


//=========================== prototypes ======================================

 uint32_t board_timer_get(void);
 bool board_timer_expired(uint32_t future);
bool board_timer_expired(uint32_t future);
//=========================== public ==========================================
/**
 * Timer runs at 32 MHz and is 32-bit wide
 * The timer is divided by 32, whichs gives a 1 microsecond ticks
 */
/*
void board_timer_init(void) {
     SysCtrlPeripheralEnable(SYS_CTRL_PERIPH_GPT2);
  
  // Configure the timer
    TimerConfigure(GPTIMER2_BASE, GPTIMER_CFG_PERIODIC_UP);
    
    // Enable the timer
    TimerEnable(GPTIMER2_BASE, GPTIMER_BOTH);
}

/**
 * Returns the current value of the timer
 * The timer is divided by 32, whichs gives a 1 microsecond ticks
 */
/*
uint32_t board_timer_get(void) {
    uint32_t current;
    
    current = TimerValueGet(GPTIMER2_BASE, GPTIMER_A) >> 5;
    
    return current;
}
*/
/**
 * Returns true if the timer has expired
 * The timer is divided by 32, whichs gives a 1 microsecond ticks
 */
/*
bool board_timer_expired(uint32_t future) {
    uint32_t current;
    int32_t remaining;

    current = TimerValueGet(GPTIMER2_BASE, GPTIMER_A) >> 5;

    remaining = (int32_t) (future - current);
    
    if (remaining > 0) {
        return false;
    } else {
        return true;
    }
}
*/

extern void delay_ms(uint32_t delay){
  uint32_t current=board_timer_get();
  uint32_t timeout=(current*32+delay*1000*32)/32;
  if (timeout>current){ //executes of timeout time is bigger than current time
    while(board_timer_get()<timeout){
      
    }
  }else{      //executes differently if timeout is smaller to to overflow
    while(board_timer_get()>timeout){   //delay until overflow
      
    }
    while(board_timer_get()<timeout){   //now count normally
      
    }
  }
}
  void get_ms(uint32_t *timestamp){
    *timestamp = board_timer_get()*32/(SysCtrlClockGet()/1000);
  }




int i2c_read_registers(uint8_t slave_addr,
                             uint8_t reg_addr,
                             uint8_t numBytes,
                             uint8_t* spaceToWrite){
                               
           i2c_write_byte(slave_addr,reg_addr);
           //i2c_read_byte(slave_addr,spaceToWrite);
           i2c_read_bytes(slave_addr,spaceToWrite,numBytes);
           delay_ms(1);
           return 0;
}

void i2c_read_register(uint8_t slave_addr,
                             uint8_t reg_addr,
                             
                             uint8_t* spaceToWrite){
                               
           i2c_write_byte(slave_addr,reg_addr);
           i2c_read_byte(slave_addr,spaceToWrite);
          
}

void i2c_write_register_8bit( uint8_t slave_addr,uint8_t reg_addr, uint8_t data){
  uint8_t buffer[2]  ={reg_addr,data};          
        
              i2c_write_bytes(slave_addr,buffer,2);

}

int i2c_write_registers( uint8_t slave_addr,uint8_t reg_addr, uint8_t length,uint8_t *data){
  uint8_t buffer[100] ;
  buffer[0]=reg_addr;
  for(int i = 0; i < length+1; i++){
    buffer[i+1]=*data;
    data++; 
  }
        
              i2c_write_bytes(slave_addr,buffer,length+1);
  delay_ms(1);
  return 0;
}


//=========================== private =========================================

