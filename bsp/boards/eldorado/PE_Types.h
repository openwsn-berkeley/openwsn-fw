/** ###################################################################
**     Generated types by CodeWarrior
**     Replaces stdint.h
** ###################################################################*/

#ifndef __PE_Types_H
#define __PE_Types_H

#ifdef __MISRA__
  #ifndef FALSE
    #define  FALSE  0u
  #endif
  #ifndef TRUE
    #define  TRUE   1u
  #endif
#else
  #ifndef FALSE
    #define  FALSE  0
  #endif
  #ifndef TRUE
    #define  TRUE   1
  #endif
#endif

/*Types definition*/
typedef unsigned char bool;
typedef unsigned char byte;
typedef unsigned int word;
typedef unsigned long dword;
typedef unsigned long dlong[2];
typedef void (*tIntFunc)(void);
typedef unsigned char TPE_ErrCode;
/*typedef void (*__far tFarPtr)(void);*/

/* Freescale types */
typedef unsigned char       VUINT8;
typedef signed char         VINT8;
typedef unsigned short int  VUINT16;
typedef signed short int    VINT16;
typedef unsigned long int   VUINT32;

/* Additional standard ANSI C types */
#ifndef int8_t
typedef signed char int8_t;
#endif
#ifndef int16_t
typedef signed int int16_t;
#endif
#ifndef int32_t
typedef signed long int int32_t;
#endif

#ifndef uint8_t
typedef unsigned char uint8_t;
#endif
#ifndef uint16_t
typedef unsigned int uint16_t;
#endif
#ifndef uint32_t
typedef unsigned long int uint32_t;
#endif
#ifndef TPE_Float
typedef float TPE_Float;
#endif
#ifndef char_t
typedef char char_t;
#endif

/*lint -save  -esym(960,19.12) -esym(961,19.13) Disable MISRA rule (19.12,19.13) checking. */
/**************************************************/
/* PE register access macros                      */
/**************************************************/
#define setRegBit(reg, bit)                                     (reg |= reg##_##bit##_##MASK)
#define clrRegBit(reg, bit)                                     (reg &= ~reg##_##bit##_##MASK)
#define getRegBit(reg, bit)                                     (reg & reg##_##bit##_##MASK)
#define setReg(reg, val)                                        (reg = (byte)(val))
#define getReg(reg)                                             (reg)
#define setRegBits(reg, mask)                                   (reg |= (byte)(mask))
#define getRegBits(reg, mask)                                   (reg & (byte)(mask))
#define clrRegBits(reg, mask)                                   (reg &= (byte)(~(mask)))
#define setRegBitGroup(reg, bits, val)                          (reg = (byte)((reg & ~reg##_##bits##_##MASK) | ((val) << reg##_##bits##_##BITNUM)))
#define getRegBitGroup(reg, bits)                               ((reg & reg##_##bits##_##MASK) >> reg##_##bits##_##BITNUM)
#define setRegMask(reg, maskAnd, maskOr)                        (reg = (byte)((getReg(reg) & ~(maskAnd)) | (maskOr)))
#define setRegBitVal(reg, bit, val)                             ((val) == 0 ? (reg &= ~reg##_##bit##_##MASK) : (reg |= reg##_##bit##_##MASK))
#define changeRegBits(reg, mask)                                (reg ^= (mask))
#define changeRegBit(reg, bit)                                  (reg ^= reg##_##bit##_##MASK)

/******************************************************************/
/* Uniform multiplatform peripheral access macros - 32 bit access */
/******************************************************************/
#define setReg32Bit(RegName, BitName)                            (RegName |= RegName##_##BitName##_##MASK)
#define clrReg32Bit(RegName, BitName)                            (RegName &= ~(dword)RegName##_##BitName##_##MASK)
#define invertReg32Bit(RegName, BitName)                         (RegName ^= RegName##_##BitName##_##MASK)
#define testReg32Bit(RegName, BitName)                           (RegName & RegName##_##BitName##_##MASK)

/* Whole peripheral register access macros */
#define setReg32(RegName, val)                                   (RegName = (dword)(val))
#define getReg32(RegName)                                        (RegName)

/* Bits peripheral register access macros */
#define testReg32Bits(RegName, GetMask)                          (RegName & (GetMask))
#define clrReg32Bits(RegName, ClrMask)                           (RegName &= (dword)(~(dword)(ClrMask)))
#define setReg32Bits(RegName, SetMask)                           (RegName |= (dword)(SetMask))
#define invertReg32Bits(RegName, InvMask)                        (RegName ^= (dword)(InvMask))
#define clrSetReg32Bits(RegName, ClrMask, SetMask)               (RegName = (RegName & (~(dword)(ClrMask))) | (dword)(SetMask))
#define seqClrSetReg32Bits(RegName, BitsMask, BitsVal)           ((RegName &= ~(~(dword)(BitsVal) & (dword)(BitsMask))),\
                                                                 (RegName |= (dword)(BitsVal) & (dword)(BitsMask)) )
#define seqSetClrReg32Bits(RegName, BitsMask, BitsVal)           ((RegName |= (dword)(BitsVal) & (dword)(BitsMask)),\
                                                                 (RegName &= ~(~(dword)(BitsVal) & (dword)(BitsMask))) )
#define seqResetSetReg32Bits(RegName, BitsMask, BitsVal)         ((RegName &= ~(dword)(BitsMask)),\
                                                                 (RegName |= (dword)(BitsVal) & (dword)(BitsMask)) )
#define clrReg32BitsByOne(RegName, ClrMask, BitsMask)            (RegName &= (dword)(ClrMask) & (dword)(BitsMask))

/* Bit group peripheral register access macros */
#define testReg32BitGroup(RegName, GroupName)                    (RegName & RegName##_##GroupName##_##MASK)
#define getReg32BitGroupVal(RegName, GroupName)                  ((RegName & RegName##_##GroupName##_##MASK) >> RegName##_##GroupName##_##BITNUM)
#define setReg32BitGroupVal(RegName, GroupName, GroupVal)        (RegName = (RegName & ~(dword)RegName##_##GroupName##_##MASK) | (((dword)(GroupVal)) << RegName##_##GroupName##_##BITNUM))
#define seqClrSetReg32BitGroupVal(RegName,GroupName,GroupVal)    ((RegName &= ~(~(((dword)(GroupVal)) << RegName##_##GroupName##_##BITNUM) & RegName##_##GroupName##_##MASK)),\
                                                                 (RegName |= (((dword)(GroupVal)) << RegName##_##GroupName##_##BITNUM) & RegName##_##GroupName##_##MASK) )
#define seqSetClrReg32BitGroupVal(RegName,GroupName,GroupVal)    ((RegName |= (((dword)(GroupVal)) << RegName##_##GroupName##_##BITNUM) & RegName##_##GroupName##_##MASK),\
                                                                 (RegName &= ~(~(((dword)(GroupVal)) << RegName##_##GroupName##_##BITNUM) & RegName##_##GroupName##_##MASK)) )
#define seqResetSetReg32BitGroupVal(RegName,GroupName,GroupVal)  ((RegName &= ~(dword)RegName##_##GroupName##_##MASK),\
                                                                 (RegName |= (((dword)(GroupVal)) << RegName##_##GroupName##_##BITNUM) & RegName##_##GroupName##_##MASK) )

/******************************************************************/
/* Uniform multiplatform peripheral access macros - 16 bit access */
/******************************************************************/
#define setReg16Bit(RegName, BitName)                            (RegName |= RegName##_##BitName##_##MASK)
#define clrReg16Bit(RegName, BitName)                            (RegName &= ~(word)RegName##_##BitName##_##MASK)
#define invertReg16Bit(RegName, BitName)                         (RegName ^= RegName##_##BitName##_##MASK)
#define testReg16Bit(RegName, BitName)                           (RegName & RegName##_##BitName##_##MASK)

/* Whole peripheral register access macros */
#define setReg16(RegName, val)                                   (RegName = (word)(val))
#define getReg16(RegName)                                        (RegName)

/* Bits peripheral register access macros */
#define testReg16Bits(RegName, GetMask)                          (RegName & (GetMask))
#define clrReg16Bits(RegName, ClrMask)                           (RegName &= (word)(~(word)(ClrMask)))
#define setReg16Bits(RegName, SetMask)                           (RegName |= (word)(SetMask))
#define invertReg16Bits(RegName, InvMask)                        (RegName ^= (word)(InvMask))
#define clrSetReg16Bits(RegName, ClrMask, SetMask)               (RegName = (RegName & (~(word)(ClrMask))) | (word)(SetMask))
#define seqClrSetReg16Bits(RegName, BitsMask, BitsVal)           (RegName &= ~(~(word)(BitsVal) & (word)(BitsMask)),\
                                                                 (RegName |= (word)(BitsVal) & (word)(BitsMask)) )
#define seqSetClrReg16Bits(RegName, BitsMask, BitsVal)           ((RegName |= (word)(BitsVal) & (word)(BitsMask)),\
                                                                 (RegName &= ~(~(word)(BitsVal) & (word)(BitsMask))) )
#define seqResetSetReg16Bits(RegName, BitsMask, BitsVal)         ((RegName &= ~(word)(BitsMask)),\
                                                                 (RegName |= (word)(BitsVal) & (word)(BitsMask)) )
#define clrReg16BitsByOne(RegName, ClrMask, BitsMask)            (RegName &= (word)(ClrMask) & (word)(BitsMask))

/* Bit group peripheral register access macros */
#define testReg16BitGroup(RegName, GroupName)                    (RegName & RegName##_##GroupName##_##MASK)
#define getReg16BitGroupVal(RegName, GroupName)                  ((RegName & RegName##_##GroupName##_##MASK) >> RegName##_##GroupName##_##BITNUM)
#define setReg16BitGroupVal(RegName, GroupName, GroupVal)        (RegName = (RegName & ~(word)RegName##_##GroupName##_##MASK) | (((word)(GroupVal)) << RegName##_##GroupName##_##BITNUM))
#define seqClrSetReg16BitGroupVal(RegName,GroupName,GroupVal)    ((RegName &= ~(~(((word)(GroupVal)) << RegName##_##GroupName##_##BITNUM) & RegName##_##GroupName##_##MASK)),\
                                                                 (RegName |= (((word)(GroupVal)) << RegName##_##GroupName##_##BITNUM) & RegName##_##GroupName##_##MASK ) )
#define seqSetClrReg16BitGroupVal(RegName,GroupName,GroupVal)    ((RegName |= (((word)(GroupVal)) << RegName##_##GroupName##_##BITNUM) & RegName##_##GroupName##_##MASK),\
                                                                 (RegName &= ~(~(((word)(GroupVal)) << RegName##_##GroupName##_##BITNUM) & RegName##_##GroupName##_##MASK)) )
#define seqResetSetReg16BitGroupVal(RegName,GroupName,GroupVal)  ((RegName &= ~(word)RegName##_##GroupName##_##MASK),\
                                                                 (RegName |= (((word)(GroupVal)) << RegName##_##GroupName##_##BITNUM) & RegName##_##GroupName##_##MASK) )

/*****************************************************************/
/* Uniform multiplatform peripheral access macros - 8 bit access */
/*****************************************************************/
#define setReg8Bit(RegName, BitName)                            (RegName |= RegName##_##BitName##_##MASK)
#define clrReg8Bit(RegName, BitName)                            (RegName &= (byte)~(byte)RegName##_##BitName##_##MASK)
#define invertReg8Bit(RegName, BitName)                         (RegName ^= RegName##_##BitName##_##MASK)
#define testReg8Bit(RegName, BitName)                           (RegName & RegName##_##BitName##_##MASK)

/* Whole peripheral register access macros */
#define setReg8(RegName, val)                                    (RegName = (byte)(val))
#define getReg8(RegName)                                         (RegName)

/* Bits peripheral register access macros */
#define testReg8Bits(RegName, GetMask)                           (RegName & (GetMask))
#define clrReg8Bits(RegName, ClrMask)                            (RegName &= (byte)(~(byte)(ClrMask)))
#define setReg8Bits(RegName, SetMask)                            (RegName |= (byte)(SetMask))
#define invertReg8Bits(RegName, InvMask)                         (RegName ^= (byte)(InvMask))
#define clrSetReg8Bits(RegName, ClrMask, SetMask)                (RegName = (RegName & ((byte)(~(byte)(ClrMask)))) | (byte)(SetMask))
#define seqClrSetReg8Bits(RegName, BitsMask, BitsVal)            ((RegName &= (byte)~((byte)~((byte)(BitsVal)) & ((byte)(BitsMask)))),\
                                                                 (RegName |= (byte)(BitsVal) & (byte)(BitsMask)) )
#define seqSetClrReg8Bits(RegName, BitsMask, BitsVal)            ((RegName |= (byte)(BitsVal) & (byte)(BitsMask)),\
                                                                 (RegName &= (byte)~((byte)~((byte)(BitsVal)) & (byte)(BitsMask))) )
#define seqResetSetReg8Bits(RegName, BitsMask, BitsVal)          ((RegName &= (byte)~((byte)(BitsMask))),\
                                                                 (RegName |= (byte)(BitsVal) & (byte)(BitsMask)) )
#define clrReg8BitsByOne(RegName, ClrMask, BitsMask)             (RegName &= (byte)(ClrMask) & (byte)(BitsMask))

/* Bit group peripheral register access macros */
#define testReg8BitGroup(RegName, GroupName)                     (RegName & RegName##_##GroupName##_##MASK)
#define getReg8BitGroupVal(RegName, GroupName)                   ((RegName & RegName##_##GroupName##_##MASK) >> RegName##_##GroupName##_##BITNUM)
#define setReg8BitGroupVal(RegName, GroupName, GroupVal)         (RegName = (RegName & (byte)~(byte)RegName##_##GroupName##_##MASK) | (byte)(((byte)(GroupVal)) << RegName##_##GroupName##_##BITNUM))
#define seqClrSetReg8BitGroupVal(RegName,GroupName,GroupVal)     ((RegName &= (byte)~((byte)~(byte)(((byte)(GroupVal)) << RegName##_##GroupName##_##BITNUM) & (byte)RegName##_##GroupName##_##MASK)),\
                                                                 (RegName |= (byte)(((byte)(GroupVal)) << RegName##_##GroupName##_##BITNUM) & (byte)RegName##_##GroupName##_##MASK) )
#define seqSetClrReg8BitGroupVal(RegName,GroupName,GroupVal)     ((RegName |= (byte)(((byte)(GroupVal)) << RegName##_##GroupName##_##BITNUM) & (byte)RegName##_##GroupName##_##MASK),\
                                                                 (RegName &= (byte)~((byte)~(byte)(((byte)(GroupVal)) << RegName##_##GroupName##_##BITNUM) & (byte)RegName##_##GroupName##_##MASK)) )
#define seqResetSetReg8BitGroupVal(RegName,GroupName,GroupVal)   ((RegName &= (byte)~(byte)RegName##_##GroupName##_##MASK),\
                                                                 (RegName |= (byte)(((byte)(GroupVal)) << RegName##_##GroupName##_##BITNUM) & RegName##_##GroupName##_##MASK) )
/*lint -restore  +esym(961,19.12) +esym(961,19.13) Enable MISRA rule (19.12,19.13) checking. */

#define __DI() \
/*lint -save  -e950 -esym(960,14.3) Disable MISRA rule (1.1,14.3) checking. */\
 { asm sei; }      /* Disable interrupts  */ \
/*lint -restore  +esym(961,14.3) Enable MISRA rule (1.1,14.3) checking. */
#define __EI() \
/*lint -save  -e950 -esym(960,14.3) Disable MISRA rule (1.1,14.3) checking. */\
 { asm cli; }      /* Enable interrupts */ \
/*lint -restore  +esym(961,14.3) Enable MISRA rule (1.1,14.3) checking. */
#define SaveStatusReg() \
/*lint -save  -e950 -esym(960,14.3) Disable MISRA rule (1.1,14.3) checking. */\
 { asm PSHA; asm TPA; asm SEI; asm STA CCR_reg; asm PULA; } /* This macro is used by Processor Expert. It saves CCR register and disable global interrupts. */ \
/*lint -restore  +esym(961,14.3) Enable MISRA rule (1.1,14.3) checking. */
#define RestoreStatusReg() \
/*lint -save  -e950 -esym(960,14.3) Disable MISRA rule (1.1,14.3) checking. */\
 { asm PSHA; asm LDA CCR_reg; asm TAP; asm PULA; } /* This macro is used by Processor Expert. It restores CCR register saved in SaveStatusReg(). */ \
/*lint -restore  +esym(961,14.3) Enable MISRA rule (1.1,14.3) checking. */
#define EnterCritical()     SaveStatusReg()
#define ExitCritical()      RestoreStatusReg()
#define PE_DEBUGHALT() \
/*lint -save  -e950 Disable MISRA rule (1.1) checking. */\
    { asm(BGND); }                     /* This macro forces entering to background debug mode, if enabled, and execute a sw breakpoint */ \
/*lint -restore Enable MISRA rule (1.1) checking. */
#define ISR(x) __interrupt void x(void)

typedef struct {                       /* Image */
  word width;                          /* Image width  */
  word height;                         /* Image height */
  const byte * pixmap;                 /* Image pixel bitmap */
  word size;                           /* Image size   */
  const char_t * name;                 /* Image name   */
} TIMAGE;
typedef TIMAGE* PIMAGE ;               /* Pointer to image */

/*lint -save  -esym(960,18.4) Disable MISRA rule (18.4) checking. */
/* 16-bit register (big endian) */
typedef union {
   word w;
   struct {
     byte high,low;
   } b;
} TWREG;
/*lint -restore  +esym(961,18.4) Enable MISRA rule (18.4) checking. */

#endif /* __PE_Types_H */
/*
** ###################################################################
**
**     This file was created by Processor Expert 5.3 [05.01]
**     for the Freescale HCS08 series of microcontrollers.
**
** ###################################################################
*/
