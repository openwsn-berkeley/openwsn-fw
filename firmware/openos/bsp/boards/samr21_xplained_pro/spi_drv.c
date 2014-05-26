

#include "compiler.h"
#include "spi.h" //From OpenWSN
#include "spi_drv.h" //From Atmel 
#include "samr21_xplained_pro.h"
#include "leds.h"


typedef struct {
	// information about the current transaction
	uint8_t*        pNextTxByte;
	uint8_t         numTxedBytes;
	uint8_t         txBytesLeft;
	spi_return_t    returnType;
	uint8_t*        pNextRxByte;
	uint8_t         maxRxBytes;
	spi_first_t     isFirst;
	spi_last_t      isLast;
	// state of the module
	uint8_t         busy;
#ifdef SPI_IN_INTERRUPT_MODE
	// callback when module done
	spi_cbt         callback;
#endif
} spi_vars_t;

spi_vars_t spi_vars;

struct spi_slave_inst_config slave_dev_config;
struct spi_config config;
struct spi_module master;
struct spi_slave_inst slave;

void spi_init(void)
{
	 // clear variables
	 memset(&spi_vars,0,sizeof(spi_vars_t));
	 
	spi_slave_inst_get_config_defaults(&slave_dev_config);
	slave_dev_config.ss_pin = AT86RFX_SPI_CS;
	spi_attach_slave(&slave, &slave_dev_config);
	spi_get_config_defaults(&config);
	AT86RFX_SPI_CONFIG(config);
	spi_init_drv(&master, AT86RFX_SPI, &config);
	spi_enable(&master);
}

uint8_t trx_reg_read(uint8_t addr)
{
	uint16_t register_value = 0;
	uint16_t dummy_read;

	/* Prepare the command byte */
	addr |= 0x80;

	/* Start SPI transaction by pulling SEL low */
	spi_select_slave(&master, &slave, true);

	/* Send the Read command byte */
	while(!spi_is_ready_to_write(&master));
	spi_write(&master,addr);
	while(!spi_is_write_complete(&master));
	/* Dummy read since SPI RX is double buffered */
	while(!spi_is_ready_to_read(&master));
	spi_read(&master, &dummy_read);

	while(!spi_is_ready_to_write(&master));
	spi_write(&master,0);
	while(!spi_is_write_complete(&master));
	while(!spi_is_ready_to_read(&master));
	spi_read(&master, &register_value);

	/* Stop the SPI transaction by setting SEL high */
	spi_select_slave(&master, &slave, false);

	return register_value;
}


#ifdef SPI_IN_INTERRUPT_MODE
void spi_setCallback(spi_cbt cb)
{
 spi_vars.callback = cb;
}
#endif

void spi_txrx(uint8_t*  bufTx,
			 uint8_t      lenbufTx,
			 spi_return_t returnType,
			 uint8_t*     bufRx,
			 uint8_t      maxLenBufRx,
			 spi_first_t  isFirst,
             spi_last_t   isLast)
{
   uint16_t register_value = 0;
#ifdef SPI_IN_INTERRUPT_MODE	
   //Disable the interrupts
   cpu_irq_disable();
#endif
   
   // register spi frame to send
   spi_vars.pNextTxByte      =  bufTx;
   spi_vars.numTxedBytes     =  0;
   spi_vars.txBytesLeft      =  lenbufTx;
   spi_vars.returnType       =  returnType;
   spi_vars.pNextRxByte      =  bufRx;
   spi_vars.maxRxBytes       =  maxLenBufRx;
   spi_vars.isFirst          =  isFirst;
   spi_vars.isLast           =  isLast;
   
   // SPI is now busy
   spi_vars.busy             =  1;
   
   // lower CS signal to have slave listening
   if (spi_vars.isFirst==SPI_FIRST) {
	  /* Start SPI transaction by pulling SEL low */
	  spi_select_slave(&master, &slave, true);

   }
   
#ifdef SPI_IN_INTERRUPT_MODE
   // implementation 1. use a callback function when transaction finishes
   
   /* Send the Read command byte */	
   
   spi_write_buffer_wait(&master, spi_vars.pNextTxByte, 1);
   
   // re-enable interrupts
   cpu_irq_enable();
#else
   // implementation 2. busy wait for each byte to be sent
   // send all bytes
   while (spi_vars.txBytesLeft > 0) 
   {
	   // write next byte to TX buffer
	   /* Send the Read command byte */
		while(!spi_is_ready_to_write(&master));
		spi_write(&master, *spi_vars.pNextTxByte);
		while(!spi_is_write_complete(&master));

	   // save the byte just received in the RX buffer
	   switch (spi_vars.returnType) {
		   case SPI_FIRSTBYTE:
		   if (spi_vars.numTxedBytes==0) {
				while(!spi_is_ready_to_read(&master));
				spi_read(&master, &register_value);
				*spi_vars.pNextRxByte = (uint8_t)register_value;			   
		   }
		   break;
		   case SPI_BUFFER:
		   while(!spi_is_ready_to_read(&master));
		   spi_read(&master, &register_value);
		   *spi_vars.pNextRxByte = (uint8_t)register_value;
		   spi_vars.pNextRxByte++;
		   break;
		   case SPI_LASTBYTE:
		   while(!spi_is_ready_to_read(&master));
		   spi_read(&master, &register_value);
		   *spi_vars.pNextRxByte = (uint8_t)register_value;
		   break;
	   }
	   // one byte less to go
	   spi_vars.pNextTxByte++;
	   spi_vars.numTxedBytes++;
	   spi_vars.txBytesLeft--;
   }
   
   // put CS signal high to signal end of transmission to slave
   if (spi_vars.isLast==SPI_LAST) 
   {
	   /* Stop the SPI transaction by setting SEL high */
	   spi_select_slave(&master, &slave, false);
   }
   
   // SPI is not busy anymore
   spi_vars.busy             =  0;
#endif			 
}

kick_scheduler_t spi_isr(void)
{
#ifdef SPI_IN_INTERRUPT_MODE
// save the byte just received in the RX buffer
switch (spi_vars.returnType) 
{
	case SPI_FIRSTBYTE:
	if (spi_vars.numTxedBytes==0) 
	{
		spi_read_buffer_wait(&master, spi_vars.pNextRxByte, 1, 0);
	}
	break;
	case SPI_BUFFER:
	spi_read_buffer_wait(&master, spi_vars.pNextRxByte, 1, 0);
	spi_vars.pNextRxByte++;
	break;
	case SPI_LASTBYTE:
	spi_read_buffer_wait(&master, spi_vars.pNextRxByte, 1, 0);
	break;
}

// one byte less to go
spi_vars.pNextTxByte++;
spi_vars.numTxedBytes++;
spi_vars.txBytesLeft--;

if (spi_vars.txBytesLeft>0) {
	// write next byte to TX buffer
	spi_write_buffer_wait(&master, spi_vars.pNextTxByte, 1);
	} else {
	// put CS signal high to signal end of transmission to slave
	if (spi_vars.isLast==SPI_LAST) {
		/* Stop the SPI transaction by setting SEL high */
		spi_select_slave(&master, &slave, false);
	}
	// SPI is not busy anymore
	spi_vars.busy             =  0;
	
	// SPI is done!
	if (spi_vars.callback!=NULL) {
		// call the callback
		spi_vars.callback();
		// kick the OS
		return KICK_SCHEDULER;
	}
}
return DO_NOT_KICK_SCHEDULER;
#else
// this should never happpen!
while(1);
// we can not print from within the BSP. Instead:
// blink the error LED
leds_error_blink();
// reset the board
board_reset();

return DO_NOT_KICK_SCHEDULER; // we will not get here, statement to please compiler
#endif
}

