/* 
 */
#ifndef _DECA_PORT_H_
#define _DECA_PORT_H_

#ifdef __cplusplus
extern "C" {
#endif

int readfromspi(uint16 headerLength, const uint8 *headerBuffer, uint32 readLength, uint8 *readBuffer);
int writetospi(uint16 headerLength, const uint8 *headerBuffer, uint32 bodyLength, const uint8 *bodyBuffer);

void deca_sleep( unsigned int time_ms);

decaIrqStatus_t decamutexon(void);
void decamutexoff(decaIrqStatus_t state);

void deca_spi_init(uint8_t speed);



#ifdef __cplusplus
}
#endif

#endif // _DECA_PORT_H_
