    .syntax    unified
    .arch    armv6-m

    .section .stack
    .align    4
#ifdef __STACK_SIZE
    .equ    Stack_Size, __STACK_SIZE
#else
    .equ    Stack_Size, 0x800
#endif
    .globl    __StackTop
    .globl    __StackLimit
__StackLimit:
    .space    Stack_Size
    .size    __StackLimit, . - __StackLimit
__StackTop:
    .size    __StackTop, . - __StackTop

    .section .heap
    .align    4
#ifdef __HEAP_SIZE
    .equ    Heap_Size, __HEAP_SIZE
#else
    .equ    Heap_Size, 0x400
#endif
    .globl    __HeapBase
    .globl    __HeapLimit
__HeapBase:
    .if    Heap_Size
    .space    Heap_Size
    .endif
    .size    __HeapBase, . - __HeapBase
__HeapLimit:
    .size    __HeapLimit, . - __HeapLimit

    .section .isr_vector
    .align    2
    .globl    __isr_vector
__isr_vector:
    .long    __StackTop            /* Top of Stack */
    .long    Reset_Handler         /* Reset Handler */
    .long    0                     /* Reserved */
    .long    0                     /* Reserved */
    .long    0                     /* Reserved */
    .long    0                     /* Reserved */
    .long    0                     /* Reserved */
    .long    0                     /* Reserved */
    .long    0                     /* Reserved */
    .long    0                     /* Reserved */
    .long    0                     /* Reserved */
    .long    0                     /* Reserved */
    .long    0                     /* Reserved */
    .long    0                     /* Reserved */
    .long    0                     /* Reserved */
    .long    0                     /* Reserved */

    /* External interrupts */
    .long    UART_Handler          /*  0: UART_                      */
    .long    0                     /*  1: Reserved                   */
    .long    0                     /*  2: Reserved                   */
    .long    0                     /*  3: Reserved                   */
    .long    0                     /*  4: Reserved                   */
    .long    0                     /*  5: Reserved                   */
    .long    RF_Handler            /*  6: RF                         */
    .long    RFTIMER_Handler       /*  7: RFTimer                    */
    .long    0                     /*  8: Reserved                   */
    .long    0                     /*  9: Reserved                   */
    .long    0                     /* 10: Reserved                   */
    .long    0                     /* 11: Reserved                   */
    .long    0                     /* 12: Reserved                   */
    .long    0                     /* 13: Reserved                   */
    .long    0                     /* 14: Reserved                   */
    .long    0                     /* 15: Reserved                   */

    .size    __isr_vector, . - __isr_vector

/* Reset Handler */
    .text
    .thumb
    .thumb_func
    .align    2
    .globl    Reset_Handler
    .type    Reset_Handler, %function
Reset_Handler:

    #Interrupt Set Enable Register
    ldr     r1, =0xe000e100
    #<- REMEMBER TO ENABLE THE INTERRUPTS!!
    ldr     r0, =0xc1
    str     r0, [r1]

    ldr     r0,=main
    blx     r0

    .size    Reset_Handler, . - Reset_Handler
    
/*    Macro to define default handlers. Default handler
 *    will be weak symbol and just dead loops. They can be
 *    overwritten by other handlers */
    .macro    def_default_handler    handler_name
    .align 1
    .thumb_func
    .weak    \handler_name
    .type    \handler_name, %function
\handler_name :
    b    .
    .size    \handler_name, . - \handler_name
    .endm
    
/* System Exception Handlers */
    
/* IRQ Handlers */
    def_default_handler    UART_Handler
    def_default_handler    RF_Handler
    def_default_handler    RFTIMER_Handler
    
    .end
