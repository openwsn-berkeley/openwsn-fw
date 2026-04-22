/*********************************************************************
*                    SEGGER Microcontroller GmbH                     *
*                        The Embedded Experts                        *
**********************************************************************
*                                                                    *
*            (c) 2014 - 2023 SEGGER Microcontroller GmbH             *
*                                                                    *
*       www.segger.com     Support: support@segger.com               *
*                                                                    *
**********************************************************************
*                                                                    *
* All rights reserved.                                               *
*                                                                    *
* Redistribution and use in source and binary forms, with or         *
* without modification, are permitted provided that the following    *
* condition is met:                                                  *
*                                                                    *
* - Redistributions of source code must retain the above copyright   *
*   notice, this condition and the following disclaimer.             *
*                                                                    *
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND             *
* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,        *
* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF           *
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE           *
* DISCLAIMED. IN NO EVENT SHALL SEGGER Microcontroller BE LIABLE FOR *
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR           *
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT  *
* OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;    *
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF      *
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT          *
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE  *
* USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH   *
* DAMAGE.                                                            *
*                                                                    *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------

File      : SEGGER_THUMB_Startup.s
Purpose   : Generic runtime init startup code for ARM CPUs running 
            in THUMB mode.
            Designed to work with the SEGGER linker to produce 
            smallest possible executables.
            
            This file does not normally require any customization.

Additional information:
  Preprocessor Definitions
    FULL_LIBRARY
      If defined then 
        - argc, argv are set up by calling SEGGER_SEMIHOST_GetArgs().
        - the exit symbol is defined and executes on return from main.
        - the exit symbol calls destructors, atexit functions and then
          calls SEGGER_SEMIHOST_Exit().
    
      If not defined then
        - argc and argv are not valid (main is assumed to not take parameters)
        - the exit symbol is defined, executes on return from main and
          halts in a loop.
*/

        .syntax unified  

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/

#ifndef   APP_ENTRY_POINT
  #define APP_ENTRY_POINT main
#endif

#ifndef   ARGSSPACE
  #define ARGSSPACE 128
#endif

/*********************************************************************
*
*       Macros
*
**********************************************************************
*/
//
// Declare a label as function symbol (without switching sections)
//
.macro MARK_FUNC Name
        .global \Name
        .thumb_func
        .code 16
\Name:
.endm
//
// Declare a regular function.
// Functions from the startup are placed in the init section.
//
.macro START_FUNC Name
        .section .init.\Name, "ax"
        .global \Name
        .balign 2
        .thumb_func
        .code 16
\Name:
.endm

//
// Declare a weak function
//
.macro WEAK_FUNC Name
        .section .init.\Name, "ax", %progbits
        .weak \Name
        .balign 2
        .thumb_func
        .code 16
\Name:
.endm

//
// Mark the end of a function and calculate its size
//
.macro END_FUNC name
        .size \name,.-\name
.endm

/*********************************************************************
*
*       Externals
*
**********************************************************************
*/
        .extern APP_ENTRY_POINT     // typically main

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/
/*********************************************************************
*
*       _start
*
*  Function description
*    Entry point for the startup code. 
*    Usually called by the reset handler.
*    Performs all initialisation, based on the entries in the 
*    linker-generated init table, then calls main().
*    It is device independent, so there should not be any need for an 
*    end-user to modify it.
*
*  Additional information
*    At this point, the stack pointer should already have been 
*    initialized 
*      - by hardware (such as on Cortex-M),
*      - by the device-specific reset handler,
*      - or by the debugger (such as for RAM Code).
*/
#undef L
#define L(label) .L_start_##label

START_FUNC _start
        //
        // Call linker init functions which in turn performs the following:
        // * Perform segment init
        // * Perform heap init (if used)
        // * Call constructors of global Objects (if any exist)
        //
        ldr     R4, =__SEGGER_init_table__      // Set table pointer to start of initialization table
L(RunInit): 
        ldr     R0, [R4]                        // Get next initialization function from table
        adds    R4, R4, #4                      // Increment table pointer to point to function arguments
        blx     R0                              // Call initialization function
        b       L(RunInit)
        //
MARK_FUNC __SEGGER_init_done
MARK_FUNC __startup_complete
        //
        // Time to call main(), the application entry point.
        //
#ifndef FULL_LIBRARY
        //
        // In a real embedded application ("Free-standing environment"), 
        // main() does not get any arguments,
        // which means it is not necessary to init R0 and R1.
        //
        bl      APP_ENTRY_POINT                 // Call to application entry point (usually main())

END_FUNC _start
        //
        // end of _start
        // Fall-through to exit if main ever returns.
        //
MARK_FUNC exit
        //
        // In a free-standing environment, if returned from application:
        // Loop forever.
        //
        b       .
        .size exit,.-exit
#else
        //
        // In a hosted environment, 
        // we need to load R0 and R1 with argc and argv, in order to handle 
        // the command line arguments.
        // This is required for some programs running under control of a 
        // debugger, such as automated tests.
        //
        movs    R0, #ARGSSPACE
        ldr     R1, =__SEGGER_init_arg_data
        bl      SEGGER_SEMIHOST_GetArgs
        ldr     R1, =__SEGGER_init_arg_data
        bl      APP_ENTRY_POINT                 // Call to application entry point (usually main())
        bl      exit                            // Call exit function
        b       .                               // If we unexpectedly return from exit, hang.
END_FUNC _start
#endif
        // 
#ifdef FULL_LIBRARY
/*********************************************************************
*
*       exit
*
*  Function description
*    Exit of the system.
*    Called on return from application entry point or explicit call 
*    to exit.
*
*  Additional information
*    In a hosted environment exit gracefully, by
*    saving the return value,
*    calling destructurs of global objects, 
*    calling registered atexit functions, 
*    and notifying the host/debugger.
*/
#undef L
#define L(label) .L_exit_##label

WEAK_FUNC exit
        mov     R5, R0                  // Save the exit parameter/return result
        //
        // Call destructors
        //
        ldr     R0, =__dtors_start__    // Pointer to destructor list
        ldr     R1, =__dtors_end__
L(Loop):
        cmp     R0, R1
        beq     L(End)                  // Reached end of destructor list? => Done
        ldr     R2, [R0]                // Load current destructor address into R2
        adds    R0, R0, #4              // Increment pointer
        push    {R0-R1}                 // Save R0 and R1
        blx     R2                      // Call destructor
        pop     {R0-R1}                 // Restore R0 and R1
        b       L(Loop)
L(End):
        //
        // Call atexit functions
        //
        bl      __SEGGER_RTL_execute_at_exit_fns
        //
        // Call debug_exit with return result/exit parameter
        //
        mov     R0, R5
        bl      SEGGER_SEMIHOST_Exit
        //
        // If execution is not terminated, loop forever
        //
L(ExitLoop):
        b       L(ExitLoop) // Loop forever.
END_FUNC exit
#endif

#ifdef FULL_LIBRARY
        .bss
        .balign 4
__SEGGER_init_arg_data:
        .space ARGSSPACE
        .size __SEGGER_init_arg_data, .-__SEGGER_init_arg_data
        .type __SEGGER_init_arg_data, %object
#endif

/*************************** End of file ****************************/
