/**
\brief This is a standalone test program for 8 outputting Pulse Width Modulated (PWM)
       digital signals through the expansion ports of the GINA2.2b/c boards.

The digital output pins are: TBC

\author Ankur Mehta <mehtank@eecs.berkeley.edu>, August 2010
*/

#include "drivers/gina.h"
#include "commands/commands.h"

/* Allocate Space for RX, TX */
int tx_int[(AT_MAX_PACKET_LEN + 4)>>2];
int rx_int[(AT_MAX_PACKET_LEN + 4)>>2];
char *tx;
char *rx;
int rx_chan;
int tx_chan;
static int *cnt;
char *i2c_data;

void imu_measure() {
    //i2c_read(i2c_data);
	ADC_START;
    (*cnt)++;
}

void imu_send() {
    at_txmode(tx_chan);
    ADC_WAIT cmd_wait();
    //XL_WAIT cmd_wait();
    
	while(at_send(tx, rx, (int)tx[1])) cmd_wait();
    AT_WAIT cmd_wait();
    AT_TX_START;
}  

/* Setup packet for IMU loop */
void imu_cfg()
{
    tx = (char*)tx_int;
    rx = (char*)rx_int;

    // setup header
    tx[0] = AT_FRAME_TX;
    tx[1] = 24;     // packet length 
    tx[2] = 0x00;
    tx[3] = 0x80;
    
    // setup counter
    cnt = (int*)&tx[4];
    *cnt = -1;
    
    // bind xl vars
    adc_ax = cnt+1;
    adc_ay = cnt+2;
    adc_az1 = cnt+3;
    adc_az3 = cnt+4;
    adc_ti = cnt+5;

    // bind xl+mag+gyro vars
    i2c_data = (char*)cnt+6;
}

void radio_cfg()
{
  IRQ_ENABLE;
  at_config();
  tx_chan = 20;
  rx_chan = 20;
  at_rxmode(rx_chan);
  //here is the antenna diversity stuff
  char reg_old_div;
  char reg_old_ctrl;
  reg_old_div = at_get_reg(RG_ANT_DIV);
  reg_old_ctrl = at_get_reg(RG_RX_CTRL);

  //without diversity:
  reg_old_div |= 0x05; //use antenna 1
  //reg_old_div |= 0x06; //use antenna 0

  reg_old_ctrl |= 0x07; //no matter which antenna is used

  /* //with diversity:
  reg_old_div |= 0x0d; //use antenna diversity
  reg_old_ctrl |= 0x03; //recommended value when using antenna diversity*/


  at_set_reg(RG_ANT_DIV, reg_old_div);
  at_set_reg(RG_RX_CTRL, reg_old_ctrl);
  
  
  cmd_mode = CMD_MODE_SPIN;
  cmd_mode_changed = 0;
  cmd_ledmode = CMD_LEDMODE_CNT;
  return;
}

int main(void)
{
// power up the mote and use an oscilloscope to probe the pins 2,4,6,8 and 10 on connector J1
// and pins 5 and 7 on connector J2  
  // you should see a pwm being generated with a period of 500 microsecond, and a duty cycle as given below:
  // J1 - pin 2: 50%
  // J1 - pin 4: 90%
  // J1 - pin 6: 80%
  // J1 - pin 8: 70%
  // J1 - pin 10: 60%
  // J2 - pin 5: 30%
  // J2 - pin 7: 40%
  // J2 - pin 9: not sure if this will work

    gina_init();  // initialize hardware
    //at_init();    // enable Radio
    //adc_init();   // init Analog-to-Digital (temp+xl)
	//i2c_init();
    pwm_config();
    pwm_init();	
    //radio_cfg(); // init Radio
    //i2c_config();
    //imu_cfg();
    //if(at_test()) //|| xl_test())
      //  cmd_mode = CMD_MODE_ERROR;
    while(1) {
//        cmd_loop();
//issue PWM signals here
      pwm_setservo(1,900);
      pwm_setservo(2,800);
      pwm_setservo(3,700);
      pwm_setservo(4,600);
      pwm_setservo(5,500);
      pwm_setmotor(1,400);
      pwm_setmotor(2,300);
    //  pwm_setservo(6,200);
      while(1){};
        continue;
        //Collect and send data 
        if (cmd_mode == CMD_MODE_IMU_LOOP) {
            imu_measure();
            imu_send();
        }
        
        //Clear out RX interrupts if we aren't expecting something
        if ((at_state != AT_STATE_RX_READY &&
                    at_state != AT_STATE_RX_WAITING)) { 
            at_rxmode(0);
            AT_CLR_IRQ;
        }
        
        //Check RX
        if (at_state == AT_STATE_RX_WAITING)
                at_read(&bytes, &len);
    }
    return 0;
}

#pragma vector=IRQ_VECTOR
__interrupt void gina_ISR (void)
{
   /* if (IRQ_GET(IRQ_AT_RX_DONE)) {	// Radio pkt rec'd
                char *buf = bytes;
                char cmd = cmd_parsepacket(&buf, *len, &at_state, AT_STATE_RX_READY);
		IRQ_CLR(IRQ_AT_RX_DONE);
	} else if (IRQ_GET(IRQ_AT_RX_WAIT)) {	// Radio pkt waiting
		IRQ_CLR(IRQ_AT_RX_WAIT);
	} else {
		IRQ_CLR(0xff);
    }*/
}
