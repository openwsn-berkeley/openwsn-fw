/**
\brief K20-specific definition of the "spi" bsp module.

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, May 2012.
 */

#include "spi.h"
#include "board.h"
#include "common.h"



//=========================== defines =========================================
#define SPI_SPEED_KHZ 6000 //6Mhz
#define SPI_INTERDATA_DELAY 5 //
#define SPI_SETUP_DELAY 5 //

#define SPI_DEFAULT_CPOL 0
#define SPI_DEFAULT_CPHA 0

#define SPI_CS_PIN   0
#define SPI_CS_PIN_MASK (1<<SPI_CS_PIN)
//=========================== variables =======================================


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

extern periph_clk_khz;

//=========================== prototypes ======================================
static bool  private_spi_findBaudRate(uint32_t clk_div, uint8_t *p_BR, uint8_t *p_PBR, uint8_t *p_DBR);
static bool  private_spi_calcDelayParams(uint32_t  pclk_freq, uint32_t  delay, uint8_t *p_scaler, uint8_t *p_prescaler);
//=========================== public ==========================================

void spi_init() {
	// clear variables

	uint32_t    clk_div;
	uint8_t     br;
	uint8_t     pbr;
	uint8_t     dbr;
	uint8_t     asc;
	uint8_t     pasc;
	uint8_t     cssck;
	uint8_t     pcssck;

	memset(&spi_vars,0,sizeof(spi_vars_t));

	//this coded is intended for dynamic SPI bus speed configuration. Needs testing.
	//	if(periph_clk_khz < SPI_SPEED_KHZ){                            
	//		return;//error
	//	}
	//	clk_div  = periph_clk_khz / SPI_SPEED_KHZ;     
	//
	//	if(periph_clk_khz%SPI_SPEED_KHZ != 0){                                /* See Note 2.d                                     */
	//		clk_div = clk_div + 1;
	//	}
	//
	//	if(private_spi_findBaudRate( periph_clk_khz,  &br,  &pbr,  &dbr) == FALSE){
	//		return;
	//	}
	//
	//	if (private_spi_calcDelayParams(periph_clk_khz, SPI_INTERDATA_DELAY, &asc, &pasc) == FALSE){
	//		return;
	//	}
	//
	//	if (private_spi_calcDelayParams(periph_clk_khz, SPI_SETUP_DELAY, &cssck, &pcssck) == FALSE){
	//		return;
	//	}

	//init clock
	SIM_SCGC6 |= (SIM_SCGC6_SPI0_MASK);//power SPI0

	// configure SPI-related pins 
	PORTD_PCR0 = PORT_PCR_MUX(1);//CS0 -- PTD0 -- as gpio
	PORTD_PCR1 = PORT_PCR_MUX(2);//CLK -- PTD1
	PORTD_PCR2 = PORT_PCR_MUX(2);//MOSI -- PTD2
	PORTD_PCR3 = PORT_PCR_MUX(2);//MISO-- PTD3

	GPIOD_PDDR |= SPI_CS_PIN_MASK;//CS0 -- as output gpio

	GPIOD_PSOR |= SPI_CS_PIN_MASK;//set cs

	SPI0_MCR   = SPI_MCR_MSTR_MASK | SPI_MCR_DIS_RXF_MASK |  /* Configure SPI as master. Disable rx/tx fifos. Set */
			SPI_MCR_DIS_TXF_MASK | SPI_MCR_ROOE_MASK |  /* overwrite incoming data. Set state to STOPPED.    */ 
			SPI_MCR_HALT_MASK |	SPI_MCR_PCSIS(1);                         /* Chipselects inactive high        using PCS0         */

	/* SPI0_TCR: SPI_TCNT=0 */
	SPI0_TCR = (uint32_t)0x00UL;     
	/* SPI0_RSER: TCF_RE=0,EOQF_RE=0,TFUF_RE=0,TFFF_RE=0,TFFF_DIRS=0,RFOF_RE=0,RFDF_RE=0,RFDF_DIRS=0 */
	SPI0_RSER  = (uint32_t)0x00UL;                               /* Disable interrupts and DMA.                       */

	//this coded is intended for dynamic SPI bus speed configuration. Needs testing.
	/* Configure exclusive port CTAR                     */
	//	SPI0_CTAR0 |= SPI_CTAR_FMSZ(8-1)     | /*                                      */
	//			SPI_CTAR_DBR_MASK   | /* Configure clock                                   */
	//			SPI_CTAR_BR(br)        |
	//			SPI_CTAR_PBR(pbr)      |
	//			SPI_CTAR_PASC(pasc)    | /* Inter-data delay                                  */
	//			SPI_CTAR_ASC(asc)      |
	//			SPI_CTAR_PCSSCK(pcssck)| /* Setup delay                                       */
	//			SPI_CTAR_CSSCK(cssck)  |
	//			/* Configure CPOL and CPHA with parameter values     */
	//			((SPI_DEFAULT_CPOL << SPI_CTAR_CPOL_SHIFT) && SPI_CTAR_CPOL_MASK) |
	//			((SPI_DEFAULT_CPHA << SPI_CTAR_CPHA_SHIFT) && SPI_CTAR_CPHA_MASK);


	//acceptable values for spi bus when talking with the radio are: (in case dynamic configuration is used please remove that.)
	// assumming core cpu at 72Mhz and peripheral clk at 36Mhz
	// 1.8Mhz --> pbr=7--> SPI_CTAR_PBR(3) and 	br=16 -->SPI_CTAR_BR(4)
	// 6 Mhz  --> pbr=5--> SPI_CTAR_PBR(2) and  br=8 -->SPI_CTAR_BR(3)
	SPI0_CTAR0 = (SPI_CTAR_DBR_MASK | SPI_CTAR_FMSZ(8-1) | SPI_CTAR_PDT(0) | SPI_CTAR_BR(4)|SPI_CTAR_PBR(3) ); 


	SPI0_SR       = SPI_SR_EOQF_MASK|SPI_SR_TCF_MASK|SPI_SR_TFUF_MASK|SPI_SR_TFFF_MASK|SPI_SR_RFOF_MASK|SPI_SR_RFDF_MASK;             
	//SPI0_SR       = SPI_SR_EOQF_MASK;
	/* Set RUNNING state.                                */
	SPI0_MCR    &= ~SPI_MCR_HALT_MASK; /* SPI0_MCR: HALT=0 */


	// enable interrupts via the IEx SFRs
#ifdef SPI_IN_INTERRUPT_MODE
	//TODO                      // we only enable the SPI RX interrupt
	// since TX and RX happen concurrently,
	// i.e. an RX completion necessarily
	// implies a TX completion.
#endif
}

#ifdef SPI_IN_INTERRUPT_MODE
void spi_setCallback(spi_cbt cb) {
	spi_vars.callback = cb;
}
#endif

void spi_txrx(uint8_t*     bufTx,
		uint8_t      lenbufTx,
		spi_return_t returnType,
		uint8_t*     bufRx,
		uint8_t      maxLenBufRx,
		spi_first_t  isFirst,
		spi_last_t   isLast) {

	uint32_t  cont_trans_mask;


#ifdef SPI_IN_INTERRUPT_MODE
	// disable interrupts
	__disable_interrupt();
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
		GPIOD_PCOR |=SPI_CS_PIN_MASK;//clear cs
		//cont_trans_mask=SPI_PUSHR_CONT_MASK;
	}

#ifdef SPI_IN_INTERRUPT_MODE
	// implementation 1. use a callback function when transaction finishes

	// write first byte to TX buffer
	SPI0_PUSHR                 = *spi_vars.pNextTxByte;

	// re-enable interrupts
	__enable_interrupt();
#else
	// implementation 2. busy wait for each byte to be sent
	/* Decide whether to return PCSn signals to inactive.   */
	//cont_trans_mask = TRUE == spi_vars.isLast ? 0x00000000u : SPI_PUSHR_CONT_MASK;
	// send all bytes
	while (spi_vars.txBytesLeft>0) {
		// write next byte to TX buffer
		while (! (SPI0_SR & SPI_SR_TFFF_MASK)) { };           /* wait write buffer not full flag                      */
		SPI0_SR    = SPI_SR_RFOF_MASK | SPI_SR_TCF_MASK       /* Clear all flags.                                     */
				| SPI_SR_TFFF_MASK | SPI_SR_RFDF_MASK;

//		if (spi_vars.txBytesLeft==1 && spi_vars.isLast==SPI_LAST){
//			cont_trans_mask=0x00000000u;
//		}
		SPI0_PUSHR = SPI_PUSHR_CTAS(0)   /* Transmit data.      use ctar0                                 */
					 | SPI_PUSHR_CONT_MASK //1 Keep PCSn signals asserted between transfers.
		             | SPI_PUSHR_PCS(0)  //negate the pcs signal.
		             | *spi_vars.pNextTxByte;        

		while (! (SPI0_SR & SPI_SR_TCF_MASK)) {}
		while (! (SPI0_SR & SPI_SR_RFDF_MASK)) {}             /* wait read buffer not empty flag                      */


		// save the byte just received in the RX buffer
		switch (spi_vars.returnType) {
		case SPI_FIRSTBYTE:
			if (spi_vars.numTxedBytes==0) {
				*spi_vars.pNextRxByte  = SPI0_POPR;                               /* Rd byte.                                             */
			}
			break;
		case SPI_BUFFER:
			*spi_vars.pNextRxByte  = SPI0_POPR;                               /* Rd byte.                                             */
			spi_vars.pNextRxByte++;
			break;
		case SPI_LASTBYTE:
			*spi_vars.pNextRxByte  = SPI0_POPR;                               /* Rd byte.                                             */
			break;
		}
		// one byte less to go
		spi_vars.pNextTxByte++;
		spi_vars.numTxedBytes++;
		spi_vars.txBytesLeft--;
	}

	// put CS signal high to signal end of transmission to slave
	if (spi_vars.isLast==SPI_LAST) {
		GPIOD_PSOR |=SPI_CS_PIN_MASK;//set cs
		//cont_trans_mask=0x00000000u;
	}

	// SPI is not busy anymore
	spi_vars.busy             =  0;
#endif
}

//=========================== private =========================================

//=========================== interrupt handlers ==============================
//wrapper for isr as needs to be void.
void _spi_isr(){
	spi_isr();
}

//todo make it inline later.
kick_scheduler_t spi_isr() {
#ifdef SPI_IN_INTERRUPT_MODE
	// save the byte just received in the RX buffer
	switch (spi_vars.returnType) {
	case SPI_FIRSTBYTE:
		if (spi_vars.numTxedBytes==0) {
			*spi_vars.pNextRxByte = SPI0_POPR;
		}
		break;
	case SPI_BUFFER:
		*spi_vars.pNextRxByte    = SPI0_POPR;
		spi_vars.pNextRxByte++;
		break;
	case SPI_LASTBYTE:
		*spi_vars.pNextRxByte    = SPI0_POPR;
		break;
	}

	// one byte less to go
	spi_vars.pNextTxByte++;
	spi_vars.numTxedBytes++;
	spi_vars.txBytesLeft--;

	if (spi_vars.txBytesLeft>0) {
		// write next byte to TX buffer
		SPI0_PUSHR              = *spi_vars.pNextTxByte;
	} else {
		// put CS signal high to signal end of transmission to slave
		if (spi_vars.isLast==SPI_LAST) {
			//TODO
		}
		// SPI is not busy anymore
		spi_vars.busy          =  0;

		// SPI is done!
		if (spi_vars.callback!=NULL) {
			// call the callback
			spi_vars.callback();
			// kick the OS
			return 1;
		}
	}
#else
	while(1);// this should never happen
#endif
}


static bool  private_spi_findBaudRate(uint32_t clk_div, uint8_t *p_BR, uint8_t *p_PBR, uint8_t *p_DBR){
	bool  div_found;                                    /* Acceptable values found                           */ 
	uint32_t   best_distance;                                /* Distance between clk_div and calculated div       */ 
	uint8_t   test_BR_idx;                                  /* Auxiliar var to calculate the BR                  */ 
	uint8_t   test_PBR_idx;                                 /* Auxiliar var to calculate the PBR                 */ 
	uint8_t   test_DBR;                                     /* Auxiliar var to calculate the DBR                 */ 
	uint32_t   test_distance;
	uint32_t   test_div;
	uint16_t   BR_values[16] = {2, 4, 6, 8, 16, 32, 64, 128, /* Availabrle values for BR                          */ 
			256, 512, 1024, 2048, 4096,
			8192, 16384, 32768};
	uint8_t   PBR_values[4] = {2, 3, 5, 7};                 /* Availabrle values for PBR                         */ 

	/* Try out all the Baud Rates and keep best fit      */ 
	div_found = FALSE;
	best_distance = clk_div + 1;                               /* Initialize distance beyound maximum value         */ 
	for(test_BR_idx = 0u; test_BR_idx < 16; test_BR_idx++){
		for(test_PBR_idx = 0u; test_PBR_idx < 4; test_PBR_idx++){
			for(test_DBR = 0u; test_DBR < 2; test_DBR++){

				test_div = BR_values[test_BR_idx] * PBR_values[test_PBR_idx] / (1 + test_DBR);
				if(test_div < clk_div){                        /* See note #2                                       */ 
					continue;
				}
				test_distance = test_div - clk_div;
				if(test_distance < best_distance){             /* Keep test values if better than the previous.     */ 
					div_found = TRUE;
					best_distance = test_distance;
					*p_BR         = test_BR_idx;
					*p_PBR        = test_PBR_idx;
					*p_DBR        = test_DBR;
				}
			}
		}
	}

	return div_found;
}


static bool  private_spi_calcDelayParams(uint32_t  pclk_freq, uint32_t  delay, uint8_t *p_scaler, uint8_t *p_prescaler){
	bool  delay_found;                                  /* Acceptable values found                           */ 
	uint32_t   best_distance;                                /* Distance between clk_div and calculated div       */ 
	uint8_t   test_scaler_idx;                              /* Auxiliar var to calculate the BR                  */ 
	uint8_t   test_prescaler_idx;                           /* Auxiliar var to calculate the PBR                 */ 
	uint32_t   test_distance;
	uint32_t   test_delay;
	uint32_t   ns_per_cycle;                                 /* Number of nanoseconds in a pclk cycle             */
	uint32_t   scaler_values[16] = {2, 4, 8, 16, 32, 64, 128,/* Availabrle values for scaler                      */ 
			256, 512, 1024, 2048, 4096,
			8192, 16384, 32768, 65536};
	uint8_t   prescaler_values[4] = {1, 3, 5, 7};           /* Availabrle values for prescaler                   */ 

	ns_per_cycle = 1000000000/pclk_freq;
	if (0 == ns_per_cycle) {                                   /* Note #3                                           */
		ns_per_cycle = 1;
	}

	/* Try out all the Baud Rates and keep best fit      */ 
	delay_found   = FALSE;
	best_distance = 0xFFFFFFFF;                                /* Initialize distance beyound maximum value         */ 
	for(test_scaler_idx = 0u; test_scaler_idx < 16; test_scaler_idx++){
		for(test_prescaler_idx = 0u; test_prescaler_idx < 4; test_prescaler_idx++){

			test_delay = scaler_values[test_scaler_idx] * prescaler_values[test_prescaler_idx] * ns_per_cycle;
			if(test_delay < delay){                            /* See note #2                                       */ 
				continue;
			}
			test_distance = test_delay - delay;
			if(test_distance < best_distance){                 /* Keep test values if better than the previous.     */ 
				delay_found   = TRUE;
				best_distance = test_distance;
				*p_scaler     = test_scaler_idx;
				*p_prescaler  = test_prescaler_idx;
			}
		}
	}

	return delay_found;
}