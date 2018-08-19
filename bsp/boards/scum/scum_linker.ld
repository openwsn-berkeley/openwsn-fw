
ENTRY(Reset_Handler)

MEMORY
{
    ROM (rx)  : ORIGIN = 0x00000000, LENGTH = 64K
    RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 64K
}

stack_size = 2048;
heap_size = 1024;

_stack_start = ORIGIN(RAM)+LENGTH(RAM);
_stack_end = _stack_start - stack_size;

SECTIONS
    {
        /* The startup code goes first into ROM */
        .isr_vector :
        {
            . = ALIGN(4);
            KEEP(*(.isr_vector)) /* Startup code */
            . = ALIGN(4);
        } >ROM

        /* The program code and other data goes into ROM */
        .text :
        {
            . = ALIGN(4);
            *(.text)           /* .text sections (code) */
            *(.text*)          /* .text* sections (code) */
            *(.rodata)         /* .rodata sections (constants, strings, etc.) */
            *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
            *(.glue_7)         /* glue arm to thumb code */
            *(.glue_7t)        /* glue thumb to arm code */
            . = ALIGN(4);
            _etext = .;        /* define a global symbols at end of code */
        } >ROM

        .ARM.extab   : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >ROM
        .ARM : {
            __exidx_start = .;
            *(.ARM.exidx*)
            __exidx_end = .;
        } >ROM

        /* used by the startup to initialize data */
        _sidata = .;

        /* Initialized data sections goes into RAM, load LMA copy after code */
        .data : AT ( _sidata )
        {
            . = ALIGN(4);
            _sdata = .;        /* create a global symbol at data start */
            *(.data)           /* .data sections */
            *(.data*)          /* .data* sections */

            . = ALIGN(4);
            _edata = .;        /* define a global symbol at data end */
        } >RAM

        /* Uninitialized data section */
        . = ALIGN(4);
        .bss :
        {
            /*  Used by the startup in order to initialize the .bss secion */
            _sbss = .;         /* define a global symbol at bss start */
            __bss_start__ = _sbss;
            *(.bss)
            *(.bss*)
            *(COMMON)

            . = ALIGN(4);
            _ebss = .;         /* define a global symbol at bss end */
            __bss_end__ = _ebss;
        } >RAM

        . = ALIGN(4);
        .heap :
        {
            _heap_start = .;
            . = . + heap_size;
                    _heap_end = .;
            end = _heap_start;
            _end = end;
        } > RAM

        .ARM.attributes 0 : { *(.ARM.attributes) }
    }