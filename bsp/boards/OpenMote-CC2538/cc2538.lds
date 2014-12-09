/**
 * Author: Xavier Vilajosana (xvilajosana@eecs.berkeley.edu)
 *         Pere Tuset (peretuset@openmote.com)
 * Date:   July 2013
 * Description: CC2538-specific linker script configuration file.
 */
 
/**
 * Indicate to the linker the entry point.
 */
ENTRY(ResetISR)

/**
 * RAM is 16 KB retention and 16 KB no retention
 * NON-RETENTION RAM starts at 0x20000000 with length 0x00004000 
 * RETENTION RAM starts at  0x20004000 with length 0x00004000
 */
MEMORY
{
    FLASH (rx) : ORIGIN = 0x200000, LENGTH = 0x0007FFD4
    FLASH_CCA (RX) : ORIGIN = 0x0027FFD4, LENGTH = 12
    SRAM (RWX) : ORIGIN = 0x20004000, LENGTH = 0x00004000 
}

SECTIONS
{
    .text :
    {
        _text = .;
        KEEP(*(.vectors))
        *(.text*)
        *(.rodata*)
        _etext = .;
    } > FLASH=0

    .data :
    AT (ADDR (.text) + SIZEOF (.text))
    {
        _data = .;
        *(vtable)
        *(.data*)
        _edata = .;
    } > SRAM

    .ARM.exidx :
    {
        *(.ARM.exidx*)
    } > FLASH

    .bss :
    {
        _bss = .;
        *(.bss*)
        *(COMMON)
        _ebss = .;
    } > SRAM

    .flashcca :
    {
        KEEP(*(.flashcca))
    } > FLASH_CCA
}
