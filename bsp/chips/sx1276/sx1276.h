#include <math.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

/* ========================== prototypes =================================== */

//void sx1276_spiStrobe      (uint8_t strobe, uint8_t frequency_type);

void sx1276_spiWriteReg    (uint8_t reg, uint8_t regValueToWrite);
uint8_t sx1276_spiReadReg  (uint8_t reg);

//void sx1276_spiWriteFifo   (uint8_t* bufToWrite, uint16_t len, uint8_t frequency_type);
//void sx1276_spiReadRxFifo  (uint8_t* pBufRead, uint16_t* lenRead, uint8_t frequency_type, uint16_t maxBuffLength);

//uint8_t sx1276_status      (uint8_t frequency_type);

//void sx1276_read_isr       (uint8_t* rf09_isr, uint8_t frequency_type);
//void sx1276_readBurst(uint16_t reg, uint8_t* regValueRead, uint16_t size);
