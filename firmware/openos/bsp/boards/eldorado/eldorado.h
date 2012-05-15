/*
 * $Author: USP Generated pinouts, specific to Eldorado
 * $Date: Feb 2012
 */
 
/* includes */
#include "mc13213.h" //uC
#include "MC13192_regs.h"
#include <hidef.h>
//#include "mc1392.h"  //radio
/* Prototypes */


/*WatchdogDefinitions*/
#define DisableWatchdog SOPT = 0x73;   // disable watchdog timer
#define EnableWatchdog  SOPT = 0xf3;   // enable watchdog timer
#define DEFAULT_SCI_PORT 1
#define ELDORADO_BOARD //specify the board


/*radio IO info*/
#define MC13192_CE              PTED_PTED2                
#define MC13192_CE_PORT         PTEDD_PTEDD2                
#define MC13192_ATTN            PTDD_PTDD0
#define MC13192_ATTN_PORT       PTDDD_PTDDD0
#define MC13192_RTXEN           PTDD_PTDD1
#define MC13192_RTXEN_PORT      PTDDD_PTDDD1
#define MC13192_RESET           PTDD_PTDD3
#define MC13192_RESET_PORT      PTDDD_PTDDD3
#define MC13192_RESET_PULLUP    PTDPE_PTDPE3
#define MC13192_IRQ_SOURCE      IRQSC
#define MC13192_IRQ_IE_BIT      IRQSC_IRQIE

/* Modes defined for the transciever */
#define IDLE_MODE               0x00		/*!< Idle mode */
#define RX_MODE                 0x02		/*!< Receive mode */
#define TX_MODE                 0x03		/*!< Trasmit mode */
#define PULSE_TX_MODE           0x43		/*!< Pulse transmit mode */
#define CCA_MODE                0x01		/*!< CCA mode */
#define RX_MODE_WTO             0x80		/*!< Receive mode wto */
#define HIBERNATE_MODE          0x81		/*!< Hibernate mode */
#define DOZE_MODE               0x82		/*!< Doze mode */
#define IDLE_MODE_DOZE          0x83		/*!< Idle mode doze */
#define IDLE_MODE_ATTN          0x84		/*!< Idle mode attenuation */
#define TIMEOUT                 0x85		/*!< Timeout */
#define SYSTEM_RESET_MODE       0x86    /*!< Not implemented */
#define MC13192_RESET_MODE      0x87    /*!< Not implemented */
#define MC13192_CONFIG_MODE     0x88    /*!< Not implemented */
#define RESET_DELAY             0x89    /*!< Set to 100ms    */

#define TX_IRQ_MASK         0x0040  /*!< Transmit IRQ Mask */
#define RAMERR_IRQ_MASK     0x4000  /*!< RAM add error IRQ Mask */
#define RX_IRQ_MASK         0x0080  /*!< Receive IRQ Mask */
#define ATTN_IRQ_MASK       0x0400  /*!< Attention IRQ Mask */
#define CRC_VALID_MASK      0x0001  /*!< CRC valid Mask */
#define TIMER1_IRQ_MASK     0x0100  /*!< Timer1 IRQ Mask */
#define XCVR_SEQ_MASK       0xFFF8  /*!< Transceiver seq mask */
#define LO_LOCK_IRQ_MASK    0x8000  /*!< Lo lock IRQ Mask */
#define CCA_IRQ_MASK        0x0020  /*!< CCA IRQ Mask */
#define DOZE_IRQ_MASK       0x0200  /*!< Doze IRQ Mask */
#define RESET_BIT_MASK      0x0080  /*!< Reset bit Mask */
#define CCA_BIT_MASK        0x0002


/* Antenna Switch/PA can be implemented via the CT_Bias control line */
/* See the MC1321x reference manual */

#ifdef ANTENNA_SWITCH
    #define MC13192_ANT_CTRL        PTBD_PTBD6
    #define MC13192_ANT_CTRL2       PTBD_PTBD6   /* 
                                                  * Second Control line not
                                                  * used in MC13192EVB
                                                  */
    #define MC13192_ANT_CTRL_PORT   PTBDD_PTBDD6
    #define MC13192_ANT_CTRL2_PORT  PTBDD_PTBDD6 /* 
                                                  * Second Control line not 
                                                  * used in MC13192EVB
                                                  */
    #define ANT_CTRL_OFF            0       /* Logic low is off */
    #define ANT_CTRL_ON             1       /* Logic high is on */
#endif ANTENNA_SWITCH

#ifdef LNA
    #define MC13192_LNA_CTRL        PTBD_PTBD0
    #define MC13192_LNA_CTRL_PORT   PTBDD_PTBDD0
    #define LNA_ON                  1
    #define LNA_OFF                 0
#endif LNA  



/**************************************************************
Enable peripherials that are on the Board
**************************************************************/

#define ACCEL_ENABLED         FALSE
#define TEMP_SENSOR_ENABLED   FALSE
#define BUZZER_ENABLED        TRUE
#define DEFAULT_BUS_SPEED	   8000000

/**************************************************************
Define the SCI perameters
**************************************************************/

#if (DEFAULT_SCI_PORT == 1)
   #define  SCIBDH     SCI1BDH
   #define  SCIBDL     SCI1BDL
   #define  SCIC1      SCI1C1 
   #define  SCIC2      SCI1C2 
   #define  SCIS1      SCI1S1 
   #define  SCIS2      SCI1S2 
   #define  SCIC3      SCI1C3 
   #define  SCID       SCI1D

   #define  SCIS1_TDRE  SCI1S1_TDRE
   #define  SCIS1_TC    SCI1S1_TC

   #define  RTS			PTAD_PTAD1
   #define  RTSDIR		PTADD_PTADD1
#else 

   #define  SCIBDH     SCI2BDH
   #define  SCIBDL     SCI2BDL
   #define  SCIC1      SCI2C1 
   #define  SCIC2      SCI2C2 
   #define  SCIS1      SCI2S1 
   #define  SCIS2      SCI2S2 
   #define  SCIC3      SCI2C3 
   #define  SCID       SCI2D

   #define  SCIS1_TDRE  SCI2S1_TDRE
   #define  SCIS1_TC    SCI2S1_TC

   #define  RTS			PTAD_PTAD6
   #define  RTSDIR		PTADD_PTADD6
#endif 
#define  SCI_DEFAULT_BAUD  38400

/**************************************************************
Define the LED perameters
**************************************************************/
#ifdef ELDORADO_BOARD
#define LED1				  PTAD_PTAD0
#define LED1DIR				PTADD_PTADD0
#define LED2			  	PTAD_PTAD1
#define LED2DIR				PTADD_PTADD1
#define LED3				  PTAD_PTAD2
#define LED3DIR				PTADD_PTADD2
#define LED4				  PTAD_PTAD3
#define LED4DIR				PTADD_PTADD3
#else
#define LED1                    PTDD_PTDD4
#define LED1DIR                 PTDDD_PTDDD4
#define LED2                    PTDD_PTDD5
#define LED2DIR                 PTDDD_PTDDD5
#define LED3                    PTDD_PTDD6
#define LED3DIR                 PTDDD_PTDDD6
#define LED4                    PTDD_PTDD7
#define LED4DIR                 PTDDD_PTDDD7
#define LED5					          PTCD_PTCD4
#define LED5DIR                 PTCDD_PTCDD4
#endif

    
#define LED_ON                  0
#define LED_OFF					1
#define DDIR_OUTPUT             1
#define DDIR_INPUT              0
    
#define LED_INIT_MACRO LED1     = LED_OFF; \
                       LED2     = LED_OFF; \
                       LED3     = LED_OFF; \
                       LED4     = LED_OFF; \
                       LED5     = LED_OFF; \
                       LED1DIR  = DDIR_OUTPUT; \
                       LED2DIR  = DDIR_OUTPUT; \
                       LED3DIR  = DDIR_OUTPUT; \
                       LED4DIR  = DDIR_OUTPUT; \
                       LED5DIR  = DDIR_OUTPUT;        
    
/***********************************************
Define Buzzer
***********************************************/
    
    
#define BUZZER         PTDD_PTDD2
#define BUZZERDIR      PTDDD_PTDDD2
#define BUZZER_ON      1
#define BUZZER_OFF     0
   
#define BUZZER_INIT_MACRO BUZZER      = BUZZER_OFF; \
                          BUZZERDIR   = DDIR_OUTPUT;
    
/***********************************************
Define Switches
***********************************************/
    
    
/* SW1 on schematic */
#define PB0 PTAD_PTAD2
#define PB0PU PTAPE_PTAPE2
#define PB0DIR PTADD_PTADD2
#define PB0IE KBI1PE_KBIPE2
  
/* SW2 on schematic */
#define PB1 PTAD_PTAD3
#define PB1PU PTAPE_PTAPE3
#define PB1DIR PTADD_PTADD3
#define PB1IE KBI1PE_KBIPE3
   
/* SW3 on schematic */
#define PB2 PTAD_PTAD4
#define PB2PU PTAPE_PTAPE4
#define PB2DIR PTADD_PTADD4
#define PB2IE KBI1PE_KBIPE4
  
/* SW4 on schematic */
#define PB3 PTAD_PTAD5
#define PB3PU PTAPE_PTAPE5
#define PB3DIR PTADD_PTADD5
#define PB3IE KBI1PE_KBIPE5

/* Define the KBI Pins */
#define KBI_SW1 0x04
#define KBI_SW2 0x08
#define KBI_SW3 0x10
#define KBI_SW4 0x20

#define SWITCH_INIT_MACRO    PB0PU = TRUE; \
                             PB1PU = TRUE; \
                             PB2PU = TRUE; \
                             PB3PU = TRUE; \
                             PB0DIR = FALSE;\
                             PB1DIR = FALSE;\
                             PB2DIR = FALSE;\
                             PB3DIR = FALSE;
    
/***********************************************
Define Port B
Accelerometer and LCD
***********************************************/

#define ACCEL_X           PTBD_PTBD2
#define ACCEL_Y           PTBD_PTBD3
#define ACCEL_Z           PTBD_PTBD4
#define ACCEL_PS          PTBD_PTBD5
#define ACCEL_GSELECT1    PTBD_PTBD1
#define ACCEL_GSELECT2    PTBD_PTBD0

#define ACCEL_START_X     ATD1SC = 2;
#define ACCEL_START_Y     ATD1SC = 3;
#define ACCEL_START_Z     ATD1SC = 4;     
  
#define ACCEL_INIT_MACRO  PTBDD_PTBDD2 = DDIR_INPUT; \
                          PTBDD_PTBDD3 = DDIR_INPUT; \
                          PTBDD_PTBDD4 = DDIR_INPUT; \
                          PTBDD_PTBDD5 = DDIR_OUTPUT; \
                          PTBDD_PTBDD1 = DDIR_OUTPUT; \
                          PTBDD_PTBDD0 = DDIR_OUTPUT;

#define ACCEL_G_OFF       ACCEL_PS = 0;
#define ACCEL_G_ON        ACCEL_PS = 1; 
#define ACCEL_G_1p5       ACCEL_GSELECT1 = 0; ACCEL_GSELECT2 = 0;
#define ACCEL_G_2p0       ACCEL_GSELECT1 = 0; ACCEL_GSELECT2 = 1;
#define ACCEL_G_4p0       ACCEL_GSELECT1 = 1; ACCEL_GSELECT2 = 0;
#define ACCEL_G_6p0       ACCEL_GSELECT1 = 1; ACCEL_GSELECT2 = 1;
                          
/***********************************************
LCD Settings
***********************************************/
#define LCD_ENABLE        PTBD_PTBD0
#define LCD_REG_SEL       PTBD_PTBD1
#define LCD_EN            PTBD_PTBD2
#define LCD_RW            PTBD_PTBD3
    /* Data bits are 4-7 */
#define LCD_DATA1         PTBD_PTBD4
#define LCD_DATA2         PTBD_PTBD5
#define LCD_DATA3         PTBD_PTBD6
#define LCD_DATA4         PTBD_PTBD7
                              
#define LCD_INIT_MACRO    PTBDD = 0xFF;  /* Set whole port to output */
    
                              
                              
#define ENTER_LOW_POWER _asm stop

/***********************************************
Duplicate PB0 to PUSH_BUTTON1 etc for different
formats.
***********************************************/

#define PUSH_BUTTON1 PB0
#define PUSH_BUTTON2 PB1
#define PUSH_BUTTON3 PB2
#define PUSH_BUTTON4 PB3

#define PB_PRESSED 0





//radio functions etc
//isr
__interrupt void IRQIsr(void);

#ifdef LNA
    #define MC13192_LNA_CTRL        PTBD_PTBD0
    #define MC13192_LNA_CTRL_PORT   PTBDD_PTBDD0
    #define LNA_ON                  1
    #define LNA_OFF                 0
#endif

#define EN_PA1_DIR    PTCDD_PTCDD6 //pa1 enable pin
#define EN_PA2_DIR    PTCDD_PTCDD7  //pa2 enable pin
#define EN_PA1    PTCD_PTCD6   
#define EN_PA2    PTCD_PTCD7  
  
#define IRQFLAG                     IRQSC_IRQF	 /*!< IRQ Flag*/
#define IRQACK()                    IRQSC |= 0x04 /*!< IRQ Acknowledge enabled*/
#define IRQInit()                   IRQSC = 0x14  /*!< Configures IRQ*/
#define IRQPinEnable()              IRQSC = 0x16  /*!<Enables IRQ pin */
#define IRQ_Disable()               IRQSC = 0x00;/*!< Set for negative edge. */
#define MC13192_IRQ_Disable()       MC13192_IRQ_SOURCE = MC13192_IRQ_SOURCE & ~(0x06) /*!<Disables MC13192 transceiver */
#define MC13192_IRQ_Enable()        MC13192_IRQ_SOURCE = MC13192_IRQ_SOURCE | (0x02)  /*!<Enables MC13192 transceiver */
#define CLEAR_IRQ_FLAG()            IRQACK()	 /*!< Clears IRQ flag*/