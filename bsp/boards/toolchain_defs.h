#ifndef __TOOLCHAIN_DEFS_H
#define __TOOLCHAIN_DEFS_H

/**
\brief Definitions which depends on the toolchain used.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, June 2014.
\author Diogo Guerra    <dy090.guerra@gmail.com>, August 2015.
*/

#include <stdint.h>
//===== types

#ifndef bool
#define bool uint8_t
#endif

//===== ISR and pragma

#if defined(__GNUC__)  &&  defined(__MSP430__)
    /* This is the MSPGCC compiler */
   #if __MSP430X__ & __MSP430_CPUX_TARGET_C20__ /* support for F5 familly processors */
      #define __MSPGCC_MAYBE_C16__ __attribute__((__c16__))
   #else /* -mc20 */
    #define __MSPGCC_MAYBE_C16__
      #if defined(__GNUC__) && (__GNUC__==4)  && (__GNUC_MINOR__<=5) && defined(__MSP430__)
         // mspgcc <4.5.x
         #include <signal.h>
         #define              ISR(v) interrupt (v ## _VECTOR) v ## _ISR(void)
      #endif
   #endif /* -mc20 */
   #define        ISR(v)   void __attribute__((__interrupt__ (a##_VECTOR))) __MSPGCC_MAYBE_C16__ (v##_ISR)(void)
#else /* -mc20 */
   // other
   #define              __PRAGMA__(x) _Pragma(#x)
   #define              ISR(v) __PRAGMA__(vector=v ##_VECTOR) __interrupt void v ##_ISR(void)
#endif

//===== inline

#ifdef _MSC_VER
   // visual studio
   #define port_INLINE  __inline
#else
   // other
   #define port_INLINE  inline
#endif

//===== packing

#ifdef _MSC_VER
   // visual studio
   #define BEGIN_PACK __pragma(pack(push,1));
   #define END_PACK   __pragma(pack(pop));
#else
   // other
   #define BEGIN_PACK   _Pragma("pack(1)");
   #define END_PACK     _Pragma("pack()");
#endif

//Pragma for newmspgcc
//    // other
//    #define BEGIN_PACK     __attribute__ ( (__packed__) )                                      /*_Pragma("pack(1)");*/
//    #define END_PACK     ;         /*_Pragma("pack()"); */

#endif
