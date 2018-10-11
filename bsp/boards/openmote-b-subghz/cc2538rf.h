/**
 * Author: Xavier Vilajosana (xvilajosana@eecs.berkeley.edu)
 *         Pere Tuset (peretuset@openmote.com)
 * Date:   July 2013
 * Description: CC2538-specific definition of the "radio" bsp module.
 */

#ifndef CC2538RF_H_
#define CC2538RF_H_

#include <headers/hw_rfcore_sfr.h>
#include <headers/hw_rfcore_xreg.h>

/*---------------------------------------------------------------------------
 * RF Config
 *---------------------------------------------------------------------------*/
/* Constants */
#define CC2538_RF_CCA_THRES_USER_GUIDE 0xF8
/** Tx Power register
 dbm - value
{  7, 0xFF },
{  5, 0xED },
{  3, 0xD5 },
{  1, 0xC5 },
{  0, 0xB6 },
{ -1, 0xB0 },
{ -3, 0xA1 },
{ -5, 0x91 },
{ -7, 0x88 },
{ -9, 0x72 },
{-11, 0x62 },
{-13, 0x58 },
{-15, 0x42 },
{-24, 0x00 },
*/
#define CC2538_RF_TX_POWER_RECOMMENDED 0xFF // use 7dbm maxmium txpower
#define CC2538_RF_CHANNEL_MIN            11 // poipoi -- in fact is sending on 0x17 check that.
#define CC2538_RF_CHANNEL_MAX            26
#define CC2538_RF_CHANNEL_SPACING         5
#define CC2538_RF_MAX_PACKET_LEN        127
#define CC2538_RF_MIN_PACKET_LEN          4
#define CC2538_RF_CCA_CLEAR               1
#define CC2538_RF_CCA_BUSY                0
/*---------------------------------------------------------------------------*/
#ifdef CC2538_RF_CONF_TX_POWER
#define CC2538_RF_TX_POWER CC2538_RF_CONF_TX_POWER
#else
#define CC2538_RF_TX_POWER CC2538_RF_TX_POWER_RECOMMENDED
#endif /* CC2538_RF_CONF_TX_POWER */


#ifdef CC2538_RF_CONF_CHANNEL
#define CC2538_RF_CHANNEL CC2538_RF_CONF_CHANNEL
#else
#define CC2538_RF_CHANNEL CC2538_RF_CHANNEL_MIN
#endif /* CC2538_RF_CONF_CHANNEL */

#ifdef CC2538_RF_CONF_AUTOACK
#define CC2538_RF_AUTOACK CC2538_RF_CONF_AUTOACK
#else
#define CC2538_RF_AUTOACK 1
#endif /* CC2538_RF_CONF_AUTOACK */
/*---------------------------------------------------------------------------
 * Command Strobe Processor
 *---------------------------------------------------------------------------*/
/* OPCODES */
#define CC2538_RF_CSP_OP_ISRXON                0xE3
#define CC2538_RF_CSP_OP_ISTXON                0xE9
#define CC2538_RF_CSP_OP_ISTXONCCA             0xEA
#define CC2538_RF_CSP_OP_ISRFOFF               0xEF
#define CC2538_RF_CSP_OP_ISFLUSHRX             0xED
#define CC2538_RF_CSP_OP_ISFLUSHTX             0xEE

/**
 * \brief Send an RX ON command strobe to the CSP
 */
#define CC2538_RF_CSP_ISRXON()    \
  do { HWREG(RFCORE_SFR_RFST) = CC2538_RF_CSP_OP_ISRXON; } while(0)

/**
 * \brief Send a TX ON command strobe to the CSP
 */
#define CC2538_RF_CSP_ISTXON()    \
  do { HWREG(RFCORE_SFR_RFST) = CC2538_RF_CSP_OP_ISTXON; } while(0)

/**
 * \brief Send a RF OFF command strobe to the CSP
 */
#define CC2538_RF_CSP_ISRFOFF()   \
  do { HWREG(RFCORE_SFR_RFST) = CC2538_RF_CSP_OP_ISRFOFF; } while(0)

/**
 * \brief Flush the RX FIFO
 */
#define CC2538_RF_CSP_ISFLUSHRX()  do { \
  HWREG(RFCORE_SFR_RFST) = CC2538_RF_CSP_OP_ISFLUSHRX; \
  HWREG(RFCORE_SFR_RFST) = CC2538_RF_CSP_OP_ISFLUSHRX; \
} while(0)

/**
 * \brief Flush the TX FIFO
 */
#define CC2538_RF_CSP_ISFLUSHTX()  do { \
  HWREG(RFCORE_SFR_RFST) = CC2538_RF_CSP_OP_ISFLUSHTX; \
  HWREG(RFCORE_SFR_RFST) = CC2538_RF_CSP_OP_ISFLUSHTX; \
} while(0)
/*---------------------------------------------------------------------------*/


#endif /* CC2538RF_H_ */
