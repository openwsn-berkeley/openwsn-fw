
 GROUP(
 libcr_semihost.a
 libcr_c.a
 libcr_eabihelpers.a
 )

GROUP(libgcc.a libc.a)

MEMORY
{
     FLASH (rx) : ORIGIN = 0x0 LENGTH = 0x80000
     SRAM (rwx) : ORIGIN = 0x10000000, LENGTH = 0x8000
	 AHBRAM0   : ORIGIN = 0x2007c000, LENGTH = 0x4000
	 AHBRAM1   : ORIGIN = 0x20080000, LENGTH = 0x4000
}

_vRamTop = ORIGIN( SRAM ) + LENGTH( SRAM );

/* Variables used by FreeRTOS-MPU. */
_Privileged_Functions_Region_Size = 16K;
_Privileged_Data_Region_Size = 256;

__FLASH_segment_start__ = ORIGIN( FLASH ); 
__FLASH_segment_end__ = __FLASH_segment_start__ + LENGTH( FLASH );

__privileged_functions_start__ = ORIGIN( FLASH );
__privileged_functions_end__ = __privileged_functions_start__ + _Privileged_Functions_Region_Size;

__SRAM_segment_start__ = ORIGIN( SRAM );
__SRAM_segment_end__ = __SRAM_segment_start__ + LENGTH( SRAM );

__privileged_data_start__ = ORIGIN( SRAM );
__privileged_data_end__ = ORIGIN( SRAM ) + _Privileged_Data_Region_Size;	

ENTRY(ResetISR)

SECTIONS
{
	/* Privileged section at the start of the flash - vectors must be first
	whatever. */
	privileged_functions :
	{
		KEEP(*(.isr_vector))
		. = 0x100;
		KEEP(*(.openAL_registry))
		*(privileged_functions)
	} > FLASH
	
	.text :
	{
		/* Non privileged code kept out of the first 16K or flash. */
		. = __privileged_functions_start__ + _Privileged_Functions_Region_Size;
	
		*(.text*)
		_API_start_of_text = .;
		KEEP(*(.openAL*))
		KEEP(*(.openAL_pubdata*))
		_API_end_of_text = .;
		*(.rodata*)
	} > FLASH


	/* for exception handling/unwind - some Newlib functions (in common with C++ and STDC++) use this. */
	.ARM.extab : 
	{
		*(.ARM.extab* .gnu.linkonce.armextab.*)
	} > FLASH

	__exidx_start = .;
	.ARM.exidx :
	{
		*(.ARM.exidx* .gnu.linkonce.armexidx.*)
	} > FLASH
	__exidx_end = .;

	_etext = .;
	
	/* zero initialized data */		
	privileged_data :
	{
		_bss = .;
		*(privileged_data)
		/* Non kernel data is kept out of the first 256 bytes of SRAM. */
	} > SRAM	

	. = ORIGIN( SRAM ) + _Privileged_Data_Region_Size;

	.bss :
	{
		*(.bss*)
		*(COMMON)
		_ebss = .;
	} > SRAM
	
	.data : AT (__exidx_end)
	{
		_data = .;	
		*(vtable)
		*(.data*)
		_edata = .;
	} > SRAM


	/* Where we put the heap with cr_clib */
	.cr_heap :
	{
		end = .;
		_pvHeapStart = .;
	} > SRAM

/*
	Note: (ref: M0000066)
	Moving the stack down by 16 is to work around a GDB bug.
	This space can be reclaimed for Production Builds.
*/	
	_vStackTop = _vRamTop - 16;
	
	.ETHRAM :
	{
	} > AHBRAM0
	
	.USBRAM :
	{
	} > AHBRAM1
}
