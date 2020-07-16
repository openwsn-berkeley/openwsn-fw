#include <math.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "spi.h"

// ============== SX1276 definitions =================
#define XTAL_FREQ                                   32000000
#define FREQ_STEP                                   61.03515625

//======== Radio Operating Modes Functions ===========

void SX1276SetSleep( void );
void SX1276SetStby( void );
void SX1276SetFstx( void );
void SX1276SetTx( void );
void SX1276SetFsrx(void);
void SX1276SetRxContinuous( void );
void SX1276SetRxSingle( void );
void SX1276SetCad( void );

//======== Radio TX/RX configuration ===========
void SX1276SetTxConfig();
void SX1276SetRxConfig();

//======== Radio CHANNEL/FREQUENCY =============
void SX1276SetChannel( uint32_t freq );

//======== Radio FIFO buffer Write/Read ========
void SX1276WriteFifo(uint8_t size);
void SX1276WriteFifoBuffer(uint8_t buffer);
uint8_t sx1276ReadFifoBuffer(void);

//======== SX1276 Send/Receive ========
void sx1276Send(void);
void sx1276Receive(void);

//======== SX1276 callback functions =============
//void exercice(void);
/*!
 * \brief DIO 0 IRQ callback
 */

uint8_t SX1276ReadTxDone(void);
uint8_t SX1276ReadRxDone(void);
uint8_t SX1276ReadCrcError(void);
