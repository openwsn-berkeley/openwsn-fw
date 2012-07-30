/*
 * File:        rcm.c
 * Purpose:     Provides routines for the reset controller module
 *              
 */

#include "common.h"
#include "rcm.h"



void software_reset(){
	uint32_t temp_AIRCR=SCB_AIRCR;
	temp_AIRCR &= 0x0000FFFF; //DES mask of the top 16-bits
	temp_AIRCR |= 0x05FA0000; //idem to temp_AIRCR|=SCB_AIRCR_VECTKEY(0xFA05);
	temp_AIRCR |=SCB_AIRCR_SYSRESETREQ_MASK; //appl request to reset
	SCB_AIRCR = temp_AIRCR; //reboot.
}


#ifdef TOWER_K20
/* OutSRS routine - checks the value in the SRS registers and sends
 * messages to the terminal announcing the status at the start of the 
 * code.
 */
void outSRS(void){                         //[outSRS]

  
  /* 	if (RCM_SRS1 & RCM_SRS1_TAMPER_MASK)
		printf("Tamper Detect Reset\n");*/
  
	if (RCM_SRS1 & RCM_SRS1_SACKERR_MASK)
		printf("Stop Mode Acknowledge Error Reset\n");
	if (RCM_SRS1 & RCM_SRS1_EZPT_MASK)
		printf("EzPort Reset\n");
	if (RCM_SRS1 & RCM_SRS1_MDM_AP_MASK)
		printf("MDM-AP Reset\n");
	if (RCM_SRS1 & RCM_SRS1_SW_MASK)
		printf("Software Reset\n");
	if (RCM_SRS1 & RCM_SRS1_LOCKUP_MASK)
		printf("Core Lockup Event Reset\n");
	if (RCM_SRS1 & RCM_SRS1_JTAG_MASK)
		printf("JTAG Reset\n");
	
	if (RCM_SRS0 & RCM_SRS0_POR_MASK)
		printf("Power-on Reset\n");
	if (RCM_SRS0 & RCM_SRS0_PIN_MASK)
		printf("External Pin Reset\n");
	if (RCM_SRS0 & RCM_SRS0_WDOG_MASK)
		printf("Watchdog(COP) Reset\n");
	if (RCM_SRS0 & RCM_SRS0_LOC_MASK)
		printf("Loss of Clock Reset\n");
	if (RCM_SRS0 & RCM_SRS0_LVD_MASK)
		printf("Low-voltage Detect Reset\n");
	if (RCM_SRS0 & RCM_SRS0_WAKEUP_MASK)
        {
          printf("[outSRS]Wakeup bit set from low power mode exit\n");
          printf("[outSRS]SMC_PMPROT   = %#02X \r\n", (SMC_PMPROT))  ;
          printf("[outSRS]SMC_PMCTRL   = %#02X \r\n", (SMC_PMCTRL))  ;
          printf("[outSRS]SMC_VLLSCTRL   = %#02X \r\n", (SMC_VLLSCTRL))  ;
          printf("[outSRS]SMC_PMSTAT   = %#02X \r\n", (SMC_PMSTAT))  ;

          if ((SMC_PMCTRL & SMC_PMCTRL_STOPM_MASK)== 3)
            printf("[outSRS] LLS exit \n") ;
          if (((SMC_PMCTRL & SMC_PMCTRL_STOPM_MASK)== 4) && ((SMC_VLLSCTRL & SMC_VLLSCTRL_VLLSM_MASK)== 1))
            printf("[outSRS] VLLS1 exit \n") ;
          if (((SMC_PMCTRL & SMC_PMCTRL_STOPM_MASK)== 4) && ((SMC_VLLSCTRL & SMC_VLLSCTRL_VLLSM_MASK)== 2))
            printf("[outSRS] VLLS2 exit \n") ;
          if (((SMC_PMCTRL & SMC_PMCTRL_STOPM_MASK)== 4) && ((SMC_VLLSCTRL & SMC_VLLSCTRL_VLLSM_MASK)== 3))
            printf("[outSRS] VLLS3 exit \n") ; 
	}

        if ((RCM_SRS0 == 0) && (RCM_SRS1 == 0)) 
        {
	       printf("[outSRS]RCM_SRS0 is ZERO   = %#02X \r\n", (RCM_SRS0))  ;
	       printf("[outSRS]RCM_SRS1 is ZERO   = %#02X \r\n", (RCM_SRS1))  ;	 
        }
  }

void outsrs(void){
	char i;
	/* OutSRS routine - checks the value in the SRS registers and sends
	 * messages to two pins that indicate the bits set in the two SRS  
	 * registers.  PORT C19 and PORT E26 are used in this driver
         * initialized in Port_init(); 
	 */ 
	//	void outSRS(void){                         //[outSRS]
        //used for wakeup testing
        PORTE_PCR26 = (PORT_PCR_MUX(01) |    //  GPIO
                       PORT_PCR_IRQC(0x00) |  //  falling edge enable
                       PORT_PCR_DSE_MASK);    // enable high drive strength
        GPIOE_PDDR  |= 0x04000000;           // set Port E 26 as output
        GPIOE_PCOR = 0x04000000;             // set Port E 26 indicate wakeup - clears it in llwu_isr
        //used for wakeup testing
        PORTC_PCR19 = (PORT_PCR_MUX(01) |    //  GPIO
                       PORT_PCR_IRQC(0x00) |  //  falling edge enable
                       PORT_PCR_DSE_MASK);    // enable high drive strength
        GPIOC_PDDR  |= 0x00080000;           // set Port C 19 as output
        GPIOC_PSOR   = 0x00080000;             // set Port C 19 SRS output outsrs uses this pin
	
 	if (RCM_SRS0 & RCM_SRS0_POR_MASK){
		for (i=0;i<8;i++){
                          GPIOE_PTOR = 0x04000000;           
                          GPIOE_PTOR = 0x04000000;           
		}
	}
	if (RCM_SRS0 & RCM_SRS0_PIN_MASK){
			for (i=0;i<7;i++){
                          GPIOE_PTOR = 0x04000000;           
                          GPIOE_PTOR = 0x04000000;           
			}
	}
	if (RCM_SRS0 & RCM_SRS0_WDOG_MASK){
			for (i=0;i<6;i++){
                          GPIOE_PTOR = 0x04000000;           
                          GPIOE_PTOR = 0x04000000;           
			}
	}
	if (RCM_SRS0 & RCM_SRS0_LOC_MASK){
			for (i=0;i<3;i++){
                          GPIOE_PTOR = 0x04000000;           
                          GPIOE_PTOR = 0x04000000;           
			}
	}
	if (RCM_SRS0 & RCM_SRS0_LVD_MASK){
			for (i=0;i<2;i++){
                          GPIOE_PTOR = 0x04000000;           
                          GPIOE_PTOR = 0x04000000;           
			}
	}
	if (RCM_SRS0 & RCM_SRS0_WAKEUP_MASK){
                        GPIOE_PTOR = 0x04000000;           
                        GPIOE_PTOR = 0x04000000;           
	}
        GPIOC_PCOR   = 0x00080000;             // set Port C 19 SRS output outsrs uses this pin

/*	if (RCM_SRS1 & RCM_SRS1_TAMPER_MASK){
			for (i=0;i<8;i++){
                          GPIOE_PTOR = 0x04000000;           
                          GPIOE_PTOR = 0x04000000;           
			}
	}*/
        
        
	if (RCM_SRS1 & RCM_SRS1_SACKERR_MASK){
			for (i=0;i<6;i++){
                           GPIOE_PTOR = 0x04000000;           
                           GPIOE_PTOR = 0x04000000;           
			}
	}
	if (RCM_SRS1 & RCM_SRS1_EZPT_MASK){
			for (i=0;i<5;i++){
                          GPIOE_PTOR = 0x04000000;           
	                  GPIOE_PTOR = 0x04000000;           
			}
	}
	if (RCM_SRS1 & RCM_SRS1_MDM_AP_MASK){
			for (i=0;i<4;i++){
                          GPIOE_PTOR = 0x04000000;           
	                  GPIOE_PTOR = 0x04000000;           
			}
	}
	if (RCM_SRS1 & RCM_SRS1_SW_MASK){
			for (i=0;i<3;i++){
                          GPIOE_PTOR = 0x04000000;           
	                  GPIOE_PTOR = 0x04000000;           
			}
	}
	if (RCM_SRS1 & RCM_SRS1_LOCKUP_MASK){
			for (i=0;i<2;i++){
                          GPIOE_PTOR = 0x04000000;           
                          GPIOE_PTOR = 0x04000000;           
			}
	}
	if (RCM_SRS1 & RCM_SRS1_JTAG_MASK){
                        GPIOE_PTOR = 0x04000000;           
	                GPIOE_PTOR = 0x04000000;           
	}
 	GPIOC_PCOR = 0x04000000;   // output SRS1 done
       
}
#endif