Stack_Size      EQU     0x0800                            ; 4KB of STACK

                AREA    STACK, NOINIT, READWRITE, ALIGN=4
Stack_Mem       SPACE   Stack_Size
__initial_sp    


Heap_Size       EQU     0x0400                            ; 2KB of HEAP

                AREA    HEAP, NOINIT, READWRITE, ALIGN=4
__heap_base                
Heap_Mem        SPACE   Heap_Size
__heap_limit


; Vector Table Mapped to Address 0 at Reset

                    PRESERVE8
                    THUMB

                        AREA    RESET, DATA, READONLY
                        EXPORT     __Vectors
                    
__Vectors               DCD        __initial_sp
                        DCD        Reset_Handler
                        DCD        0
                        DCD        0
                        DCD        0
                        DCD        0
                        DCD        0
                        DCD        0
                        DCD        0
                        DCD        0
                        DCD        0
                        DCD        0
                        DCD        0
                        DCD        0
                        DCD        0
                        DCD        0
                        
                        ; External Interrupts
                        
                        DCD        UART_Handler
                        DCD        EXT_GPIO3_ACTIVEHIGH_DEBOUNCED_Handler
                        DCD        EXT_OPTICAL_IRQ_IN_Handler
                        DCD        ADC_Handler
                        DCD        0
                        DCD        0
                        DCD        RF_Handler
                        DCD        RFTIMER_Handler
                        DCD        RAWCHIPS_STARTVAL_Handler
                        DCD        RAWCHIPS_32_Handler
                        DCD        0
                        DCD        OPTICAL_SFD_Handler
                        DCD        EXT_GPIO8_ACTIVEHIGH_Handler
                        DCD        EXT_GPIO9_ACTIVELOW_Handler
                        DCD        EXT_GPIO10_ACTIVELOW
                        DCD        0 
              
                AREA |.text|, CODE, READONLY
;Interrupt Handlers
Reset_Handler       PROC
        GLOBAL      Reset_Handler
        ENTRY
        
        LDR         R1, =0xE000E100         ;Interrupt Set Enable Register
        LDR         R0, =0x0000             ;<- REMEMBER TO ENABLE THE INTERRUPTS!!
        STR         R0, [R1]
        
        ;IP wake up just to solve interrupts
        ; LDR r0, =0xE000ED10; System Control Register address
        ; LDR r1, [r0]
        ; MOVS r2, #0x6
        ; ORRS r1, r2; Set SLEEPONEXIT bit
        ; STR r1, [r0]
        
        IMPORT      __main
        LDR         R0, =__main               
        BX          R0                        ;Branch to __main
        ENDP

UART_Handler        PROC
        EXPORT      UART_Handler
        IMPORT      uart_rx_isr
        
        PUSH        {R0,LR}
        
        MOVS        R0, #1 ;         ;MASK all interrupts
        MSR         PRIMASK, R0 ;         
        
        BL          uart_rx_isr
        
                
        MOVS        R0, #0        ;ENABLE all interrupts
        MSR         PRIMASK, R0
        
        POP         {R0,PC}
        ENDP

ADC_Handler         PROC
        EXPORT      ADC_Handler
        IMPORT      adc_isr
        
        PUSH        {R0,LR}
        
        MOVS        R0, #1 ;         ;MASK all interrupts
        MSR         PRIMASK, R0 ; 
        ;STR        R0,[R1]    
        
        BL          adc_isr
        
        MOVS        R0, #0        ;ENABLE all interrupts
        MSR         PRIMASK, R0
        
        POP         {R0,PC}
        ENDP

RF_Handler    PROC
        EXPORT      RF_Handler
        IMPORT      radio_isr
        
        PUSH        {R0,LR}
        
        MOVS        R0, #1 ;         ;MASK all interrupts
        MSR         PRIMASK, R0 ; 
        ;STR        R0,[R1]    
        
        BL          radio_isr
        
        MOVS        R0, #0        ;ENABLE all interrupts
        MSR         PRIMASK, R0
        
        POP         {R0,PC}
        ENDP
        
RFTIMER_Handler     PROC
        EXPORT      RFTIMER_Handler
        IMPORT      sctimer_isr
        
        PUSH        {R0,LR}
        
        MOVS        R0, #1 ;         ;MASK all interrupts
        MSR         PRIMASK, R0 ; 
        ;STR        R0,[R1]    
        
        BL          sctimer_isr
        
        MOVS        R0, #0        ;ENABLE all interrupts
        MSR         PRIMASK, R0
        
        POP         {R0,PC}
        ENDP

; -----------------------------
; BEGIN EXTERNAL INTERRUPT ISRS
; -----------------------------
EXT_GPIO3_ACTIVEHIGH_DEBOUNCED_Handler    PROC
        EXPORT      EXT_GPIO3_ACTIVEHIGH_DEBOUNCED_Handler
        IMPORT      ext_gpio3_activehigh_debounced_isr
        
        PUSH        {R0,LR}
        
        MOVS        R0, #1 ;         ;MASK all interrupts
        MSR         PRIMASK, R0 ; 
        ;STR        R0,[R1]    
        
        BL          ext_gpio3_activehigh_debounced_isr
        
        MOVS        R0, #0        ;ENABLE all interrupts
        MSR         PRIMASK, R0
        
        POP         {R0,PC}
        ENDP

EXT_GPIO8_ACTIVEHIGH_Handler    PROC
        EXPORT      EXT_GPIO8_ACTIVEHIGH_Handler
        IMPORT      ext_gpio8_activehigh_isr
        
        PUSH        {R0,LR}
        
        MOVS        R0, #1 ;         ;MASK all interrupts
        MSR         PRIMASK, R0 ; 
        ;STR        R0,[R1]    
        
        BL          ext_gpio8_activehigh_isr
        
        MOVS        R0, #0        ;ENABLE all interrupts
        MSR         PRIMASK, R0
        
        POP         {R0,PC}
        ENDP

EXT_GPIO9_ACTIVELOW_Handler    PROC
        EXPORT      EXT_GPIO9_ACTIVELOW_Handler
        IMPORT      ext_gpio9_activelow_isr
        
        PUSH        {R0,LR}
        
        MOVS        R0, #1 ;         ;MASK all interrupts
        MSR         PRIMASK, R0 ; 
        ;STR        R0,[R1]    
        
        BL          ext_gpio9_activelow_isr
        
        MOVS        R0, #0        ;ENABLE all interrupts
        MSR         PRIMASK, R0
        
        POP         {R0,PC}
        ENDP

EXT_GPIO10_ACTIVELOW    PROC
        EXPORT      EXT_GPIO10_ACTIVELOW
        IMPORT      ext_gpio10_activelow_isr
        
        PUSH        {R0,LR}
        
        MOVS        R0, #1 ;         ;MASK all interrupts
        MSR         PRIMASK, R0 ; 
        ;STR        R0,[R1]    
        
        BL          ext_gpio10_activelow_isr
        
        MOVS        R0, #0        ;ENABLE all interrupts
        MSR         PRIMASK, R0
        
        POP         {R0,PC}
        ENDP

; ---------------------------
; END EXTERNAL INTERRUPT ISRS
; ---------------------------

RAWCHIPS_STARTVAL_Handler    PROC
        EXPORT      RAWCHIPS_STARTVAL_Handler
        IMPORT      rawchips_startval_isr
        
        PUSH        {R0,LR}
        
        MOVS        R0, #1 ;         ;MASK all interrupts
        MSR         PRIMASK, R0 ; 
        ;STR        R0,[R1]    
        
        BL          rawchips_startval_isr
        
        MOVS        R0, #0        ;ENABLE all interrupts
        MSR         PRIMASK, R0
        
        POP         {R0,PC}
        ENDP

RAWCHIPS_32_Handler    PROC
        EXPORT      RAWCHIPS_32_Handler
        IMPORT      rawchips_32_isr
        
        PUSH        {R0,LR}
        
        MOVS        R0, #1 ;         ;MASK all interrupts
        MSR         PRIMASK, R0 ; 
        ;STR        R0,[R1]    
        
        BL          rawchips_32_isr
        
        MOVS        R0, #0        ;ENABLE all interrupts
        MSR         PRIMASK, R0
        
        POP         {R0,PC}
        ENDP

EXT_OPTICAL_IRQ_IN_Handler      PROC
        EXPORT      EXT_OPTICAL_IRQ_IN_Handler
        IMPORT      optical_32_isr
        
        PUSH        {R0,LR}
        
        MOVS        R0, #1 ;         ;MASK all interrupts
        MSR         PRIMASK, R0 ; 
        ;STR        R0,[R1]    
        
        BL          optical_32_isr
        
        MOVS        R0, #0        ;ENABLE all interrupts
        MSR         PRIMASK, R0
        
        POP         {R0,PC}
        ENDP

OPTICAL_SFD_Handler    PROC
        EXPORT      OPTICAL_SFD_Handler
        IMPORT      optical_sfd_isr
        
        PUSH        {R0,LR}
        
        MOVS        R0, #1 ;         ;MASK all interrupts
        MSR         PRIMASK, R0 ; 
        ;STR        R0,[R1]    
        
        BL          optical_sfd_isr
        
        MOVS        R0, #0        ;ENABLE all interrupts
        MSR         PRIMASK, R0
        
        POP         {R0,PC}
        ENDP

        ALIGN 4

; User Initial Stack & Heap
                IF      :DEF:__MICROLIB
                EXPORT  __initial_sp
                EXPORT  __heap_base
                EXPORT  __heap_limit
                ELSE
                IMPORT  __use_two_region_memory
                EXPORT  __user_initial_stackheap
__user_initial_stackheap

                LDR     R0, =  Heap_Mem
                LDR     R1, =(Stack_Mem + Stack_Size)
                LDR     R2, = (Heap_Mem +  Heap_Size)
                LDR     R3, = Stack_Mem
                BX      LR

                ALIGN

                ENDIF

        END