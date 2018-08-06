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

                    AREA        RESET, DATA, READONLY
                    EXPORT      __Vectors
                    
__Vectors           DCD         __initial_sp
                    DCD         Reset_Handler
                    DCD         0
                    DCD         0
                    DCD         0
                    DCD         0
                    DCD         0
                    DCD         0
                    DCD         0
                    DCD         0
                    DCD         0
                    DCD         0
                    DCD         0
                    DCD         0
                    DCD         0
                    DCD         0
                    
                    ; External Interrupts
                    
                    DCD         UART_Handler
                    DCD         0
                    DCD         0
                    DCD         0
                    DCD         0
                    DCD         0
                    DCD         RF_Handler
                    DCD         RFTIMER_Handler
                    DCD         0
                    DCD         0
                    DCD         0
                    DCD         0
                    DCD         0
                    DCD         0
                    DCD         0
                    DCD         0
                    
                    AREA |.text|, CODE, READONLY
                    
;Interrupt Handlers
Reset_Handler   PROC
        GLOBAL Reset_Handler
        ENTRY
        
        LDR     R1, =0xE000E100         ;Interrupt Set Enable Register
        LDR     R0, =0xC1               ;<- REMEMBER TO ENABLE THE INTERRUPTS!!
        STR     R0, [R1]
        
        IMPORT  __main
        LDR     R0, =__main             
        BX      R0                      ;Branch to __main
                ENDP

UART_Handler    PROC
        EXPORT      UART_Handler
        IMPORT      uart_rx_isr
        
        PUSH        {R0,LR}
        
        MOVS        R0, #1 ;            ;MASK all interrupts
        MSR         PRIMASK, R0 ;       
        
        BL          uart_rx_isr

                
        MOVS        R0, #0              ;ENABLE all interrupts
        MSR         PRIMASK, R0
        
        POP         {R0,PC}
                ENDP
                    
RF_Handler      PROC
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
                    
RFTIMER_Handler PROC
        EXPORT      RFTIMER_Handler
        IMPORT      sctimer_isr
        
        PUSH        {R0,LR}
        
        MOVS        R0, #1         ;MASK all interrupts
        MSR         PRIMASK, R0 ; 
        ;STR        R0,[R1]
        
        BL          sctimer_isr
        
        MOVS        R0, #0          ;ENABLE all interrupts
        MSR         PRIMASK, R0
        
        POP         {R0,PC}
        
                ENDP
                    
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