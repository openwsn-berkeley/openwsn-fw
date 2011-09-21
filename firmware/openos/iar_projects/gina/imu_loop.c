//!!!!!!!!!!THIS CODE IS STILL UNDER DVELOPMENT. DO NOT USE.

#include "drivers/gina.h"
#include "commands/commands.h"

/* Allocate Space for RX, TX */
unsigned int tx_int[(AT_MAX_PACKET_LEN + 4)>>1];
int      rx_int[(AT_MAX_PACKET_LEN + 4)>>1];
char*    tx;
char*    rx;
int      rx_chan;
int      tx_chan;
static int *cnt;
unsigned char *i2c_data;

#define XL_ADDR 0x18
#define MG_ADDR 0x1E
#define GY_ADDR 0x68
#define SCALE   0x28
#define CHANNEL 20

//Gyroscope registers
unsigned char readg[1]  = {0x1d};
unsigned char g1[2]     = {0x15, 0x01};
unsigned char g2[2]     = {0x16, 0x1E};
unsigned char g3[2]     = {0x17, 0x00};
unsigned char g4[2]     = {0x3e, 0x00};

//Mag registers
unsigned char magc[2]   = {0x02, 0x00};

//XL Registers
unsigned char regc[2]   = {0x0C, 0xA0};
unsigned char regb[2]   = {0x0D, 0xC0};
unsigned char read[1]   = {0x00};

void i2c_cfg() {
   //Enable XL
   P5OUT = 0x10;
   P5DIR = 0x10;

   //Config XL
   ti_i2c_send(XL_ADDR,SCALE,regc,2);
   ti_i2c_send(XL_ADDR,SCALE,regb,2);

   //Config Mag
   ti_i2c_send(MG_ADDR,SCALE,magc,2);

   //Config Gyro
   ti_i2c_send(GY_ADDR,SCALE,g1,2);
   ti_i2c_send(GY_ADDR,SCALE,g2,2);
   ti_i2c_send(GY_ADDR,SCALE,g3,2);
   ti_i2c_send(GY_ADDR,SCALE,g4,2);
}

void imu_measure() {
   recv_3axis(XL_ADDR,SCALE,i2c_data  ,read);
   recv_3axis(GY_ADDR,SCALE,i2c_data+6,readg);
   //recv_3axis(MG_ADDR,SCALE,i2c_data+12,read,350);
   //recv_mag(i2c_data+12);
   ADC_START;
   (*cnt)++;
}

void imu_send() {
   at_txmode(tx_chan);
   ADC_WAIT cmd_wait();

   while(at_send(tx, rx, (int)tx[1])) cmd_wait();
   AT_WAIT cmd_wait();
   AT_TX_START;
}  

/* Setup packet for IMU loop */
void imu_cfg() {
   tx = (char*)tx_int;
   rx = (char*)rx_int;

   // setup header
   tx[0] = AT_FRAME_TX;
   tx[1] = 32; // packet length 
   tx[2] = 0x00;
   tx[3] = 0x80;

   // setup counter
   cnt  = (int*)&tx[4];
   *cnt = -1;

   // bind xl vars
   adc_ax   = cnt+1;
   adc_ay   = cnt+2;
   adc_az1  = cnt+3;
   adc_az3  = cnt+4;
   adc_ti   = cnt+5;

   // bind xl+mag+gyro vars
   i2c_data = (unsigned char*)(cnt+6);
}

void radio_cfg() {
   IRQ_ENABLE;
   at_config();
   tx_chan = CHANNEL;
   rx_chan = CHANNEL;
   at_rxmode(rx_chan);

   //here is the antenna diversity stuff
   char reg_old_div;
   char reg_old_ctrl;
   reg_old_div = at_get_reg(RG_ANT_DIV);
   reg_old_ctrl = at_get_reg(RG_RX_CTRL);

   //without diversity:
   //reg_old_div |= 0x05; //use antenna 1 On-chip 60,56
   reg_old_div |= 0x06; //use antenna 0 Coax 57,57

   reg_old_ctrl |= 0x07; //no matter which antenna is used

   //with diversity:
   //reg_old_div |= 0x0d; //use antenna diversity
   //reg_old_ctrl |= 0x03; //recommended value when using antenna diversity*/

   at_set_reg(RG_ANT_DIV, reg_old_div);
   at_set_reg(RG_RX_CTRL, reg_old_ctrl);

   cmd_mode         = CMD_MODE_SPIN;
   cmd_mode_changed = 0;
   cmd_ledmode       = CMD_LEDMODE_CNT;
   return;
}

int main(void) {
   char blah;
   
   //initialization
   gina_init();  // initialize hardware
   at_init();    // enable radio
   pwm_init();   // enable timers
   adc_init();   // init Analog-to-Digital (temp+xl)
   i2c_cfg();

   radio_cfg(); // init Radio
   imu_cfg();

   if(at_test()) {
      cmd_mode = CMD_MODE_ERROR;
   }

   cmd_mode = CMD_MODE_IMU_LOOP; //have the mote just blast data
   
   while(1) {
      //radio_cfg();
      //cmd_mode = CMD_MODE_IMU_LOOP; 
      cmd_loop();
      //Collect and send data 
      if (cmd_mode == CMD_MODE_IMU_LOOP) {
         imu_measure();
         imu_send();
         //blah=at_get_reg(RG_ANT_DIV);
         //blah+1;
      }

      //Clear out RX interrupts if we aren't expecting something
      if ((at_state!=AT_STATE_RX_READY && at_state!=AT_STATE_RX_WAITING)) { 
         at_rxmode(0);
         AT_CLR_IRQ;
      }
      PWM_WAIT;  //wait for the 3ms timer
         //Check RX
      if (at_state == AT_STATE_RX_WAITING) {
         at_read(&bytes, &len);
      }
   }
   return 0;
}

#pragma vector=IRQ_VECTOR
__interrupt void gina_ISR (void) {
   if (IRQ_GET(IRQ_AT_RX_DONE)) {	         // Radio pkt rec'd
      char *buf = bytes;
      char cmd = cmd_parsepacket(&buf, *len, &at_state, AT_STATE_RX_READY);
      IRQ_CLR(IRQ_AT_RX_DONE);
   } else if (IRQ_GET(IRQ_AT_RX_WAIT)) {	 // Radio pkt waiting
      IRQ_CLR(IRQ_AT_RX_WAIT);
   } else {
      IRQ_CLR(0xff);
   }
}
