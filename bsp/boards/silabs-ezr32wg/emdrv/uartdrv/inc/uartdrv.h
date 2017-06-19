/***************************************************************************//**
 * @file uartdrv.h
 * @brief UARTDRV API definition.
 * @version 4.2.1
 *******************************************************************************
 * @section License
 * <b>(C) Copyright 2014 Silicon Labs, http://www.silabs.com</b>
 *******************************************************************************
 *
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/

#ifndef __SILICON_LABS_UARTDRV_H__
#define __SILICON_LABS_UARTDRV_H__

#include "em_device.h"
#include "em_usart.h"
#include "em_cmu.h"
#include "ecode.h"
#include "uartdrv_config.h"
#include "dmadrv.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * @addtogroup EM_Drivers
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup UARTDRV
 * @brief UARTDRV Universal asynchronous receiver/transmitter driver, see
 *        @ref uartdrv_doc page for detailed
 *        documentation.
 * @{
 ******************************************************************************/

#define ECODE_EMDRV_UARTDRV_OK                (ECODE_OK)                              ///< Success return value.
#define ECODE_EMDRV_UARTDRV_WAITING           (ECODE_EMDRV_UARTDRV_BASE | 0x00000001) ///< Operation is waiting in queue.
#define ECODE_EMDRV_UARTDRV_ILLEGAL_HANDLE    (ECODE_EMDRV_UARTDRV_BASE | 0x00000002) ///< Illegal UART handle.
#define ECODE_EMDRV_UARTDRV_PARAM_ERROR       (ECODE_EMDRV_UARTDRV_BASE | 0x00000003) ///< Illegal input parameter.
#define ECODE_EMDRV_UARTDRV_BUSY              (ECODE_EMDRV_UARTDRV_BASE | 0x00000004) ///< The UART port is busy.
#define ECODE_EMDRV_UARTDRV_ILLEGAL_OPERATION (ECODE_EMDRV_UARTDRV_BASE | 0x00000005) ///< Illegal operation on UART port.
#define ECODE_EMDRV_UARTDRV_IDLE              (ECODE_EMDRV_UARTDRV_BASE | 0x00000008) ///< No UART transfer in progress.
#define ECODE_EMDRV_UARTDRV_ABORTED           (ECODE_EMDRV_UARTDRV_BASE | 0x00000009) ///< UART transfer has been aborted.
#define ECODE_EMDRV_UARTDRV_QUEUE_FULL        (ECODE_EMDRV_UARTDRV_BASE | 0x0000000A) ///< UART operation queue is full.
#define ECODE_EMDRV_UARTDRV_QUEUE_EMPTY       (ECODE_EMDRV_UARTDRV_BASE | 0x0000000B) ///< UART operation queue is empty.
#define ECODE_EMDRV_UARTDRV_PARITY_ERROR      (ECODE_EMDRV_UARTDRV_BASE | 0x0000000C) ///< UART parity error frame. Data is ignored.
#define ECODE_EMDRV_UARTDRV_FRAME_ERROR       (ECODE_EMDRV_UARTDRV_BASE | 0x0000000D) ///< UART frame error. Data is ignored.
#define ECODE_EMDRV_UARTDRV_DMA_ALLOC_ERROR   (ECODE_EMDRV_UARTDRV_BASE | 0x0000000E) ///< Unable to allocated DMA channels.

/// @cond DO_NOT_INCLUDE_WITH_DOXYGEN
#if defined(UART_PRESENT) && defined(USART_PRESENT)
#define UART_NUM_PORTS  (UART_COUNT + USART_COUNT)
#elif defined(UART_PRESENT)
#define UART_NUM_PORTS  (UART_COUNT)
#else
#define UART_NUM_PORTS  (USART_COUNT)
#endif
/// @endcond

typedef uint32_t UARTDRV_Count_t;     ///< UART transfer count
typedef uint32_t UARTDRV_Status_t;    ///< UART status return type

/// Flow Control method
typedef enum UARTDRV_FlowControlType
{
  uartdrvFlowControlNone  = 0,       ///< None
  uartdrvFlowControlSw    = 1,       ///< Software XON/XOFF
  uartdrvFlowControlHw    = 2        ///< nRTS/nCTS hardware handshake
} UARTDRV_FlowControlType_t;

/// Flow Control state
typedef enum UARTDRV_FlowControlState
{
  uartdrvFlowControlOn = 0,         ///< XON or nRTS/nCTS low
  uartdrvFlowControlOff = 1,        ///< XOFF or nRTS/nCTS high
  uartdrvFlowControlAuto = 2        ///< This driver controls the state
} UARTDRV_FlowControlState_t;

/// Transfer abort type
typedef enum UARTDRV_AbortType
{
  uartdrvAbortTransmit = 1,          ///< Abort current and queued transmit operations
  uartdrvAbortReceive = 2,           ///< Abort current and queued receive operations
  uartdrvAbortAll = 3                ///< Abort all current and queued operations
} UARTDRV_AbortType_t;

struct UARTDRV_HandleData;

/***************************************************************************//**
 * @brief
 *  UARTDRV transfer completion callback function.
 *
 * @details
 *  The callback function is called when a transfer has completed. An
 *  application should check the transferStatus and itemsTransferred values.
 *
 * @param[in] handle
 *   The UARTDRV device handle used to start the transfer.
 *
 * @param[in] transferStatus
 *   Number of bytes actually transferred.
 *
 * @param[in] itemsTransferred
 *   Number of bytes transferred.
 ******************************************************************************/
typedef void (*UARTDRV_Callback_t)(struct UARTDRV_HandleData *handle,
                                   Ecode_t transferStatus,
                                   uint8_t *data,
                                   UARTDRV_Count_t transferCount);

/// UART transfer buffer
typedef struct
{
  uint8_t *data;                         ///< Transfer data buffer
  UARTDRV_Count_t transferCount;         ///< Transfer item count
  UARTDRV_Count_t itemsRemaining;        ///< Transfer items remaining
  UARTDRV_Callback_t callback;           ///< Completion callback
  Ecode_t transferStatus;                ///< Completion status of transfer operation
} UARTDRV_Buffer_t;

/// Transfer operation FIFO queue typedef
typedef struct
{
  uint16_t head;                         ///< Index of next byte to send.
  uint16_t tail;                         ///< Index of where to enqueue next message.
  uint16_t used;                         ///< Number of bytes queued.
  const uint16_t size;                   ///< Size of FIFO.
  UARTDRV_Buffer_t fifo[];               ///< FIFO of queue data.
} UARTDRV_Buffer_FifoQueue_t;

/// Macros to define fifo and buffer queues, can't use a typedef becuase the size
/// of the fifo array in the queues can change.
#define DEFINE_BUF_QUEUE(qSize, qName)  \
typedef struct {                        \
  uint16_t head;                        \
  uint16_t tail;                        \
  uint16_t used;                        \
  const uint16_t size;                  \
  UARTDRV_Buffer_t fifo[qSize];         \
} _##qName;                             \
static volatile _##qName qName =        \
{                                       \
  .head = 0,                            \
  .tail = 0,                            \
  .used = 0,                            \
  .size = qSize,                        \
}


/// UART driver instance initialization structure.
/// This data structure contains a number of UARTDRV configuration options
/// required for driver instance initialization.
/// This struct is passed to @ref UARTDRV_Init() when initializing a UARTDRV
/// instance.
typedef struct
{
  USART_TypeDef              *port;             ///< The peripheral used for UART
  uint32_t                   baudRate;          ///< UART baud rate
#if defined( _USART_ROUTELOC0_MASK )
  uint8_t                    portLocationTx;    ///< Location number for UART Tx pin.
  uint8_t                    portLocationRx;    ///< Location number for UART Rx pin.
#else
  uint8_t                    portLocation;      ///< Location number for UART pins
#endif
  USART_Stopbits_TypeDef     stopBits;          ///< Number of stop bits
  USART_Parity_TypeDef       parity;            ///< Parity configuration
  USART_OVS_TypeDef          oversampling;      ///< Oversampling mode
#if defined(USART_CTRL_MVDIS)
  bool                       mvdis;             ///< Majority Vote Disable for 16x, 8x and 6x oversampling modes
#endif
  UARTDRV_FlowControlType_t  fcType;            ///< Flow control mode
  GPIO_Port_TypeDef          ctsPort;           ///< CTS pin port number
  uint8_t                    ctsPin;            ///< CTS pin number
  GPIO_Port_TypeDef          rtsPort;           ///< RTS pin port number
  uint8_t                    rtsPin;            ///< RTS pin number
  UARTDRV_Buffer_FifoQueue_t *rxQueue;          ///< Receive operation queue
  UARTDRV_Buffer_FifoQueue_t *txQueue;          ///< Transmit operation queue
} UARTDRV_Init_t;

/// UART driver instance handle data structure.
/// The handle is allocated by the application using UARTDRV. There may be
/// several concurrent driver instances in an application. The application must
/// not modify the contents of this handle.
typedef struct UARTDRV_HandleData
{
  /// @cond DO_NOT_INCLUDE_WITH_DOXYGEN
  UARTDRV_Init_t             initData;          // Driver instance initialization data
  unsigned int               txDmaCh;           // DMA ch assigned to Tx
  unsigned int               rxDmaCh;           // DMA ch assigned to Rx
  DMADRV_PeripheralSignal_t  txDmaSignal;       // DMA Tx trigger source signal
  DMADRV_PeripheralSignal_t  rxDmaSignal;       // DMA Rx trigger source signal
  UARTDRV_FlowControlState_t fcSelfState;       // Current flow control state of self
  UARTDRV_FlowControlState_t fcSelfCfg;         // Flow control override configuration of self
  UARTDRV_FlowControlState_t fcPeerState;       // Current flow control state of peer
  bool                       IgnoreRestrain;    // Transmit does not respect uartdrvFlowControlOff
  GPIO_Port_TypeDef          rxPort;            // RX pin port number
  uint8_t                    rxPin;             // RX pin number
  GPIO_Port_TypeDef          txPort;            // TX pin port number
  uint8_t                    txPin;             // RTS pin number
  CMU_Clock_TypeDef          uartClock;         // Clock source select
  UARTDRV_Buffer_FifoQueue_t *rxQueue;          // Receive operation queue
  UARTDRV_Buffer_FifoQueue_t *txQueue;          // Transmit operation queue
  volatile bool              rxDmaActive;       // Receive DMA is currently active
  volatile bool              txDmaActive;       // Transmit DMA is currently active
  /// @endcond
} UARTDRV_HandleData_t;

/// Handle pointer
typedef UARTDRV_HandleData_t * UARTDRV_Handle_t;

Ecode_t UARTDRV_Init(UARTDRV_Handle_t handle, UARTDRV_Init_t *initData);

Ecode_t UARTDRV_DeInit(UARTDRV_Handle_t handle);

UARTDRV_Status_t UARTDRV_GetReceiveStatus(UARTDRV_Handle_t handle,
                                          uint8_t **buffer,
                                          UARTDRV_Count_t *bytesReceived,
                                          UARTDRV_Count_t *bytesRemaining);

UARTDRV_Status_t UARTDRV_GetTransmitStatus(UARTDRV_Handle_t handle,
                                           uint8_t **buffer,
                                           UARTDRV_Count_t *bytesSent,
                                           UARTDRV_Count_t *bytesRemaining);

uint8_t UARTDRV_GetReceiveDepth(UARTDRV_Handle_t handle);

uint8_t UARTDRV_GetTransmitDepth(UARTDRV_Handle_t handle);

Ecode_t UARTDRV_Transmit(UARTDRV_Handle_t handle,
                         uint8_t *data,
                         UARTDRV_Count_t count,
                         UARTDRV_Callback_t callback);

Ecode_t UARTDRV_Receive(UARTDRV_Handle_t handle,
                        uint8_t *data,
                        UARTDRV_Count_t count,
                        UARTDRV_Callback_t callback);

Ecode_t UARTDRV_TransmitB(UARTDRV_Handle_t handle,
                          uint8_t *data,
                          UARTDRV_Count_t count);

Ecode_t UARTDRV_ReceiveB(UARTDRV_Handle_t handle,
                         uint8_t *data,
                         UARTDRV_Count_t count);

Ecode_t UARTDRV_ForceTransmit(UARTDRV_Handle_t handle,
                              uint8_t *data,
                              UARTDRV_Count_t count);

UARTDRV_Count_t UARTDRV_ForceReceive(UARTDRV_Handle_t handle,
                                      uint8_t *data,
                                      UARTDRV_Count_t maxLength);

Ecode_t UARTDRV_Abort(UARTDRV_Handle_t handle, UARTDRV_AbortType_t type);

UARTDRV_FlowControlState_t UARTDRV_FlowControlGetSelfStatus(UARTDRV_Handle_t handle);

UARTDRV_FlowControlState_t UARTDRV_FlowControlGetPeerStatus(UARTDRV_Handle_t handle);

Ecode_t UARTDRV_FlowControlSet(UARTDRV_Handle_t handle, UARTDRV_FlowControlState_t state);

Ecode_t UARTDRV_FlowControlIgnoreRestrain(UARTDRV_Handle_t handle);

/** @} (end addtogroup UARTDRV) */
/** @} (end addtogroup EM_Drivers) */

#ifdef __cplusplus
}
#endif
#endif // __SILICON_LABS_UARTDRV_H__
