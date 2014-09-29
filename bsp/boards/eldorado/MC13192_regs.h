/*!
 * Copyright (c) 2005, Freescale Semiconductor
 *
 * Freescale Confidential Proprietary
 * \file    MC13192_regs.h
 * \brief   Defines the MC13192 registers.
 * \author  a19259
 * \version 4.1a
 * \date    2005/07/29 02:27:50 
 *
 * \b Department: Freescale Radio Products Division 
 *
 * \b Project: SMAC (Simple Media Access Controller)
 *
 * \b History:
 * - 16/11/2005 Doc. update to Doxygen compliant by Mario Rodriguez r12369c
 */ 
 
#ifndef _MC13192_REGS_H
#define _MC13192_REGS_H

/*! MC13192 soft reset */
#define RESET                   0x00		/*!< Reset */

/* Packet RAM */
#define RX_PKT                  0x01    /*!< RX Packet RAM */
#define RX_PKT_LEN              0x2D    /*!< RX Packet RAM Length [6:0] */  
#define TX_PKT                  0x02    /*!< TX Packet RAM */
#define TX_PKT_LEN              0x03    /*!< TX Packet RAM Length */
#define TX_PKT_LEN_MASK         0x007F  /*!< TX Packet RAM Length Mask */

/* IRQ Status Register */
#define IRQ_MASK                0x05		/*!< IRQ Mask */
#define STATUS_ADDR             0x24		/*!< Status bits that can cause an interrupt request when enabled */
#define RESIND_ADDR             0x25		/*!< Reset indicator bit */
#define TIMER1_IRQMASK_BIT      0x0001  /*!< Timer1 IRQ mask bit */

/* Mask and mode */
#define MODE_ADDR               0x06    /*!< Control fields for the MC13192/93 */
#define MODE2_ADDR              0x07    /*!< Control fields for the MC13192/93 */ 
#define LO1_COURSE_TUNE         0x8000  /*!< LO1 course tune */ 

/* Main Timer */
#define TIMER_PRESCALE          0x09    /*!< Timer Prescale */
#define TIMESTAMP_HI_ADDR       0x26    /*!< Timestamp hi address */
#define TIMESTAMP_LO_ADDR       0x27		/*!< Timestamp lo address */
#define TIMESTAMP_HI_MASK       0x00FF  /*!< Timestamp hi mask */

/* Frequency */
#define XTAL_ADJ_ADDR           0x0A		/*!< Xtal trim and clock rate */
#define CLKS_ADDR               0x0A		/*!< Xtal trim and clock rate */
#define LO1_IDIV_ADDR           0x0F		/*!< LO1 fractional-N synthesizer that sets transceiver channel freq */
#define LO1_NUM_ADDR            0x10		/*!< Sets transceiver channel frequency */
#define PRESCALE_ADDR           0x09		/*!< tmr_prescale[2:0] select the frequency of the base clock for the Event Timer */

/* Timer comparators */
#define T1_HI_ADDR              0x1B    /*!< Timer1 comparator most significant 8 bits of the 24-bit compare */
#define T1_LO_ADDR              0x1C		/*!< Timer1 comparator least significant 16 bits */
#define T2_HI_ADDR              0x1D    /*!< Disable bit for Timer Comparator 2 and stores the most significant 8 bits */
#define T2_LO_ADDR              0x1E		/*!< least significant 16 bits of the 24-bit compare value.*/

/* CCA */
#define CCA_THRESHOLD           0x04	  /*!< Threshold value for Clear Channel Assessment in dB-linear format */
#define CCA_RESULT_ADDR         0x2D		/*!< Bits 15-8 cca_final [7:0]  Average CCA energy */
#define FEGAIN_ADDR             0x04		/*!< added to the measured value of the CCA operation */

/* TX */
#define PA_ADJUST_ADDR          0x12		/*!< sets the power level and drive level of the transmitter power amplifier.*/

/* GPIO */
#define GPIO_CONFIG             0x0B		/*!< config. data direction + sets output drive strength of GPIO1 through GPIO4*/
#define GPIO_DATA               0x0C		/*!< sets output value if GPIO configured as output + set the output drive strength of GPIO5 through GPIO7*/
#define GPIO_DATA_MASK          0x003F  /*!< GPIO data mask */

/* Version */
#define VERSION_REG             0x2C		/*!< 9-bit chip version code */
#define VERSION_MASK            0x1C00  /*!< Version mask */

/******* Test registers *******/
#define BER_REG                 0x30    /*!< Bit error rate register */
#define BER_MASK                0x8000   /*!< Bit error rate mask */
#define PSM_REG                 0x31     /*!< PSM register */
#define PSM_MASK                0x0008   /*!< PSM mask */
#define PAEN_REG                0x08     /*!< PA enable register */
#define PAEN_MASK               0x8000   /*!< PA enable mask  */

#endif /* _MC13192_REGS_H */