/**
 * @brief Declaration of the "openserial" driver.
 *
 * @author Fabien Chraim <chraim@eecs.berkeley.edu>, March 2012.
 * @author Thomas Watteyne <thomas.watteyne@inria.fr>, August 2016.
 * @author Timothy Claeys <timothy.claeys@inria.fr>, February 2021.
*/

#ifndef OPENWSN_OPENSERIAL_H
#define OPENWSN_OPENSERIAL_H

#include "config.h"
#include "opendefs.h"

/**
\addtogroup drivers
\{
\addtogroup OpenSerial
\{
*/

//=========================== defines ==========================================

/**
 * @brief Number of bytes of the serial output buffer, in bytes.
*/
#define SERIAL_OUTPUT_BUFFER_SIZE       (0x400)
#define OUTPUT_BUFFER_MASK              (0x3FF)

/**
 * @brief Number of bytes of the serial input buffer, in bytes.
 *
 * @warning Do not pick a number greater than 255, since its filling level is encoded by a single byte in the code.
*/
#define SERIAL_INPUT_BUFFER_SIZE         (0xFF)

// frames sent mote->PC
#define SERFRAME_MOTE2PC_DATA                    ((uint8_t)'D')
#define SERFRAME_MOTE2PC_STATUS                  ((uint8_t)'S')
#define SERFRAME_MOTE2PC_VERBOSE                 ((uint8_t)'V')
#define SERFRAME_MOTE2PC_INFO                    ((uint8_t)'I')
#define SERFRAME_MOTE2PC_WARNING                 ((uint8_t)'W')
#define SERFRAME_MOTE2PC_SUCCESS                 ((uint8_t)'U')
#define SERFRAME_MOTE2PC_ERROR                   ((uint8_t)'E')
#define SERFRAME_MOTE2PC_CRITICAL                ((uint8_t)'C')
#define SERFRAME_MOTE2PC_SNIFFED_PACKET          ((uint8_t)'P')
#define SERFRAME_MOTE2PC_PRINTF                  ((uint8_t)'F')

// frames sent PC->mote
#define SERFRAME_PC2MOTE_SETROOT                 ((uint8_t)'R')
#define SERFRAME_PC2MOTE_RESET                   ((uint8_t)'Q')
#define SERFRAME_PC2MOTE_DATA                    ((uint8_t)'D')
#define SERFRAME_PC2MOTE_TRIGGERSERIALECHO       ((uint8_t)'S')

//=========================== macros =========================================

#ifndef OPENWSN_DEBUG_LEVEL
#define OPENWSN_DEBUG_LEVEL     4
#endif

#if (OPENWSN_DEBUG_LEVEL >= 6)
#define LOG_VERBOSE(component, message, p1, p2)   openserial_printLog(L_VERBOSE, (component), (message), (p1), (p2))
#else
#define LOG_VERBOSE(component, message, p1, p2)
#endif

#if (OPENWSN_DEBUG_LEVEL >= 5)
#define LOG_INFO(component, message, p1, p2)   openserial_printLog(L_INFO, (component), (message), (p1), (p2))
#else
#define LOG_INFO(component, message, p1, p2)
#endif

#if (OPENWSN_DEBUG_LEVEL >= 4)
#define LOG_WARNING(component, message, p1, p2)   openserial_printLog(L_WARNING, (component), (message), (p1), (p2))
#else
#define LOG_WARNING(component, message, p1, p2)
#endif

#if (OPENWSN_DEBUG_LEVEL >= 3)
#define LOG_SUCCESS(component, message, p1, p2)   openserial_printLog(L_SUCCESS, (component), (message), (p1), (p2))
#else
#define LOG_SUCCESS(component, message, p1, p2)
#endif

#if (OPENWSN_DEBUG_LEVEL >= 2)
#define LOG_ERROR(component, message, p1, p2)   openserial_printLog(L_ERROR, (component), (message), (p1), (p2))
#else
#define LOG_ERROR(component, message, p1, p2)
#endif

#if (OPENWSN_DEBUG_LEVEL >= 1)
#define LOG_CRITICAL(component, message, p1, p2)   openserial_printLog(L_CRITICAL, (component), (message), (p1), (p2))
#else
#define LOG_CRITICAL(component, message, p1, p2)
#endif
//=========================== typedef =========================================

typedef enum {
    L_CRITICAL = 1,
    L_ERROR = 2,
    L_SUCCESS = 3,
    L_WARNING = 4,
    L_INFO = 5,
    L_VERBOSE = 6
} level_t;

typedef open_addr_t *(*getAddr_cb_t)(uint8_t addr_type);

typedef void (*getAsn_cb_t)(uint8_t *array);

typedef void (*setRoot_cb_t)(void);

typedef void (*callBridge_cb_t)(void);

typedef bool (*statusPrint_cb_t)(void);

typedef struct statusCtx_t {
    uint8_t id;
    statusPrint_cb_t statusPrint_cb;
    struct statusCtx_t *next;
} statusCtx_t;

typedef struct {
    // admin
    uint8_t f_Inhibited;
    uint8_t cts_StateChanged;
    uint8_t statusPrint_currentId;
    uint8_t reset_timerId;
    uint8_t statusPrint_timerId;
    statusCtx_t *statusCtx;
    // callbacks
    getAddr_cb_t addrCb;
    getAsn_cb_t asnCb;
    setRoot_cb_t rootCb;
    callBridge_cb_t bridgeCb;
    // input
    uint8_t inputBuf[SERIAL_INPUT_BUFFER_SIZE];
    uint8_t inputBufFillLevel;
    uint8_t hdlcLastRxByte;
    bool hdlcBusyReceiving;
    uint16_t hdlcInputCrc;
    bool hdlcInputEscaping;
    // output
    uint8_t outputBuf[SERIAL_OUTPUT_BUFFER_SIZE];
    uint16_t outputBufIdxW;
    uint16_t outputBufIdxR;
    bool fBusyFlushing;
    uint16_t hdlcOutputCrc;
} openserial_vars_t;

//=========================== prototypes ======================================

// admin

/**
 * @brief Initialization function of the module OpenSerial
 */
void openserial_init(void);

/**
 *
 * @param[in] cb_method     Callback method to install
 */
void openserial_setAddrCb(getAddr_cb_t cb_method);

/**
 *
 * @param[in] cb_method     Callback method to install
 */
void openserial_setAsnCb(getAsn_cb_t cb_method);

/**
 *
 * @param[in] cb_method     Callback method to install
 */
void openserial_setRootCb(setRoot_cb_t cb_method);

/**
 *
 * @param[in] cb_method     Callback method to install
 */
void openserial_setBridgeCb(callBridge_cb_t cb_method);

/**
 * @brief Append a status context. Status information is printed on a periodic basis
 *
 * @param[in] ctx   Status context to append to list.
 */
void openserial_appendStatusCtx(statusCtx_t *ctx);

// transmitting

/**
 * @brief Prints a specific status element of the mote.
 *
 * @param[in] statusElement Status element to print
 * @param[in] buffer        Buffer containing the status information
 * @param[in] length        Length of @p buffer
 *
 * @return On success returns E_SUCCESS
 * @return On failure returns E_FAIL
 */
owerror_t openserial_printStatus(uint8_t statusElement, const uint8_t *buffer, size_t length);

/**
 * @brief Prints logging information
 *
 * @param[in] lvl       Log level
 * @param[in] caller    Calling component
 * @param[in] err       The error code / log description code
 * @param[in] arg1      First argument
 * @param[in] arg2      Second argument
 *
 * @return On success returns E_SUCCESS
 * @return On failure returns E_FAIL
 */
owerror_t openserial_printLog(level_t lvl, uint8_t caller, uint8_t err, errorparameter_t arg1, errorparameter_t arg2);

/**
 * @brief Prints a buffer
 *
 * @param[in] buffer    Buffer to print
 * @param[in] length    Length of @p buffer
 *
 * @return On success returns E_SUCCESS
 * @return On failure returns E_FAIL
 */
owerror_t openserial_printData(const uint8_t *buffer, size_t length);

owerror_t openserial_printSniffedPacket(const uint8_t *buffer, uint8_t length, uint8_t channel);

owerror_t openserial_printf(const char *buffer, ...);

// receiving
uint8_t openserial_getInputBufferFillLevel(void);

uint8_t openserial_getInputBuffer(uint8_t *bufferToWrite, uint8_t maxNumBytes);

// scheduling
void openserial_flush(void);

void openserial_inhibitStart(void);

void openserial_inhibitStop(void);

/**
\}
\}
*/

#endif
