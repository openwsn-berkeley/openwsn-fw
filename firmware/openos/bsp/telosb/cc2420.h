/**
\brief Register definitions for the Texas Instruments CC2420 radio chip.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, February 2012.
*/

#ifndef __CC2420_H
#define __CC2420_H

//=========================== define ==========================================

// strobes
#define CC2420_SNOP          0x00 // [S  ] No Operation
#define CC2420_SXOSCON       0x01 // [S  ] Turn on the crystal oscillator
#define CC2420_STXCAL        0x02 // [S  ] Enable and calibrate frequency synthesizer for TX
#define CC2420_SRXON         0x03 // [S  ] Enable RX
#define CC2420_STXON         0x04 // [S  ] Enable TX after calibration (if not already performed)
#define CC2420_STXONCCA      0x05 // [S  ] If CCA indicates a clear channel, Enable calibration, then TX
#define CC2420_SRFOFF        0x06 // [S  ] Disable RX/TX and frequency synthesizer
#define CC2420_SXOSCOFF      0x07 // [S  ] Turn off the crystal oscillator and RF
#define CC2420_SFLUSHRX      0x08 // [S  ] Flush the RX FIFO buffer and reset the demodulator
#define CC2420_SFLUSHTX      0x09 // [S  ] Flush the TX FIFO buffer
#define CC2420_SACK          0x0a // [S  ] Send acknowledge frame, with pending field cleared
#define CC2420_SACKPEND      0x0b // [S  ] Send acknowledge frame, with pending field set
#define CC2420_SRXDEC        0x0c // [S  ] Start RXFIFO in-line decryption / authentication
#define CC2420_STXENC        0x0d // [S  ] Start TXFIFO in-line encryption / authentication
#define CC2420_SAES          0x0e // [S  ] AES Stand alone encryption strobe

// registers
#define CC2420_MAIN          0x10 // [R/W] Main Control Register
#define CC2420_MDMCTRL0      0x11 // [R/W] Modem Control Register 0
#define CC2420_MDMCTRL1      0x12 // [R/W] Modem Control Register 1
#define CC2420_RSSI          0x13 // [R/W] RSSI and CCA Status and Control register
#define CC2420_SYNCWORD      0x14 // [R/W] Synchronisation word control register
#define CC2420_TXCTRL        0x15 // [R/W] Transmit Control Register
#define CC2420_RXCTRL0       0x16 // [R/W] Receive Control Register 0
#define CC2420_RXCTRL1       0x17 // [R/W] Receive Control Register 1
#define CC2420_FSCTRL        0x18 // [R/W] Frequency Synthesizer Control and Status Register
#define CC2420_SECCTRL0      0x19 // [R/W] Security Control Register 0
#define CC2420_SECCTRL1      0x1a // [R/W] Security Control Register 1
#define CC2420_BATTMON       0x1b // [R/W] Battery Monitor Control and Status Register
#define CC2420_IOCFG0        0x1c // [R/W] Input / Output Control Register 0
#define CC2420_IOCFG1        0x1d // [R/W] Input / Output Control Register 1
#define CC2420_MANFIDL       0x1e // [R/W] Manufacturer ID, Low 16 bits
#define CC2420_MANFIDH       0x1f // [R/W] Manufacturer ID, High 16 bits
#define CC2420_FSMTC         0x20 // [R/W] Finite State Machine Time Constants
#define CC2420_MANAND        0x21 // [R/W] Manual signal AND override register
#define CC2420_MANOR         0x22 // [R/W] Manual signal OR override register
#define CC2420_AGCCTRL       0x23 // [R/W] AGC Control Register
#define CC2420_AGCTST0       0x24 // [R/W] AGC Test Register 0
#define CC2420_AGCTST1       0x25 // [R/W] AGC Test Register 1
#define CC2420_AGCTST2       0x26 // [R/W] AGC Test Register 2
#define CC2420_FSTST0        0x27 // [R/W] Frequency Synthesizer Test Register 0
#define CC2420_FSTST1        0x28 // [R/W] Frequency Synthesizer Test Register 1
#define CC2420_FSTST2        0x29 // [R/W] Frequency Synthesizer Test Register 2
#define CC2420_FSTST3        0x2a // [R/W] Frequency Synthesizer Test Register 3
#define CC2420_RXBPFTST      0x2b // [R/W] Receiver Bandpass Filter Test Register
#define CC2420_FSMSTATE      0x2c // [R  ] Finite State Machine State Status Register
#define CC2420_ADCTST        0x2c // [R/W] ADC Test Register
#define CC2420_DACTST        0x2e // [R/W] DAC Test Register
#define CC2420_TOPTST        0x2f // [R/W] Top Level Test Register

// buffer
#define CC2420_TXFIFO        0x3e // [  W] Transmit FIFO Byte Register
#define CC2420_RXFIFO        0x3f // [R/W] Receiver FIFO Byte Register

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

#endif