
 /**
\brief K20-specific definition of the "LLWU" bsp module. Revision of Philip Drake(rxaa60) version.
\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, May 2012.
*/

#include "common.h"
#include "llwu.h"
#include "board.h"


extern int re_init_clk;
extern int mcg_clk_hz;
extern int mcg_clk_khz;
extern int core_clk_khz;


void llwu_init(void)
{   
#ifdef OPENMOTE_K20
	SIM_SCGC4 |= SIM_SCGC4_LLWU_MASK; //power llwu, in k20 72Mhz is always powered as there isn't a register to power it. in k20 100Mhz needs to be clocked.
#endif	
    enable_irq(LLWU_IRQ_NUM);
    /*0x0200|0x0001 == LLWU_PE3_WUPE9 is radio isr and LLWU_PE1_WUPE0 is UART RX */
    llwu_configure( 0x0200|0x0001, LLWU_PIN_RISING, LLWU_ME_WUME0_MASK);//lptmr is wume0 manual page 76       
}

/*******************************************************************************
*
*   PROCEDURE NAME:
*       llwu_reset_enable -
*
*******************************************************************************/
void llwu_reset_enable(void)
{
#ifdef TOWER_K20
	// printf(" LLWU Reset pin enabled as wakeup source from Low power modes \n");
    LLWU_RST = LLWU_RST_LLRSTE_MASK;   //no reset filter for now
#endif    
}


/*******************************************************************************
*
*   PROCEDURE NAME:
*       llwu_configure -
*
*******************************************************************************/
/* function: llwu_configure

   description: Set up the LLWU for wakeup the MCU from LLS and VLLSx modes
   from the selected pin or module.

   inputs:
   pin_en - unsigned integer, bit position indicates the pin is enabled.
            More than one bit can be set to enable more than one pin at a time.

   rise_fall - 0x00 = External input disabled as wakeup
               0x01 - External input enabled as rising edge detection
               0x02 - External input enabled as falling edge detection
               0x03 - External input enablge as any edge detection
   module_en - unsigned char, bit position indicates the module is enabled.
               More than one bit can be set to enabled more than one module

   for example:  if bit 0 and 1 need to be enabled as rising edge detect call this  routine with
   pin_en = 0x0003 and rise_fall = 0x02

   Note: to set up one set of pins for rising and another for falling, 2 calls to this
         function are required, 1st for rising then the second for falling.

*/
void llwu_configure(unsigned int pin_en, unsigned char rise_fall, unsigned char module_en )
{
    uint8_t temp;

    temp = LLWU_PE1;
    if( pin_en & 0x0001)
    {
        temp |= LLWU_PE1_WUPE0(rise_fall);
        // LLWU configured pins PTE1/UART1_RX/I2C1_SCL /SPI1_SIN to be an LLWU wakeup source 
        LLWU_F1 |= LLWU_F1_WUF0_MASK;   // write one to clear the flag
    }
    if( pin_en & 0x0002)
    {
        temp |= LLWU_PE1_WUPE1(rise_fall);
      //   LLWU configured pins PTE2/SPI1_SCK/SDHC0_DCLK to be an LLWU wakeup source 
        LLWU_F1 |= LLWU_F1_WUF1_MASK;   // write one to clear the flag
    }
    if( pin_en & 0x0004)
    {
        temp |= LLWU_PE1_WUPE2(rise_fall);
        //   LLWU configured pins PTE4/SPI1_PCS0/SDHC0_D3 to be an LLWU wakeup source 
        LLWU_F1 |= LLWU_F1_WUF2_MASK;   // write one to clear the flag
    }
    if( pin_en & 0x0008)
    {
        temp |= LLWU_PE1_WUPE3(rise_fall);
        //   LLWU configured pins PTA4/FTM0_CH1/NMI/EZP_CS to be an LLWU wakeup source 
        LLWU_F1 |= LLWU_F1_WUF3_MASK;   // write one to clear the flag
    }
    LLWU_PE1 = temp;

    temp = LLWU_PE2;
    if( pin_en & 0x0010)
    {
        temp |= LLWU_PE2_WUPE4(rise_fall);
        //  LLWU configured pins PTA13/FTM1_CH1 /FTM1_QD_PHB to be an LLWU wakeup source 
        LLWU_F1 |= LLWU_F1_WUF4_MASK;   // write one to clear the flag
    }
    if( pin_en & 0x0020)
    {
        temp |= LLWU_PE2_WUPE5(rise_fall);
        //  LLWU configured pins PTB0/I2C0_SCL/FTM1_CH0/FTM1_QD_PHA to be an LLWU wakeup source 
        LLWU_F1 |= LLWU_F1_WUF5_MASK;   // write one to clear the flag
    }
    if( pin_en & 0x0040)
    {
        temp |= LLWU_PE2_WUPE6(rise_fall);
        //   LLWU configured pins PTC1/UART1_RTS/FTM0_CH0 to be an LLWU wakeup source 
        LLWU_F1 |= LLWU_F1_WUF6_MASK;   // write one to clear the flag
    }
    if( pin_en & 0x0080)
    {
        temp |= LLWU_PE2_WUPE7(rise_fall);
        //  LLWU configured pins PTC3/UART1_RX/FTM0_CH2 to be an LLWU wakeup source 
        LLWU_F1 |= LLWU_F1_WUF7_MASK;   // write one to clear the flag
    }
    LLWU_PE2 = temp;

    temp = LLWU_PE3;
    if( pin_en & 0x0100)
    {
        temp |= LLWU_PE3_WUPE8(rise_fall);
        //  LLWU configured pins PTC4/SPI0_PCS0/FTM0_CH3 to be an LLWU wakeup source 
        LLWU_F2 |= LLWU_F2_WUF8_MASK;   // write one to clear the flag
    }
    if( pin_en & 0x0200)
    {
        temp |= LLWU_PE3_WUPE9(rise_fall);
        //  LLWU configured pins PTC5/SPI0_SCK/I2S0_RXD0 to be an LLWU wakeup source 
        LLWU_F2 |= LLWU_F2_WUF9_MASK;   // write one to clear the flag
    }
    if( pin_en & 0x0400)
    {
        temp |= LLWU_PE3_WUPE10(rise_fall);
        // LLWU configured pins PTC6/PDB0_EXTRG to be an LLWU wakeup source 
        LLWU_F2 |= LLWU_F2_WUF10_MASK;   // write one to clear the flag
    }
    if( pin_en & 0x0800)
    {
        temp |= LLWU_PE3_WUPE11(rise_fall);
        // LLWU configured pins PTC11/I2S0_RXD1 to be an LLWU wakeup source 
        LLWU_F2 |= LLWU_F2_WUF11_MASK;   // write one to clear the flag
    }
    LLWU_PE3 = temp;

    temp = LLWU_PE4;
    if( pin_en & 0x1000)
    {
        temp |= LLWU_PE4_WUPE12(rise_fall);
        //   LLWU configured pins PTD0/SPI0_PCS0/UART2_RTS to be an LLWU wakeup source 
        LLWU_F2 |= LLWU_F2_WUF12_MASK;   // write one to clear the flag
    }
    if( pin_en & 0x2000)
    {
        temp |= LLWU_PE4_WUPE13(rise_fall);
        //  LLWU configured pins PTD2/UART2_RX to be an LLWU wakeup source 
        LLWU_F2 |= LLWU_F2_WUF13_MASK;   // write one to clear the flag
    }
    if( pin_en & 0x4000)
    {
        temp |= LLWU_PE4_WUPE14(rise_fall);
        //   LLWU configured pins PTD4/UART0_RTS/FTM0_CH4/EWM_IN to be an LLWU wakeup source 
        LLWU_F2 |= LLWU_F2_WUF14_MASK;   // write one to clear the flag
    }
    if( pin_en & 0x8000)
    {
        temp |= LLWU_PE4_WUPE15(rise_fall);
        // LLWU configured pins PTD6/UART0_RX/FTM0_CH6/FTM0_FLT0 to be an LLWU wakeup source 
        LLWU_F2 |= LLWU_F2_WUF15_MASK;   // write one to clear the flag
    }
    LLWU_PE4 = temp;

    LLWU_ME = module_en;  //Set up modules to wakeup up


}


/*******************************************************************************
*
*   PROCEDURE NAME:
*       llwu_configure_filter -
*
*******************************************************************************/
void llwu_configure_filter(unsigned int wu_pin_num, unsigned char filter_en, unsigned char rise_fall )
{
   //wu_pin_num is the pin number to be written to FILTSEL.  wu_pin_num is not the same as pin_en.
    uint8_t temp;

    //Enabling Filter %x on WU Pin %x for WU sense %x \n",filter_en, wu_pin_num, rise_fall);

     temp = 0;
     //first clear filter values and clear flag by writing a 1
#ifdef TOWER_K20
     LLWU_FILT1 = LLWU_FILT1_FILTF_MASK;
     LLWU_FILT2 = LLWU_FILT2_FILTF_MASK;

     if(filter_en == 1)
     {
         //clear the flag bit and set the others
         temp |= (LLWU_FILT1_FILTF_MASK) | (LLWU_FILT1_FILTE(rise_fall) | LLWU_FILT1_FILTSEL(wu_pin_num));
         LLWU_FILT1 = temp;

     }else if (filter_en == 2)
     {
         //clear the flag bit and set the others
         temp |= (LLWU_FILT2_FILTF_MASK) | (LLWU_FILT2_FILTE(rise_fall) | LLWU_FILT2_FILTSEL(wu_pin_num));
         LLWU_FILT2 = temp;
     }else
     {
    	 //Error - invalid filter number %x\n",filter_en);
     }
#endif
     
     
}


/*******************************************************************************
*
*   PROCEDURE NAME:
*       llwu_isr - Interrupt handler for LLWU
*       the commented code is kept as an example for future alternative wake up sources.
*
*******************************************************************************/
void llwu_isr(void)
{

//debugpins_radio_set();

//   if (LLWU_F1 & LLWU_F1_WUF0_MASK) {
//       LLWU_F1 |= LLWU_F1_WUF0_MASK;   // write one to clear the flag -- UART RX flag
////       printf("\n[LLWU ISR]  ****WUF0 was set *****\r\n");
//   }
//   if (LLWU_F1 & LLWU_F1_WUF1_MASK) {
////       printf("\n [LLWU ISR] ****WUF1 was set *****\r\n");
//       LLWU_F1 |= LLWU_F1_WUF2_MASK;   // write one to clear the flag
//   }
//   if (LLWU_F1 & LLWU_F1_WUF2_MASK) {
////       printf("\n [LLWU ISR] ****WUF2 was set *****\r\n");
//       LLWU_F1 |= LLWU_F1_WUF2_MASK;   // write one to clear the flag
//   }
//   if (LLWU_F1 & LLWU_F1_WUF3_MASK){
////       printf("\n [LLWU ISR] ****WUF3 was set *****\r\n");
//       LLWU_F1 |= LLWU_F1_WUF3_MASK;   // write one to clear the flag
//    }
//   if (LLWU_F1 & LLWU_F1_WUF4_MASK) {
// //      printf("\n [LLWU ISR] ****WUF4 was set *****\r\n");
//       LLWU_F1 |= LLWU_F1_WUF4_MASK;   // write one to clear the flag
//   }
//   if (LLWU_F1 & LLWU_F1_WUF5_MASK) {
////       printf("\n [LLWU ISR] ****WUF5 was set *****\r\n");
//       LLWU_F1 |= LLWU_F1_WUF5_MASK;   // write one to clear the flag
//   }
//   if (LLWU_F1 & LLWU_F1_WUF6_MASK) {
////       printf("\n [LLWU ISR] ****WUF6 was set *****\r\n");
//       LLWU_F1 |= LLWU_F1_WUF6_MASK;   // write one to clear the flag
//    }
//   if (LLWU_F1 & LLWU_F1_WUF7_MASK) {
////       printf("\n [LLWU ISR] ****WUF7 was set from PTC3 input  *****\r\n");
//       LLWU_F1 |= LLWU_F1_WUF7_MASK;   // write one to clear the flag
//   }
//   if (LLWU_F2 & LLWU_F2_WUF8_MASK) {
// //      printf("\n [LLWU ISR] ****WUF8 was set *****\r\n");
//       LLWU_F2 |= LLWU_F2_WUF8_MASK;   // write one to clear the flag
//   }
  if (LLWU_F2 & LLWU_F2_WUF9_MASK) {
       LLWU_F2 |= LLWU_F2_WUF9_MASK;   // write one to clear the flag -- radio isr (ptc5 external interrupt)
   	  // signal_irq(RADIO_EXTERNAL_PORT_IRQ_NUM);//activate nvic irq.
       radio_isr(); //instead of that, the isr of that port can be set in the NVIC and there handle the 
       //corresponding action after clearing the LLWU_F2 WUF9 flag. (hence not clearing it here)
  }
//   if (LLWU_F2 & LLWU_F2_WUF10_MASK) {
////       printf("\n [LLWU ISR] ****WUF10 was set *****\r\n");
//       LLWU_F2 |= LLWU_F2_WUF10_MASK;   // write one to clear the flag
//   }
//   if (LLWU_F2 & LLWU_F2_WUF11_MASK) {
// //      printf("\n [LLWU ISR] ****WUF11 was set *****\r\n");
//       LLWU_F2 |= LLWU_F2_WUF11_MASK;   // write one to clear the flag
//   }
//   if (LLWU_F2 & LLWU_F2_WUF12_MASK) {
////       printf("\n [LLWU ISR] ****WUF12 was set *****\r\n");
//       LLWU_F2 |= LLWU_F2_WUF12_MASK;   // write one to clear the flag
//   }
//   if (LLWU_F2 & LLWU_F2_WUF13_MASK) {
////       printf("[LLWU ISR] ****WUF13 was set *****\r\n");
//       LLWU_F2 |= LLWU_F2_WUF13_MASK;   // write one to clear the flag
//   }
//   if (LLWU_F2 & LLWU_F2_WUF14_MASK) {
////       printf("[LLWU ISR] RTS\n");
//       SIM_SCGC5 |= SIM_SCGC5_PORTD_MASK; // turn on port D ajj per alister
//       LLWU_F2 |= LLWU_F2_WUF14_MASK;   // write one to clear the flag
//       PORTD_PCR4 |= PORT_PCR_ISF_MASK  ;   //  clear Flag if there
//   }
//   if (LLWU_F2 & LLWU_F2_WUF15_MASK) {
////       printf("[LLWU ISR] UART0\n");
//       SIM_SCGC5 |= SIM_SCGC5_PORTD_MASK; // turn on port D ajj per alister
//   //    ch = uart_getchar(UART0_BASE_PTR);
//     //  out_char(ch);
//       LLWU_F2 |= LLWU_F2_WUF15_MASK;   // write one to clear the flag
//       PORTD_PCR6  |= PORT_PCR_ISF_MASK  ;   //  clear Flag if there
//   }
//   if (LLWU_F3 & LLWU_F3_MWUF0_MASK) {
////       printf("[LLWU ISR] LPT0\n");
////       LPTMR0_CSR |=  LPTMR_CSR_TCF_MASK;   // write 1 to TCF to clear the LPT timer compare flag
////       LPTMR0_CSR = ( LPTMR_CSR_TEN_MASK | LPTMR_CSR_TIE_MASK | LPTMR_CSR_TCF_MASK  );
//       LLWU_F3 |= LLWU_F3_MWUF0_MASK;   // write one to clear the flag
//   }
//   if (LLWU_F3 & LLWU_F3_MWUF1_MASK) {
////       printf("\n [LLWU ISR] ****WUF3_MWUF1 IF  CMP0  *****\r\n");
//       LLWU_F3 |= LLWU_F3_MWUF1_MASK;   // write one to clear the flag
//   }
//   if (LLWU_F3 & LLWU_F3_MWUF2_MASK) {
////       printf("\n [LLWU ISR] ****WUF3_MWUF2 IF  CMP1 *****\r\n");
//       LLWU_F3 |= LLWU_F3_MWUF2_MASK;   // write one to clear the flag
//   }
//   if (LLWU_F3 & LLWU_F3_MWUF3_MASK) {
////       printf("\n [LLWU ISR] ****WUF3_MWUF3 IF CMP2/CMP3  *****\r\n");
//       LLWU_F3 |= LLWU_F3_MWUF3_MASK;   // write one to clear the flag
//   }
//   if (LLWU_F3 & LLWU_F3_MWUF4_MASK) {
////       printf("\n [LLWU ISR] ****WUF3_MWUF4 IF TSI  *****\r\n");
//       LLWU_F3 |= LLWU_F3_MWUF4_MASK;   // write one to clear the flag
//   }
//   if (LLWU_F3 & LLWU_F3_MWUF5_MASK) {
////       printf("\n [LLWU ISR] ****WUF3_MWUF5 IF RTC Alarm  *****\r\n");
//       LLWU_F3 |= LLWU_F3_MWUF5_MASK;   // write one to clear the flag
//   }
//   if (LLWU_F3 & LLWU_F3_MWUF6_MASK) {
////       printf("\n [LLWU ISR] ****WUF3_MWUF6 IF DryIce(Tamper Detect)  *****\r\n");
//       LLWU_F3 |= LLWU_F3_MWUF6_MASK;   // write one to clear the flag
//   }
//   if (LLWU_F3 & LLWU_F3_MWUF7_MASK) {
////       printf("[LLWU ISR] RTC\n");
//       LLWU_F3 |= LLWU_F3_MWUF7_MASK;   // write one to clear the flag
//   }
//
//   if(LLWU_FILT1 & LLWU_FILT1_FILTF_MASK){
//
//       LLWU_FILT1 |= LLWU_FILT1_FILTF_MASK;
//   }
//   if(LLWU_FILT2 & LLWU_FILT2_FILTF_MASK){
//
//       LLWU_FILT2 |= LLWU_FILT2_FILTF_MASK;
//   }
   
	   LLWU_F1 = 0xFF;            // clear wakeup flags
	   LLWU_F2 = 0xFF;            // clear wakeup flags
	   LLWU_F3 = 0xFF;            // clear wakeup flags
	
   //debugpins_radio_clr();

}
