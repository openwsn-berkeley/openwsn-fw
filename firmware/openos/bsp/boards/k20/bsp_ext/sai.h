/*
 *	File: sai.h
 *	Purpose: SAI module driver
 *	Author: Wang Hao (B17539)
 * 
*/

#ifndef __SAI_H
#define __SAI_H

/* Macros */

/* Configure whether there is dual SAI port or not */
#define SAI_DUAL_PORT	0

#define FIFO_EMPTY	0
#define FIFO_FULL	1

#define ERROR	0xff
#define OK		0x00


/* transmit or receive */
#define TX	1
#define RX	0

#define BUFFER_SIZE 32
#define SAI_FIFO_SIZE 8
#define ENTRY_CNT	4

//PTE0
#define LED0_CONFIG_GPIO (PORTE_PCR(0) = PORT_PCR_MUX(1))
#define	LED0_SET_OUTPUT	(GPIOE_PDDR |= 0x01)
#define	LED0_SET_HIGH	(GPIOE_PDOR |= 0x01)
#define	LED0_SET_LOW	(GPIOE_PDOR &= ~0x01)
#define	LED0_TOGGLE		(GPIOE_PTOR |= 0x01)

//PTE1
#define LED1_CONFIG_GPIO (PORTE_PCR(1) = PORT_PCR_MUX(1))
#define	LED1_SET_OUTPUT	(GPIOE_PDDR |= 0x02)
#define	LED1_SET_HIGH	(GPIOE_PDOR |= 0x02)
#define	LED1_SET_LOW	(GPIOE_PDOR &= ~0x02)
#define	LED1_TOGGLE		(GPIOE_PTOR |= 0x02)

//PTE2
#define LED2_CONFIG_GPIO (PORTE_PCR(2) = PORT_PCR_MUX(1))
#define	LED2_SET_OUTPUT	(GPIOE_PDDR |= 0x04)
#define	LED2_SET_HIGH	(GPIOE_PDOR |= 0x04)
#define	LED2_SET_LOW	(GPIOE_PDOR &= ~0x04)
#define	LED2_TOGGLE		(GPIOE_PTOR |= 0x04)

/* typedefs */
/* SAI FIFO watermark */
typedef enum {
	FIFO_WM_ZERO = 0,
	FIFO_WM_ONE,
	FIFO_WM_TWO,
	FIFO_WM_THREE,
	FIFO_WM_FOUR,
	FIFO_WM_FIVE,
	FIFO_WM_SIX,
	FIFO_WM_SEVEN
} SAI_FIFO_WATERMARK;

/* SAI sync mode */
typedef enum {
	ASYNC_MODE = 0,
	SYNC_WITH_RECV,
	SYNC_WITH_ANOTHER_SAI_TRANS,
	SYNC_WITH_ANOTHER_SAI_RECV
} SAI_SYNC_MODE;

/* master clock select */
typedef enum {
	BUS_CLOCK = 0,
	I2S0_MCLK,
	I2S1_MCLK
} SAI_MASTERCLK_SEL;

/* Bit clock polarity */
typedef enum {
	BIT_CLOCK_ACTIVE_HIGH = 0,
	BIT_CLOCK_ACTIVE_LOW
} SAI_BITCLK_POLARITY;

/* Bit clock direction */
typedef enum {
	BIT_CLOCK_EXTERNAL = 0, 
	BIT_CLOCK_INTERNAL
} SAI_BITCLK_DIR;

/* Channel enable */
typedef enum {
	CHANNEL_ONE_EN = 1,
	CHANNEL_TWO_EN = 2,
	CHANNEL_BOTH_EN = 3
} SAI_CHANNEL_EN;

/* MSB or LSB first */
typedef enum {
	LSB_TX_RX_FIRST = 0,
	MSB_TX_RX_FIRST
} SAI_MSB_FIRST;

/* frame sync early */
typedef enum {
	FS_ASSERT_FIRST_BIT = 0,
	FS_ASSERT_EARLY
} SAI_FRAME_SYNC_EARLY;

/* frame sync polarity */
typedef enum {
	FS_ACTIVE_HIGH = 0,
	FS_ACTIVE_LOW
} SAI_FRAME_SYNC_POLARITY;

/* frame sync direction */
typedef enum {
	FS_EXTERNAL = 0,
	FS_INTERNAL
} SAI_FRAME_SYNC_DIR;

/* master clock input */
typedef enum {
	SYSTEM_CLOCK = 0,
	OSC0ERCLK,
	OSC1ERCLK,
	MCGPLLCLK
} SAI_MCLK_INPUT;

/* Global variables */

extern unsigned int tx_buffer[BUFFER_SIZE];
extern unsigned int rx_buffer[BUFFER_SIZE];

extern unsigned int tx_index;
extern unsigned int rx_index;

/* Function prototypes */
void sai_pinmux_init(unsigned char port, unsigned char setting);
void sai_stop_dbg_enable(unsigned char port, unsigned char tx, unsigned char stope, unsigned char dbge);
void sai_fifo_reset(unsigned char port, unsigned char tx);
void sai_soft_reset(unsigned char port, unsigned char tx);
void sai_interrupt_enable(unsigned char port, unsigned char tx);
void sai_clear_flags(unsigned char port, unsigned char tx);
void sai_fifo_request_dma_enable(unsigned char port, unsigned char tx);
void sai_fifo_warning_dma_enable(unsigned char port, unsigned char tx);
void sai_enable(unsigned char port, unsigned char tx);
void sai_disable(unsigned char port, unsigned char tx);
void sai_watermark_config(unsigned char port, unsigned char tx, unsigned char watermark);
void sai_mode_config(unsigned char port, unsigned char tx, unsigned char mode, unsigned char bcs, unsigned char bci);
void sai_bclk_config(unsigned char port, unsigned char tx, unsigned char mclksel, unsigned char bclkpol, unsigned char bclkdir, unsigned char bclkdiv);
void sai_bclk_en(unsigned char port, unsigned tx);
void sai_bclk_dis(unsigned char port, unsigned tx);
void sai_channel_enable(unsigned char port, unsigned char tx, unsigned char chen);
void sai_channel_disable(unsigned char port, unsigned char tx, unsigned char chen);
void sai_word_flg_config(unsigned char port, unsigned char tx, unsigned char wdfl);
void sai_frame_config(unsigned char port, unsigned char tx, unsigned char framesize, unsigned char sywd, unsigned char mf);
void sai_frameclk_config(unsigned char port, unsigned char tx, unsigned char fse, unsigned char fsp, unsigned char fsd);
void sai_wordlength_config(unsigned char port, unsigned char tx, unsigned char wnw, unsigned char w0w, unsigned char fbt);
unsigned char sai_tx(unsigned char port, unsigned char channel, unsigned int txdata);
unsigned char sai_rx(unsigned char port, unsigned char channel, unsigned int *rxdata);
void sai_wr_tdr(unsigned char port, unsigned char channel, unsigned int txdata);
void sai_rd_rdr(unsigned char port, unsigned char channel, unsigned int *rxdata);
unsigned char sai_tx_fifo_status(unsigned char port, unsigned char channel);
unsigned char sai_rx_fifo_status(unsigned char port, unsigned char channel);
void sai_mask_config(unsigned char port, unsigned char tx, unsigned int mask);
void sai_mclk_config(unsigned char port, unsigned char mics, unsigned char moe, unsigned char fract, unsigned char divide);
void sai_clear_reg(unsigned char port);
void sai_monitor_fifo_status(unsigned char port, unsigned char channel);
void sai_inteprete_flag(unsigned char port, unsigned char tx);

/* Interrupt service routines */
void sai0_tx_isr(void);
void sai0_rx_isr(void);

void sai1_tx_isr(void);
void sai1_rx_isr(void);

#endif	/* __SAI_H */
