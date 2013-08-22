

/*
Authors: Xavier Vilajosana
         Pere Tuset 
*/

/*
 * cc2538 linker configuration file. This is not the actual file used at link
 * stage. Rather, it is used as input for the auto-generation of the actual
 * ld script, which is called cc2538.ld and will be in the project directory
 */
 
 /*
  * Indicate to the linker the entry point.
  */
ENTRY(ResetISR)

/*
 *   RAM is 16KB retention and 16kb no retention
 *   NOW we are using 32KB as retention which is not right.
 *   NON-RETENTION RAM starts at 0x20000000 with length 0x00004000 
 *   RETENTION RAM starts at  0x20004000 with length 0x00004000
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
    } > FLASH= 0

    .data :
    {
        _data = .;
        *(vtable)
        *(.data*)
        _edata = .;
    } > SRAM AT > FLASH

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
