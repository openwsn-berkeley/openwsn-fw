/**
 * Author: Xavier Vilajosana (xvilajosana@eecs.berkeley.edu)
 *         Pere Tuset (peretuset@openmote.com)
 * Date:   July 2013
 * Description: CC2538-specific linker script configuration file.
 */

/**
 * Indicate the architecture and format
 */
OUTPUT_ARCH(arm)
OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")

/**
 * Indicate the entry point to the linker
 */
ENTRY(reset_handler)

/**
 * Linker script for Texas Instruments CC2538SF53 (32K RAM, 512K Flash)
 * FLASH stores the program code
 * FLASH_CCA stores the backdoor bootloader configuration
 * SRAM1 is 16K NO-RETENTION and SRAM2 is 16K RETENTION
 * Currently we are only using SRAM2 with RETENTION
 */
MEMORY
{
    FLASH     (RX)  : ORIGIN = 0x00200000, LENGTH = 512K
    FLASH_CCA (RX)  : ORIGIN = 0x0027FFD4, LENGTH = 12
    SRAM1     (RWX) : ORIGIN = 0x20000000, LENGTH = 16K
    SRAM2     (RWX) : ORIGIN = 0x20004000, LENGTH = 16K
}

SECTIONS
{
    .text :
    {
        _text_start = .;
        . = ALIGN(4);
        
         /* Interrupt vector in Flash */
        KEEP(*(.vectors))
        
        /* Code and read-only data */
        *(.text*)
        *(.rodata*)
        
        _text_end = .;
    } > FLASH = 0
    
    /* Hods the CCA section of the CC2538 */
    .flashcca :
    {
        KEEP(*(.flashcca))
    } > FLASH_CCA

    /* Holds the stack at the start of SRAM */
    .stack (NOLOAD) : {
        . = ALIGN(4);
        KEEP(*(.stack))
    } > SRAM2

    /* Holds variables stored in FLASH and copied to SRAM upon initialization */
    .data :
    {
        _data_start = .;
        
        /* Interrupt vector in SRAM */
        *(vtable)
        
        /* Initialized data */
        *(.data*)
        
        _data_end = .;
    } > SRAM2 AT > FLASH

    /* Holds unitialized variables stored in RAM */
    .bss :
    {
        _bss_start = .;
        
        /* Uninitialized data */ 
        *(.bss*)
        *(COMMON)
        
        _bss_end = .;
    } > SRAM2
    
    /* Contains information to unwind the stack for debugging purposes */
    PROVIDE_HIDDEN (__exidx_start = .);
    .ARM.exidx :
    {
        . = ALIGN(4);
        *(.ARM.exidx*)
    } > FLASH
    PROVIDE_HIDDEN (__exidx_end = .);
}
