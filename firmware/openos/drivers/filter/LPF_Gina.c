/******************************************************************************
;   Lattice Wave Digital Filter to perform Low Pass filtering
;
;   Description; This code calls a Lattice Wave Digital Filter assembly function 
;                wdf_ex1 and obtains the gain for frequency starting from 50Hz
;                to 7950 Hz. The filter coefficients conform with the LPF 
;                specifications of Example 1 in the Application Report. 
;
;   Note:        This code assumes 32.768 KHz XTAL on LFXT1 on the MSP430FG439 
;
;   K. Venkat
;   Texas Instruments Inc.
;   August 2006
;   Built with IAR Embedded Workbench Version: 3.41A
;*******************************************************************************/

#include "sine_data_ex1.dat"  
#include  <stdio.h>
#include  <msp430xG43x.h>
int wdf_ex1(int);
void DCO();
int delay0,delay1, delay2,delay3, delay4,delay5,delay6,delay7,delay8,input,output; 
int i,outer_loop; 
float sum=0,gain[44],finalout;

main()
{
      WDTCTL = WDTPW + WDTHOLD;         //    Stop WDT.
      FLL_CTL0 |= XCAP18PF;             //    Configure load caps.
      DCO();                            //    Configure the CPU MCLK to 8 MHz
      
      // Frequency sweep starts here
      
      for (outer_loop=0;outer_loop<44;outer_loop++) 
      {
          delay0=0;                     //   Delay elements initialized to zero
          delay1=0;                     //   at the start of each frequency
          delay2=0; 
          delay3=0;
          delay4=0; 
          delay5=0;
          delay6=0; 
          delay7=0;
          delay8=0; 
          sum=0;
          for (i=0;i<400;i++)           //  400 samples for each frequency
          {
              input=(data[(outer_loop)*400+i]);
              wdf_ex1(input);           //  Wave Digital filter called for each 
                                        //  sample to output one sample  
              finalout=output/2047.0;   //  Converted to Floating point before 
                                        //  gain calculation 
              sum=sum+finalout*finalout;//  Gain calculated with simple square and add  
          }
          gain[outer_loop]=sum;         //  Gain stored to test filter response
          printf("%f\n",sum);           //  Open Terminal I/O to view gain 
     }
}
                                   




   
