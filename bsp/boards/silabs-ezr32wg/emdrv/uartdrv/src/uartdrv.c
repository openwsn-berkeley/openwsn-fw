/***************************************************************************//**
 * @file uartdrv.c
 * @brief UARTDRV API implementation.
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
#include <string.h>

#include "em_device.h"
#include "em_emu.h"
#include "em_gpio.h"
#include "em_int.h"
#include "em_usart.h"
#include "uartdrv.h"
#if defined(EMDRV_UARTDRV_HW_FLOW_CONTROL_ENABLE)
#include "gpiointerrupt.h"
#endif

/// @cond DO_NOT_INCLUDE_WITH_DOXYGEN

#if (EMDRV_UARTDRV_HW_FLOW_CONTROL_ENABLE)
static bool uartdrvHandleIsInitialized = false;
static UARTDRV_Handle_t uartdrvHandle[EMDRV_UARTDRV_MAX_DRIVER_INSTANCES];
#endif

static bool ReceiveDmaComplete(  unsigned int channel,
                                 unsigned int sequenceNo,
                                 void *userParam );
static bool TransmitDmaComplete( unsigned int channel,
                                 unsigned int sequenceNo,
                                 void *userParam );

/***************************************************************************//**
 * @brief Get UARTDRV_Handle_t from GPIO pin number (HW FC CTS pin interrupt)
 ******************************************************************************/
#if (EMDRV_UARTDRV_HW_FLOW_CONTROL_ENABLE)
static UARTDRV_Handle_t HwFcCtsIrqGetDrvHandle(uint32_t gpioPinNo)
{
  uint32_t i;

  for(i = 0; i < EMDRV_UARTDRV_MAX_DRIVER_INSTANCES; i++)
  {
    if (uartdrvHandle[i]->initData.ctsPin == gpioPinNo)
    {
      return uartdrvHandle[i];
    }
  }
  return NULL;
}


/***************************************************************************//**
 * @brief Get CTS pin state
 ******************************************************************************/
static UARTDRV_FlowControlState_t HwFcGetClearToSendPin(UARTDRV_Handle_t handle)
{
  return (UARTDRV_FlowControlState_t)GPIO_PinInGet(handle->initData.ctsPort, handle->initData.ctsPin);
}


/***************************************************************************//**
 * @brief Manage CTS pin change
 ******************************************************************************/
static void HwFcManageClearToSend(uint8_t gpioPinNo)
{
  UARTDRV_Handle_t handle = HwFcCtsIrqGetDrvHandle(gpioPinNo);

  if (handle->initData.fcType == uartdrvFlowControlHw)
  {
    // If not auto mode, just assign the CTS pin state to the self state
    // If auto mode, also control UART TX enable
    handle->fcSelfState = HwFcGetClearToSendPin(handle);
    if (handle->fcSelfCfg == uartdrvFlowControlAuto)
    {
     if ((HwFcGetClearToSendPin(handle) == uartdrvFlowControlOn) || handle->IgnoreRestrain)
      {
        handle->IgnoreRestrain = false;
        handle->initData.port->CMD = USART_CMD_TXEN;
      }
      else
      {
        handle->initData.port->CMD = USART_CMD_TXDIS;
      }
    }
  }
}


static Ecode_t FcApplyState(UARTDRV_Handle_t handle)
{
  uint8_t FcSwCode;

  if (handle->initData.fcType == uartdrvFlowControlHw)
  {
    if (handle->fcSelfCfg == uartdrvFlowControlOn)
    {
      // Assert nRTS (application control)
      GPIO_PinOutClear(handle->initData.rtsPort, handle->initData.rtsPin);
    }
    else if (handle->fcSelfCfg == uartdrvFlowControlOff)
    {
      // Deassert nRTS (application control)
      GPIO_PinOutSet(handle->initData.rtsPort, handle->initData.rtsPin);
    }
    else // Auto mode
    {
      if (handle->fcSelfState == uartdrvFlowControlOn)
      {
        // Assert nRTS
        GPIO_PinOutClear(handle->initData.rtsPort, handle->initData.rtsPin);
      }
      else // Off
      {
        // Deassert nRTS
        GPIO_PinOutSet(handle->initData.rtsPort, handle->initData.rtsPin);
      }
    }
  }
  else if (handle->initData.fcType == uartdrvFlowControlSw)
  {
    if (handle->fcSelfState == uartdrvFlowControlOn)
    {
      FcSwCode = UARTDRV_FC_SW_XON;
      UARTDRV_ForceTransmit(handle, &FcSwCode, 1);
    }
    else
    {
      FcSwCode = UARTDRV_FC_SW_XOFF;
      UARTDRV_ForceTransmit(handle, &FcSwCode, 1);
    }
  }
  return ECODE_EMDRV_UARTDRV_OK;
}
#endif /* EMDRV_UARTDRV_HW_FLOW_CONTROL_ENABLE */


/***************************************************************************//**
 * @brief Enqueue UART transfer buffer
 ******************************************************************************/
static Ecode_t EnqueueBuffer(UARTDRV_Buffer_FifoQueue_t *queue,
                             UARTDRV_Buffer_t *inputBuffer,
                             UARTDRV_Buffer_t **queueBuffer)
{
  INT_Disable();
  if (queue->used >= queue->size)
  {
    *queueBuffer = NULL;
    INT_Enable();
    return ECODE_EMDRV_UARTDRV_QUEUE_FULL;
  }
  memcpy((void *)&queue->fifo[queue->head], (const void *)inputBuffer, sizeof(UARTDRV_Buffer_t));
  *queueBuffer = &queue->fifo[queue->head];
  queue->head = (queue->head + 1) % queue->size;
  queue->used++;
  INT_Enable();

  return ECODE_EMDRV_UARTDRV_OK;
}


/***************************************************************************//**
 * @brief Dequeue UART transfer buffer
 ******************************************************************************/
static Ecode_t DequeueBuffer(UARTDRV_Buffer_FifoQueue_t *queue,
                             UARTDRV_Buffer_t **buffer)
{
  INT_Disable();
  if (queue->used == 0)
  {
    *buffer = NULL;
    INT_Enable();
    return ECODE_EMDRV_UARTDRV_QUEUE_EMPTY;
  }
  *buffer = &queue->fifo[queue->tail];
  queue->tail = (queue->tail + 1) % queue->size;
  queue->used--;
  INT_Enable();

  return ECODE_EMDRV_UARTDRV_OK;
}


/***************************************************************************//**
 * @brief Get tail UART transfer buffer
 ******************************************************************************/
static Ecode_t GetTailBuffer(UARTDRV_Buffer_FifoQueue_t *queue,
                             UARTDRV_Buffer_t **buffer)
{
  INT_Disable();
  if (queue->used == 0)
  {
    *buffer = NULL;
    INT_Enable();
    return ECODE_EMDRV_UARTDRV_QUEUE_EMPTY;
  }
  *buffer = &queue->fifo[queue->tail];

  INT_Enable();
  return ECODE_EMDRV_UARTDRV_OK;
}


/***************************************************************************//**
 * @brief Enable UART transmitter
 ******************************************************************************/
static void EnableTransmitter(UARTDRV_Handle_t handle)
{
  handle->initData.port->CMD = USART_CMD_TXEN;
  while (!(handle->initData.port->STATUS & USART_STATUS_TXENS));
  #if defined( USART_ROUTEPEN_TXPEN )
  handle->initData.port->ROUTEPEN |= USART_ROUTEPEN_TXPEN;
  #else
  handle->initData.port->ROUTE |= USART_ROUTE_TXPEN;
  #endif
}


/***************************************************************************//**
 * @brief Disable UART transmitter
 ******************************************************************************/
static void DisableTransmitter(UARTDRV_Handle_t handle)
{
  #if defined( USART_ROUTEPEN_TXPEN )
  handle->initData.port->ROUTEPEN &= ~USART_ROUTEPEN_TXPEN;
  #else
  handle->initData.port->ROUTE &= ~USART_ROUTE_TXPEN;
  #endif
  handle->initData.port->CMD = USART_CMD_TXDIS;
}


/***************************************************************************//**
 * @brief Enable UART receiver
 ******************************************************************************/
static void EnableReceiver(UARTDRV_Handle_t handle)
{
  handle->initData.port->CMD = USART_CMD_RXEN;
  while (!(handle->initData.port->STATUS & USART_STATUS_RXENS));
  #if defined( USART_ROUTEPEN_RXPEN )
  handle->initData.port->ROUTEPEN |= USART_ROUTEPEN_RXPEN;
  #else
  handle->initData.port->ROUTE |= USART_ROUTE_RXPEN;
  #endif
}


/***************************************************************************//**
 * @brief Disable UART receiver
 ******************************************************************************/
static void DisableReceiver(UARTDRV_Handle_t handle)
{
  #if defined( USART_ROUTEPEN_RXPEN )
  handle->initData.port->ROUTEPEN &= ~USART_ROUTEPEN_RXPEN;
  #else
  handle->initData.port->ROUTE &= ~USART_ROUTE_RXPEN;
  #endif
  handle->initData.port->CMD = USART_CMD_RXDIS;
}


/***************************************************************************//**
 * @brief Start a UART receive DMA.
 ******************************************************************************/
static void StartReceiveDma(UARTDRV_Handle_t handle,
                            UARTDRV_Buffer_t *buffer)
{
  void *rxPort;

  handle->rxDmaActive = true;
  rxPort = (void *)&(handle->initData.port->RXDATA);

  DMADRV_PeripheralMemory( handle->rxDmaCh,
                           handle->rxDmaSignal,
                           buffer->data,
                           rxPort,
                           true,
                           buffer->transferCount,
                           dmadrvDataSize1,
                           ReceiveDmaComplete,
                           handle );
}


/***************************************************************************//**
 * @brief Start a UART transmit DMA.
 ******************************************************************************/
static void StartTransmitDma(UARTDRV_Handle_t handle,
                             UARTDRV_Buffer_t *buffer)
{
  void *txPort;

  handle->txDmaActive = true;
  txPort = (void *)&(handle->initData.port->TXDATA);

  DMADRV_MemoryPeripheral( handle->txDmaCh,
                           handle->txDmaSignal,
                           txPort,
                           buffer->data,
                           true,
                           buffer->transferCount,
                           dmadrvDataSize1,
                           TransmitDmaComplete,
                           handle );
}


/***************************************************************************//**
 * @brief DMA transfer completion callback. Called by DMA interrupt handler.
 ******************************************************************************/
static bool ReceiveDmaComplete( unsigned int channel,
                                unsigned int sequenceNo,
                                void *userParam )
{
  UARTDRV_Handle_t handle;
  UARTDRV_Buffer_t *buffer;
  (void)channel;
  (void)sequenceNo;

  handle = (UARTDRV_Handle_t)userParam;
  GetTailBuffer(handle->rxQueue, &buffer);

  if (handle->initData.port->IF & USART_IF_FERR)
  {
    buffer->transferStatus = ECODE_EMDRV_UARTDRV_FRAME_ERROR;
    buffer->itemsRemaining = buffer->transferCount; // nothing received
    handle->initData.port->IFC = USART_IFC_FERR;
  }
  else if (handle->initData.port->IF & USART_IF_PERR)
  {
    buffer->transferStatus = ECODE_EMDRV_UARTDRV_PARITY_ERROR;
    buffer->itemsRemaining = buffer->transferCount; // nothing received
    handle->initData.port->IFC = USART_IFC_PERR;
  }
  else
  {
    buffer->transferStatus = ECODE_EMDRV_UARTDRV_OK;
    buffer->itemsRemaining = 0;
  }

  INT_Disable();

  if (buffer->callback != NULL)
  {
    buffer->callback(handle, buffer->transferStatus, buffer->data, buffer->transferCount - buffer->itemsRemaining);
  }
  // Dequeue the current tail RX operation, check if more in queue
  DequeueBuffer(handle->rxQueue, &buffer);

  if (handle->rxQueue->used > 0)
  {
    GetTailBuffer(handle->rxQueue, &buffer);
    StartReceiveDma(handle, buffer);
  }
  else
  {
    #if (EMDRV_UARTDRV_HW_FLOW_CONTROL_ENABLE)
    handle->fcSelfState = uartdrvFlowControlOff;
    FcApplyState(handle);
    #endif
    handle->rxDmaActive = false;
    DisableReceiver(handle);
  }
  INT_Enable();
  return true;
}


/***************************************************************************//**
 * @brief DMA transfer completion callback. Called by DMA interrupt handler.
 ******************************************************************************/
static bool TransmitDmaComplete( unsigned int channel,
                                 unsigned int sequenceNo,
                                 void *userParam )
{
  UARTDRV_Handle_t handle;
  UARTDRV_Buffer_t *buffer;
  (void)channel;
  (void)sequenceNo;

  handle = (UARTDRV_Handle_t)userParam;
  GetTailBuffer(handle->txQueue, &buffer);

  buffer->transferStatus = ECODE_EMDRV_UARTDRV_OK;
  buffer->itemsRemaining = 0;

  INT_Disable();

  if (buffer->callback != NULL)
  {
    buffer->callback(handle, ECODE_EMDRV_UARTDRV_OK, buffer->data, buffer->transferCount);
  }
  // Dequeue the current tail TX operation, check if more in queue
  DequeueBuffer(handle->txQueue, &buffer);

  if (handle->txQueue->used > 0)
  {
    GetTailBuffer(handle->txQueue, &buffer);
    StartTransmitDma(handle, buffer);
  }
  else
  {
    handle->txDmaActive = false;
  }
  INT_Enable();
  return true;
}


/***************************************************************************//**
 * @brief Parameter checking function for blocking transfer API functions.
 ******************************************************************************/
static Ecode_t CheckParams(UARTDRV_Handle_t handle, void *data, uint32_t count)
{
  if (handle == NULL)
  {
    return ECODE_EMDRV_UARTDRV_ILLEGAL_HANDLE;
  }
  if ((data == NULL) || (count == 0) || (count > DMADRV_MAX_XFER_COUNT )) {
    return ECODE_EMDRV_UARTDRV_PARAM_ERROR;
  }
  return ECODE_EMDRV_UARTDRV_OK;
}


/***************************************************************************//**
 * @brief Configure/deconfigure UART GPIO pins.
 ******************************************************************************/
static Ecode_t ConfigGPIO(UARTDRV_Handle_t handle, bool enable)
{
#if defined( _USART_ROUTELOC0_MASK )
  UARTDRV_Init_t *initData;
#else
  uint32_t location;
#endif

#if defined( _USART_ROUTELOC0_MASK )
  initData = &handle->initData;

  if ( 0 ) {
  #if defined(USARTRF0)
  } else if (handle->initData.port == USARTRF0) {
    handle->txPort = (GPIO_Port_TypeDef)AF_USARTRF0_TX_PORT( initData->portLocationTx );
    handle->rxPort = (GPIO_Port_TypeDef)AF_USARTRF0_RX_PORT( initData->portLocationRx );
    handle->txPin  = AF_USARTRF0_TX_PIN( initData->portLocationTx );
    handle->rxPin  = AF_USARTRF0_RX_PIN( initData->portLocationRx );
  #endif
  #if defined(USARTRF1)
  } else if (handle->initData.port == USARTRF1) {
    handle->txPort = (GPIO_Port_TypeDef)AF_USARTRF1_TX_PORT( initData->portLocationTx );
    handle->rxPort = (GPIO_Port_TypeDef)AF_USARTRF1_RX_PORT( initData->portLocationRx );
    handle->txPin  = AF_USARTRF1_TX_PIN( initData->portLocationTx );
    handle->rxPin  = AF_USARTRF1_RX_PIN( initData->portLocationRx );
  #endif
  #if defined(USART0)
  } else if (handle->initData.port == USART0) {
    handle->txPort = (GPIO_Port_TypeDef)AF_USART0_TX_PORT( initData->portLocationTx );
    handle->rxPort = (GPIO_Port_TypeDef)AF_USART0_RX_PORT( initData->portLocationRx );
    handle->txPin  = AF_USART0_TX_PIN( initData->portLocationTx );
    handle->rxPin  = AF_USART0_RX_PIN( initData->portLocationRx );
  #endif
  #if defined(USART1)
  } else if (handle->initData.port == USART1) {
    handle->txPort  = (GPIO_Port_TypeDef)AF_USART1_TX_PORT( initData->portLocationTx );
    handle->rxPort  = (GPIO_Port_TypeDef)AF_USART1_RX_PORT( initData->portLocationRx );
    handle->txPin   = AF_USART1_TX_PIN( initData->portLocationTx );
    handle->rxPin   = AF_USART1_RX_PIN( initData->portLocationRx );
  #endif
  #if defined(USART2)
  } else if (handle->initData.port == USART2) {
    handle->txPort  = (GPIO_Port_TypeDef)AF_USART2_TX_PORT( initData->portLocationTx );
    handle->rxPort  = (GPIO_Port_TypeDef)AF_USART2_RX_PORT( initData->portLocationRx );
    handle->txPin   = AF_USART2_TX_PIN( initData->portLocationTx );
    handle->rxPin   = AF_USART2_RX_PIN( initData->portLocationRx );
  #endif
  #if defined(UART0)
  } else if (handle->initData.port == UART0) {
    handle->txPort  = (GPIO_Port_TypeDef)AF_UART0_TX_PORT( initData->portLocationTx );
    handle->rxPort  = (GPIO_Port_TypeDef)AF_UART0_RX_PORT( initData->portLocationRx );
    handle->txPin   = AF_UART0_TX_PIN( initData->portLocationTx );
    handle->rxPin   = AF_UART0_RX_PIN( initData->portLocationRx );
  #endif
  #if defined(UART1)
  } else if (handle->initData.port == UART1) {
    handle->txPort  = (GPIO_Port_TypeDef)AF_UART1_TX_PORT( initData->portLocationTx );
    handle->rxPort  = (GPIO_Port_TypeDef)AF_UART1_RX_PORT( initData->portLocationRx );
    handle->txPin   = AF_UART1_TX_PIN( initData->portLocationTx );
    handle->rxPin   = AF_UART1_RX_PIN( initData->portLocationRx );
  #endif
  } else {
    return ECODE_EMDRV_UARTDRV_PARAM_ERROR;
  }
#else
  location = handle->initData.portLocation;

  if ( 0 ) {
  #if defined(USART0)
  } else if (handle->initData.port == USART0) {
    handle->txPort  = (GPIO_Port_TypeDef)AF_USART0_TX_PORT(location);
    handle->rxPort  = (GPIO_Port_TypeDef)AF_USART0_RX_PORT(location);
    handle->txPin   = AF_USART0_TX_PIN(location);
    handle->rxPin   = AF_USART0_RX_PIN(location);
  #endif
  #if defined(USART1)
  } else if (handle->initData.port == USART1) {
    handle->txPort  = (GPIO_Port_TypeDef)AF_USART1_TX_PORT(location);
    handle->rxPort  = (GPIO_Port_TypeDef)AF_USART1_RX_PORT(location);
    handle->txPin   = AF_USART1_TX_PIN(location);
    handle->rxPin   = AF_USART1_RX_PIN(location);
  #endif
  #if defined(USART2)
  } else if (handle->initData.port == USART2) {
    handle->txPort  = (GPIO_Port_TypeDef)AF_USART2_TX_PORT(location);
    handle->rxPort  = (GPIO_Port_TypeDef)AF_USART2_RX_PORT(location);
    handle->txPin   = AF_USART2_TX_PIN(location);
    handle->rxPin   = AF_USART2_RX_PIN(location);
  #endif
  #if defined(UART0)
  } else if (handle->initData.port == UART0) {
    handle->txPort  = (GPIO_Port_TypeDef)AF_UART0_TX_PORT(location);
    handle->rxPort  = (GPIO_Port_TypeDef)AF_UART0_RX_PORT(location);
    handle->txPin   = AF_UART0_TX_PIN(location);
    handle->rxPin   = AF_UART0_RX_PIN(location);
  #endif
  #if defined(UART1)
  } else if (handle->initData.port == UART1) {
    handle->txPort  = (GPIO_Port_TypeDef)AF_UART1_TX_PORT(location);
    handle->rxPort  = (GPIO_Port_TypeDef)AF_UART1_RX_PORT(location);
    handle->txPin   = AF_UART1_TX_PIN(location);
    handle->rxPin   = AF_UART1_RX_PIN(location);
  #endif
  } else {
    return ECODE_EMDRV_UARTDRV_PARAM_ERROR;
  }
#endif

  if (enable)
  {
    GPIO_PinModeSet(handle->txPort, handle->txPin, gpioModePushPull, 1);
    GPIO_PinModeSet(handle->rxPort, handle->rxPin, gpioModeInput, 0);
    #if (EMDRV_UARTDRV_HW_FLOW_CONTROL_ENABLE)
    if (handle->initData.fcType == uartdrvFlowControlHw)
    {
      GPIO_PinModeSet(handle->initData.ctsPort, handle->initData.ctsPin, gpioModeInput, 0);
      GPIO_PinModeSet(handle->initData.rtsPort, handle->initData.rtsPin, gpioModePushPull, 0);
      GPIO_IntConfig(handle->initData.ctsPort, handle->initData.ctsPin, true, true, true);
    }
    #endif
  }
  else
  {
    GPIO_PinModeSet(handle->txPort, handle->txPin, gpioModeDisabled, 0);
    GPIO_PinModeSet(handle->rxPort, handle->rxPin, gpioModeDisabled, 0);
    #if (EMDRV_UARTDRV_HW_FLOW_CONTROL_ENABLE)
    if (handle->initData.fcType == uartdrvFlowControlHw)
    {
      GPIO_PinModeSet(handle->initData.ctsPort, handle->initData.ctsPin, gpioModeDisabled, 0);
      GPIO_PinModeSet(handle->initData.rtsPort, handle->initData.rtsPin, gpioModeDisabled, 0);
      GPIO_IntConfig(handle->initData.ctsPort, handle->initData.ctsPin, true, true, false);
    }
    #endif
  }
  return ECODE_EMDRV_UARTDRV_OK;
}

/// @endcond


/***************************************************************************//**
 * @brief
 *    Initialize a UART driver instance.
 *
 * @param[out] handle  Pointer to a UART driver handle, refer to @ref
 *                     UARTDRV_Handle_t.
 *
 * @param[in] initData Pointer to an initialization data structure,
 *                     refer to @ref UARTDRV_Init_t.
 *
 * @return
 *    @ref ECODE_EMDRV_UARTDRV_OK on success. On failure an appropriate
 *    UARTDRV @ref Ecode_t is returned.
 ******************************************************************************/
Ecode_t UARTDRV_Init(UARTDRV_Handle_t handle, UARTDRV_Init_t *initData)
{
  Ecode_t retVal;
  #if (EMDRV_UARTDRV_HW_FLOW_CONTROL_ENABLE)
  uint32_t handleIdx;
  bool handleIsSet;
  #endif
  USART_InitAsync_TypeDef usartInit = USART_INITASYNC_DEFAULT;


  if (handle == NULL)
  {
    return ECODE_EMDRV_UARTDRV_ILLEGAL_HANDLE;
  }
  if (initData == NULL)
  {
    return ECODE_EMDRV_UARTDRV_PARAM_ERROR;
  }
  memset(handle, 0, sizeof(UARTDRV_HandleData_t));


  #if (EMDRV_UARTDRV_HW_FLOW_CONTROL_ENABLE)
  // Set handler pointer in handler array
  if (!uartdrvHandleIsInitialized)
  {
    for (handleIdx = 0; handleIdx < EMDRV_UARTDRV_MAX_DRIVER_INSTANCES; handleIdx++)
    {
      uartdrvHandle[handleIdx] = NULL;
    }
    uartdrvHandleIsInitialized = true;
  }

  handleIsSet = false;
  for (handleIdx = 0; handleIdx < EMDRV_UARTDRV_MAX_DRIVER_INSTANCES; handleIdx++)
  {
    if ((uartdrvHandle[handleIdx] == NULL) || (uartdrvHandle[handleIdx] == handle))
    {
      uartdrvHandle[handleIdx] = handle;
      handleIsSet = true;
      break;
    }
  }

  if (!handleIsSet)
  {
    return ECODE_EMDRV_UARTDRV_ILLEGAL_HANDLE;
  }
  #else
  // Force init data to uartdrvFlowControlNone if flow control is excluded by EMDRV_UARTDRV_HW_FLOW_CONTROL_ENABLE
  handle->initData.fcType = uartdrvFlowControlNone;
  #endif


  // Set clocks and DMA requests according to available peripherals
  if (false)
  {
  #if defined(USART0)
  } else if (initData->port == USART0)
  {
    handle->uartClock   = cmuClock_USART0;
    handle->txDmaSignal = dmadrvPeripheralSignal_USART0_TXBL;
    handle->rxDmaSignal = dmadrvPeripheralSignal_USART0_RXDATAV;
  #endif
  #if defined(USART1)
  }
  else if (initData->port == USART1)
  {
    handle->uartClock   = cmuClock_USART1;
    handle->txDmaSignal = dmadrvPeripheralSignal_USART1_TXBL;
    handle->rxDmaSignal = dmadrvPeripheralSignal_USART1_RXDATAV;
  #endif
  #if defined(USART2)
  }
  else if (initData->port == USART2)
  {
    handle->uartClock   = cmuClock_USART2;
    handle->txDmaSignal = dmadrvPeripheralSignal_USART2_TXBL;
    handle->rxDmaSignal = dmadrvPeripheralSignal_USART2_RXDATAV;
  #endif
  #if defined(UART0)
  }
  else if (initData->port == UART0)
  {
    handle->uartClock   = cmuClock_UART0;
    handle->txDmaSignal = dmadrvPeripheralSignal_UART0_TXBL;
    handle->rxDmaSignal = dmadrvPeripheralSignal_UART0_RXDATAV;
  #endif
  #if defined(UART1)
  }
  else if (initData->port == UART1)
  {
    handle->uartClock   = cmuClock_UART1;
    handle->txDmaSignal = dmadrvPeripheralSignal_UART1_TXBL;
    handle->rxDmaSignal = dmadrvPeripheralSignal_UART1_RXDATAV;
  #endif
  #if defined(UART2)
  }
  else if (initData->port == UART2)
  {
    handle->uartClock   = cmuClock_UART2;
    handle->txDmaSignal = dmadrvPeripheralSignal_UART2_TXBL;
    handle->rxDmaSignal = dmadrvPeripheralSignal_UART2_RXDATAV;
  #endif
  }
  else
  {
    return ECODE_EMDRV_UARTDRV_PARAM_ERROR;
  }

  memcpy((void *)&handle->initData, (const void *)initData, sizeof(UARTDRV_Init_t));
  handle->rxQueue = initData->rxQueue;
  handle->rxQueue->head = 0;
  handle->rxQueue->tail = 0;
  handle->rxQueue->used = 0;
  handle->rxDmaActive = false;

  handle->txQueue = initData->txQueue;
  handle->txQueue->head = 0;
  handle->txQueue->tail = 0;
  handle->txQueue->used = 0;
  handle->txDmaActive = false;

  handle->IgnoreRestrain = false;

  usartInit.baudrate = initData->baudRate;
  usartInit.stopbits = initData->stopBits;
  usartInit.parity = initData->parity;
  usartInit.oversampling = initData->oversampling;
#if defined(USART_CTRL_MVDIS)
  usartInit.mvdis = initData->mvdis;
#endif

  // UARTDRV is fixed at 8 bit frames.
  usartInit.databits = (USART_Databits_TypeDef)USART_FRAME_DATABITS_EIGHT;

  // Enable clocks
  CMU_ClockEnable(cmuClock_HFPER, true);
  CMU_ClockEnable(cmuClock_GPIO, true);
  CMU_ClockEnable(handle->uartClock, true);

  // Init U(S)ART to default async config.
  // RX/TX enable is done on demand
  usartInit.enable = usartDisable;
  USART_InitAsync(initData->port, &usartInit);

  #if defined( USART_ROUTEPEN_TXPEN )
  initData->port->ROUTEPEN = USART_ROUTEPEN_TXPEN
                             | USART_ROUTEPEN_RXPEN;
  initData->port->ROUTELOC0 = ( initData->port->ROUTELOC0 &
                                ~( _USART_ROUTELOC0_TXLOC_MASK
                                   | _USART_ROUTELOC0_RXLOC_MASK ) )
                              | ( initData->portLocationTx  << _USART_ROUTELOC0_TXLOC_SHIFT  )
                              | ( initData->portLocationRx  << _USART_ROUTELOC0_RXLOC_SHIFT  );
  #else
  initData->port->ROUTE = USART_ROUTE_TXPEN
                        | USART_ROUTE_RXPEN
                        | (initData->portLocation
                          << _USART_ROUTE_LOCATION_SHIFT);
  #endif

  if ((retVal = ConfigGPIO(handle, true)) != ECODE_EMDRV_UARTDRV_OK)
  {
    return retVal;
  }

  INT_Disable();

  // Configure hardware flow control pins and interrupt vectors
  #if (EMDRV_UARTDRV_HW_FLOW_CONTROL_ENABLE)
  GPIOINT_Init();
  GPIOINT_CallbackRegister(initData->ctsPin, HwFcManageClearToSend);
  handle->fcPeerState = uartdrvFlowControlOn;
  handle->fcSelfState = uartdrvFlowControlOn;
  handle->fcSelfCfg = uartdrvFlowControlAuto;
  FcApplyState(handle);
  #endif

  // Clear any false IRQ/DMA request
  USART_IntClear(initData->port, ~0x0);

  // Enable TX permanently as the TX circuit consumes very little energy.
  // RX is enabled on demand as the RX circuit consumes some energy due to
  // continuous (over)sampling.
  USART_Enable(initData->port, usartEnableTx);

  // Discard false frames and/or IRQs
  initData->port->CMD = USART_CMD_CLEARRX | USART_CMD_CLEARTX;

  // Initialize DMA.
  DMADRV_Init();

  if ( DMADRV_AllocateChannel(&handle->txDmaCh,NULL) != ECODE_EMDRV_DMADRV_OK )
  {
    INT_Enable();
    return ECODE_EMDRV_UARTDRV_DMA_ALLOC_ERROR;
  }

  if ( DMADRV_AllocateChannel(&handle->rxDmaCh,NULL) != ECODE_EMDRV_DMADRV_OK )
  {
    INT_Enable();
    return ECODE_EMDRV_UARTDRV_DMA_ALLOC_ERROR;
  }

  INT_Enable();
  return ECODE_EMDRV_UARTDRV_OK;
}


/***************************************************************************//**
 * @brief
 *    Deinitialize a UART driver instance.
 *
 * @param[in] handle Pointer to a UART driver handle.
 *
 * @return
 *    @ref ECODE_EMDRV_UARTDRV_OK on success. On failure an appropriate
 *    UARTDRV @ref Ecode_t is returned.
 ******************************************************************************/
Ecode_t UARTDRV_DeInit(UARTDRV_Handle_t handle)
{
  if (handle == NULL)
  {
    return ECODE_EMDRV_UARTDRV_ILLEGAL_HANDLE;
  }
  // Stop DMA's.
  DMADRV_StopTransfer( handle->rxDmaCh );
  DMADRV_StopTransfer( handle->txDmaCh );

  // Do not leave any peer restrained on DeInit
  UARTDRV_FlowControlSet(handle, uartdrvFlowControlOn);

  ConfigGPIO(handle, false);

  USART_Reset(handle->initData.port);
  handle->initData.port->CMD = USART_CMD_RXDIS;
  handle->initData.port->CMD = USART_CMD_TXDIS;

  CMU_ClockEnable(handle->uartClock, false);

  #if (EMDRV_UARTDRV_HW_FLOW_CONTROL_ENABLE)
  GPIOINT_CallbackRegister(handle->initData.ctsPin, NULL);
  #endif

  DMADRV_FreeChannel( handle->txDmaCh );
  DMADRV_FreeChannel( handle->rxDmaCh );
  DMADRV_DeInit();

  handle->rxQueue->head = 0;
  handle->rxQueue->tail = 0;
  handle->rxQueue->used = 0;

  handle->txQueue->head = 0;
  handle->txQueue->tail = 0;
  handle->txQueue->used = 0;

  return ECODE_EMDRV_UARTDRV_OK;
}


/***************************************************************************//**
 * @brief
 *    Abort an ongoing UART transfer.
 *
 * @param[in] handle Pointer to a UART driver handle.
 *
 * @param[in] type Abort type
 *
 * @return
 *    @ref ECODE_EMDRV_UARTDRV_OK on success, @ref ECODE_EMDRV_UARTDRV_IDLE if
 *    the UART is idle. On failure an appropriate UARTDRV @ref Ecode_t is returned.
 ******************************************************************************/
Ecode_t UARTDRV_Abort(UARTDRV_Handle_t handle, UARTDRV_AbortType_t type)
{
  UARTDRV_Buffer_t *rxBuffer, *txBuffer;

  if (handle == NULL) {
    return ECODE_EMDRV_UARTDRV_ILLEGAL_HANDLE;
  }

  INT_Disable();
  if ((type == uartdrvAbortTransmit) && (handle->txQueue->used == 0))
  {
    INT_Enable();
    return ECODE_EMDRV_UARTDRV_IDLE;
  }
  else if ((type == uartdrvAbortReceive) && (handle->rxQueue->used == 0))
  {
    INT_Enable();
    return ECODE_EMDRV_UARTDRV_IDLE;
  }
  else if ((type == uartdrvAbortAll) && (handle->txQueue->used == 0)
                                     && (handle->rxQueue->used == 0))
  {
    INT_Enable();
    return ECODE_EMDRV_UARTDRV_IDLE;
  }

  // Stop DMA's.
  if ((type == uartdrvAbortTransmit) || (type == uartdrvAbortAll))
  {
    GetTailBuffer(handle->txQueue, &txBuffer);
    DMADRV_StopTransfer( handle->txDmaCh );
    DMADRV_TransferRemainingCount( handle->txDmaCh,
                                   (int*)&txBuffer->itemsRemaining );
    txBuffer->transferStatus = ECODE_EMDRV_UARTDRV_ABORTED;
    if (txBuffer->callback != NULL)
    {
      txBuffer->callback(handle,
                         ECODE_EMDRV_UARTDRV_ABORTED,
                         NULL,
                         txBuffer->itemsRemaining);
    }
  }
  if ((type == uartdrvAbortReceive) || (type == uartdrvAbortAll))
  {
    GetTailBuffer(handle->rxQueue, &rxBuffer);
    DMADRV_StopTransfer( handle->rxDmaCh );
    DMADRV_TransferRemainingCount( handle->rxDmaCh,
                                   (int*)&rxBuffer->itemsRemaining );
    rxBuffer->transferStatus = ECODE_EMDRV_UARTDRV_ABORTED;
    if (rxBuffer->callback != NULL)
    {
      rxBuffer->callback(handle,
                         ECODE_EMDRV_UARTDRV_ABORTED,
                         NULL,
                         rxBuffer->itemsRemaining);
    }
  }
  INT_Enable();

  return ECODE_EMDRV_UARTDRV_OK;
}


/***************************************************************************//**
 * @brief
 *    Returns the number of queued receive operations.
 *
 * @param[in] handle Pointer to a UART driver handle.
 *
 * @return
 *    The number of queued operations.
 ******************************************************************************/
uint8_t UARTDRV_GetReceiveDepth(UARTDRV_Handle_t handle)
{
  return (uint8_t)handle->rxQueue->used;
}


/***************************************************************************//**
 * @brief
 *    Check the status of the UART and gather information about any ongoing
 *    receive operations.
 *
 * @param[in] handle Pointer to a UART driver handle.
 *
 * @param[in] buffer Pointer to the current data buffer.
 *
 * @param[in] itemsReceived Current bytes received count.
 *
 * @param[in] itemsRemaining Current bytes remaining count.
 *
 * @return
 *    UART status.
 ******************************************************************************/
UARTDRV_Status_t UARTDRV_GetReceiveStatus(UARTDRV_Handle_t handle,
                                          uint8_t **buffer,
                                          UARTDRV_Count_t *itemsReceived,
                                          UARTDRV_Count_t *itemsRemaining)
{
  UARTDRV_Buffer_t *rxBuffer;
  uint32_t remaining;

  if (handle->rxQueue->used > 0)
  {
    GetTailBuffer(handle->rxQueue, &rxBuffer);
    DMADRV_TransferRemainingCount( handle->rxDmaCh,
                                   (int*)&remaining );

    *itemsReceived = rxBuffer->transferCount - remaining;
    *itemsRemaining = remaining;
    *buffer = rxBuffer->data;
  }
  else
  {
    *itemsRemaining = 0;
    *itemsReceived = 0;
    *buffer = NULL;
  }
  return handle->initData.port->STATUS;
}


/***************************************************************************//**
 * @brief
 *    Returns the number of queued transmit operations.
 *
 * @param[in] handle Pointer to a UART driver handle.
 *
 * @return
 *    The number of queued operations.
 ******************************************************************************/
uint8_t UARTDRV_GetTransmitDepth(UARTDRV_Handle_t handle)
{
  return (uint8_t)handle->txQueue->used;
}


/***************************************************************************//**
 * @brief
 *    Check the status of the UART and gather information about any ongoing
 *    transmit operations.
 *
 * @param[in] handle Pointer to a UART driver handle.
 *
 * @param[in] buffer Pointer to the current data buffer.
 *
 * @param[in] itemsSent Current bytes sent count.
 *
 * @param[in] itemsRemaining Current bytes remaining count.
 *
 * @return
 *    UART status.
 ******************************************************************************/
UARTDRV_Status_t UARTDRV_GetTransmitStatus(UARTDRV_Handle_t handle,
                                           uint8_t **buffer,
                                           UARTDRV_Count_t *itemsSent,
                                           UARTDRV_Count_t *itemsRemaining)
{
  UARTDRV_Buffer_t *txBuffer;
  uint32_t remaining;

  if (handle->txQueue->used > 0)
  {
    GetTailBuffer(handle->txQueue, &txBuffer);
    DMADRV_TransferRemainingCount( handle->txDmaCh,
                                   (int*)&remaining );

    *itemsSent = txBuffer->transferCount - remaining;
    *itemsRemaining = remaining;
    *buffer = txBuffer->data;
  }
  else
  {
    *itemsRemaining = 0;
    *itemsSent = 0;
    *buffer = NULL;
  }
  return handle->initData.port->STATUS;
}


/***************************************************************************//**
 * @brief
 *    Set UART flow control state. Set nRTS pin if hardware flow control
 *    is enabled.
 *
 * @param[in] handle Pointer to a UART driver handle.
 *
 * @param[in] state Flow control state.
 *
 * @return
 *    @ref ECODE_EMDRV_UARTDRV_OK on success.
 ******************************************************************************/
Ecode_t UARTDRV_FlowControlSet(UARTDRV_Handle_t handle, UARTDRV_FlowControlState_t state)
{
  #if (EMDRV_UARTDRV_HW_FLOW_CONTROL_ENABLE)
  handle->fcSelfCfg = state;
  if (state != uartdrvFlowControlAuto)
  {
    handle->fcSelfState = state;
  }
  return FcApplyState(handle);
  #else
  return ECODE_EMDRV_UARTDRV_OK;
  #endif
}


/***************************************************************************//**
 * @brief
 *    Checks the peer's flow control status
 *
 * @param[in] handle Pointer to a UART driver handle.
 *
 * @return
 *    Returns uartdrvFlowControlOn if clear to send.
 ******************************************************************************/
UARTDRV_FlowControlState_t UARTDRV_FlowControlGetPeerStatus(UARTDRV_Handle_t handle)
{
  return handle->fcPeerState;
}


/***************************************************************************//**
 * @brief
 *    Checks the self's flow control status
 *
 * @param[in] handle Pointer to a UART driver handle.
 *
 * @return
 *    Returns uartdrvFlowControlOn if requesting to send.
 ******************************************************************************/
UARTDRV_FlowControlState_t UARTDRV_FlowControlGetSelfStatus(UARTDRV_Handle_t handle)
{
  return handle->fcSelfState;
}


/***************************************************************************//**
 * @brief
 *    Enables transmission when restrained by flow control
 *
 * @param[in] handle Pointer to a UART driver handle.
 *
 * @return
 *    @ref ECODE_EMDRV_UARTDRV_OK on success.
 ******************************************************************************/
Ecode_t UARTDRV_FlowControlIgnoreRestrain(UARTDRV_Handle_t handle)
{
  handle->IgnoreRestrain = true;

  return ECODE_EMDRV_UARTDRV_OK;
}


/***************************************************************************//**
 * @brief
 *    Direct receive without interrupts or callback. Blocking function.
 *
 * @param[in] handle Pointer to a UART driver handle.
 *
 * @param[in] data pointer to buffer.
 *
 * @param[in] maxCount Maximum number of bytes to receive.
 *
 * @return
 *    Number of bytes received.
 ******************************************************************************/
UARTDRV_Count_t UARTDRV_ForceReceive(UARTDRV_Handle_t handle,
                                      uint8_t *data,
                                      UARTDRV_Count_t maxCount)
{
  Ecode_t retVal;
  uint32_t rxState;
  UARTDRV_Count_t i = 0;

  retVal = CheckParams(handle, data, maxCount);
  if (retVal != ECODE_EMDRV_UARTDRV_OK)
  {
    return 0;
  }

  // Wait for DMA receive to complete and clear
  while(handle->rxQueue->used > 0);

  rxState = (handle->initData.port->STATUS & USART_STATUS_RXENS);
  if (!rxState)
  {
    EnableReceiver(handle);
  }

  while ((handle->initData.port->STATUS & USART_STATUS_RXDATAV))
  {
    *data = (uint8_t)handle->initData.port->RXDATA;
    data++;
    i++;
    if (i >= maxCount)
    {
      break;
    }
  }
  data -= i;

  if (!rxState)
  {
    DisableReceiver(handle);
  }
  return i;
}


/***************************************************************************//**
 * @brief
 *    Direct transmit without interrupts or callback. Blocking function
 *    that ignores flow control if enabled.
 *
 * @param[in] handle Pointer to a UART driver handle.
 *
 * @param[in] data Pointer to buffer.
 *
 * @param[in] count Number of bytes to transmit.
 *
 * @return
 *    @ref ECODE_EMDRV_UARTDRV_OK on success.
 ******************************************************************************/
Ecode_t  UARTDRV_ForceTransmit(UARTDRV_Handle_t handle,
                               uint8_t *data,
                               UARTDRV_Count_t count)
{
  Ecode_t retVal;
  uint32_t txState;

  retVal = CheckParams(handle, data, count);
  if (retVal != ECODE_EMDRV_UARTDRV_OK)
  {
    return retVal;
  }

  // Wait for DMA transmit to complete and clear
  while(handle->txQueue->used > 0);

  txState = (handle->initData.port->STATUS & USART_STATUS_TXENS);
  if (!txState)
  {
    EnableTransmitter(handle);
  }

  while (count--)
  {
    while (!(handle->initData.port->STATUS & USART_STATUS_TXBL));
    handle->initData.port->TXDATA = *data;
    data++;
  }
  while (!(handle->initData.port->STATUS & USART_STATUS_TXC));

  if (!txState)
  {
    DisableTransmitter(handle);
  }

  return ECODE_EMDRV_UARTDRV_OK;
}


/***************************************************************************//**
 * @brief
 *    Start a non-blocking receive.
 *
 * @param[in] handle Pointer to a UART driver handle.
 *
 * @param[in] data Receive data buffer.
 *
 * @param[in] count Number of bytes received.
 *
 * @param[in]  callback Transfer completion callback.
 *
 * @return
 *    @ref ECODE_EMDRV_UARTDRV_OK on success.
 ******************************************************************************/
Ecode_t UARTDRV_Receive(UARTDRV_Handle_t handle,
                        uint8_t *data,
                        UARTDRV_Count_t count,
                        UARTDRV_Callback_t callback)
{
  Ecode_t retVal;
  UARTDRV_Buffer_t outputBuffer;
  UARTDRV_Buffer_t *queueBuffer;

  retVal = CheckParams(handle, data, count);
  if (retVal != ECODE_EMDRV_UARTDRV_OK)
  {
    return retVal;
  }
  outputBuffer.data = data;
  outputBuffer.transferCount = count;
  outputBuffer.itemsRemaining = count;
  outputBuffer.callback = callback;
  outputBuffer.transferStatus = ECODE_EMDRV_UARTDRV_WAITING;

  retVal = EnqueueBuffer(handle->rxQueue, &outputBuffer, &queueBuffer);
  if (retVal != ECODE_EMDRV_UARTDRV_OK)
  {
    return retVal;
  }
  if (!(handle->rxDmaActive))
  {
    EnableReceiver(handle);
    StartReceiveDma(handle, queueBuffer);
    #if (EMDRV_UARTDRV_HW_FLOW_CONTROL_ENABLE)
    handle->fcSelfState = uartdrvFlowControlOn;
    FcApplyState(handle);
    #endif
  } // else: started by ReceiveDmaComplete

  return ECODE_EMDRV_UARTDRV_OK;
}


/***************************************************************************//**
 * @brief
 *    Start a blocking receive.
 *
 * @param[in] handle Pointer to a UART driver handle.
 *
 * @param[in] data Receive data buffer.
 *
 * @param[in] count Number of bytes received.
 *
 * @return
 *    @ref ECODE_EMDRV_UARTDRV_OK on success.
 ******************************************************************************/
Ecode_t UARTDRV_ReceiveB(UARTDRV_Handle_t handle,
                   uint8_t *data,
                   UARTDRV_Count_t count)
{
  Ecode_t retVal;
  UARTDRV_Buffer_t inputBuffer;
  UARTDRV_Buffer_t *queueBuffer;

  retVal = CheckParams(handle, data, count);
  if (retVal != ECODE_EMDRV_UARTDRV_OK)
  {
    return retVal;
  }
  inputBuffer.data = data;
  inputBuffer.transferCount = count;
  inputBuffer.itemsRemaining = count;
  inputBuffer.callback = NULL;
  inputBuffer.transferStatus = ECODE_EMDRV_UARTDRV_WAITING;

  retVal = EnqueueBuffer(handle->rxQueue, &inputBuffer, &queueBuffer);
  if (retVal != ECODE_EMDRV_UARTDRV_OK)
  {
    return retVal;
  }
  while(handle->rxQueue->used > 1)
  {
    EMU_EnterEM1();
  }
  EnableReceiver(handle);
  #if (EMDRV_UARTDRV_HW_FLOW_CONTROL_ENABLE)
  handle->fcSelfState = uartdrvFlowControlOn;
  FcApplyState(handle);
  #endif
  StartReceiveDma(handle, queueBuffer);
  while(handle->rxDmaActive)
  {
    EMU_EnterEM1();
  }
  return queueBuffer->transferStatus;
}


/***************************************************************************//**
 * @brief
 *    Start a non-blocking transmit.
 *
 * @param[in] handle Pointer to a UART driver handle.
 *
 * @param[in] data Transmit data buffer.
 *
 * @param[in] count Number of bytes to transmit.
 *
 * @param[in]  callback Transfer completion callback.
 *
 * @return
 *    @ref ECODE_EMDRV_UARTDRV_OK on success.
 ******************************************************************************/
Ecode_t UARTDRV_Transmit(UARTDRV_Handle_t handle,
                         uint8_t *data,
                         UARTDRV_Count_t count,
                         UARTDRV_Callback_t callback)
{
  Ecode_t retVal;
  UARTDRV_Buffer_t inputBuffer;
  UARTDRV_Buffer_t *queueBuffer;

  retVal = CheckParams(handle, data, count);
  if (retVal != ECODE_EMDRV_UARTDRV_OK)
  {
    return retVal;
  }
  inputBuffer.data = data;
  inputBuffer.transferCount = count;
  inputBuffer.itemsRemaining = count;
  inputBuffer.callback = callback;
  inputBuffer.transferStatus = ECODE_EMDRV_UARTDRV_WAITING;

  retVal = EnqueueBuffer(handle->txQueue, &inputBuffer, &queueBuffer);
  if (retVal != ECODE_EMDRV_UARTDRV_OK)
  {
    return retVal;
  }
  if (!(handle->txDmaActive))
  {
    StartTransmitDma(handle, queueBuffer);
  } // else: started by TransmitDmaComplete

  return ECODE_EMDRV_UARTDRV_OK;
}


/***************************************************************************//**
 * @brief
 *    Start a blocking transmit.
 *
 * @param[in] handle Pointer to a UART driver handle.
 *
 * @param[in] data Transmit data buffer.
 *
 * @param[in] count Number of bytes to transmit.
 *
 * @return
 *    @ref ECODE_EMDRV_UARTDRV_OK on success.
 ******************************************************************************/
Ecode_t UARTDRV_TransmitB(UARTDRV_Handle_t handle,
                          uint8_t *data,
                          UARTDRV_Count_t count)
{
  Ecode_t retVal;
  UARTDRV_Buffer_t outputBuffer;
  UARTDRV_Buffer_t *queueBuffer;

  retVal = CheckParams(handle, data, count);
  if (retVal != ECODE_EMDRV_UARTDRV_OK)
  {
    return retVal;
  }
  outputBuffer.data = data;
  outputBuffer.transferCount = count;
  outputBuffer.itemsRemaining = count;
  outputBuffer.callback = NULL;
  outputBuffer.transferStatus = ECODE_EMDRV_UARTDRV_WAITING;

  retVal = EnqueueBuffer(handle->txQueue, &outputBuffer, &queueBuffer);
  if (retVal != ECODE_EMDRV_UARTDRV_OK)
  {
    return retVal;
  }
  while(handle->txQueue->used > 1)
  {
    EMU_EnterEM1();
  }
  StartTransmitDma(handle, queueBuffer);
  while(handle->txDmaActive)
  {
    EMU_EnterEM1();
  }
  return queueBuffer->transferStatus;
}


/******** THE REST OF THE FILE IS DOCUMENTATION ONLY !**********************//**
 * @addtogroup UARTDRV
 * @{

@page uartdrv_doc UARTDRV Universal asynchronous receiver/transmitter driver

  The source files for the UART driver library resides in the
  emdrv/uartdrv folder, and are named uartdrv.c and uartdrv.h.

  @li @ref uartdrv_intro
  @li @ref uartdrv_conf
  @li @ref uartdrv_api
  @li @ref uartdrv_fc
  @li @ref uartdrv_example

@n @section uartdrv_intro Introduction
  The UART driver support the UART capabilities of the USART and UART peripherals,
  but not the LEUART peripherals. The driver is fully reentrant and multiple driver
  instances can coexist. The driver does not buffer or queue data, but it queues UART
  transmit and receive operations. Both blocking and non-blocking transfer functions are
  available. Non-blocking transfer functions report transfer completion with callback
  functions. Transfers are done using DMA. Simple direct/forced transmit and receive
  functions are also available. Note that these functions are blocking and not suitable
  for low energy applications as they use CPU polling.

  UART hardware flow control (CTS/RTS) is fully supported by the driver. UART software
  flow control (XON/XOFF) is partially supported by the driver. Read more about flow control
  support in @ref uartdrv_fc.

  @note Transfer completion callback functions are called from within the DMA
  interrupt handler with interrupts disabled.

@n @section uartdrv_conf Configuration Options

  Some properties of the UARTDRV driver are compile-time configurable. These
  properties are set in a file named @ref uartdrv_config.h. A template for this
  file, containing default values, resides in the emdrv/config folder.
  To configure UARTDRV for your application, provide your own configuration file. These are the available configuration
  parameters with default values defined.
  @verbatim

  // Maximum concurrent receive operations
  #define EMDRV_UARTDRV_MAX_CONCURRENT_RX_BUFS    6

  // Maximum concurrent transmit operations
  #define EMDRV_UARTDRV_MAX_CONCURRENT_TX_BUFS    6

  // Set to 1 to enable hardware flow control
  #define EMDRV_UARTDRV_HW_FLOW_CONTROL_ENABLE    1

  // Maximum number of driver instances. This maximum applies only when EMDRV_UARTDRV_HW_FLOW_CONTROL_ENABLE = 1
  #define EMDRV_UARTDRV_MAX_DRIVER_INSTANCES      4

  // UART software flow control code: request peer to start TX
  #define UARTDRV_FC_SW_XON                       0x11

  // UART software flow control code: request peer to stop TX
  #define UARTDRV_FC_SW_XOFF                      0x13
  @endverbatim

  The properties of each UART driver instance are set at run-time via the
  @ref UARTDRV_Init_t data structure input parameter to the @ref UARTDRV_Init()
  function.

@n @section uartdrv_api The API

  This section contain brief descriptions of the functions in the API. You will
  find detailed information on input and output parameters and return values by
  clicking on the hyperlinked function names. Most functions return an error
  code, @ref ECODE_EMDRV_UARTDRV_OK is returned on success,
  see @ref ecode.h and @ref uartdrv.h for other error codes.

  Your application code must include one header file: @em uartdrv.h.

  @ref UARTDRV_Init(), @ref UARTDRV_DeInit() @n
    These functions initializes or deinitializes the UARTDRV driver. Typically
    @htmlonly UARTDRV_Init() @endhtmlonly is called once in your startup code.

  @ref UARTDRV_GetReceiveStatus(), @ref UARTDRV_GetTransmitStatus() @n
    Query the status of a current transmit or receive operations. Reports number
    of items (frames) transmitted and remaining.

  @ref UARTDRV_GetReceiveDepth(), @ref UARTDRV_GetTransmitDepth() @n
    Get the number of queued receive or transmit operations.

  @ref UARTDRV_Transmit(), UARTDRV_Receive() @n
  UARTDRV_TransmitB(), UARTDRV_ReceiveB() @n
  UARTDRV_ForceTransmit(), UARTDRV_ForceReceive() @n
    Transfer functions come in both blocking and non-blocking versions,
    the blocking versions have an uppercase B (for Blocking) at the end of
    their function name. Blocking functions will not return before the transfer
    has completed. The non-blocking functions signal transfer completion with a
    callback function. @ref UARTDRV_ForceTransmit() and @ref UARTDRV_ForceReceive()
    are also blocking. These two functions access the UART peripheral directly without
    using DMA or interrupts. @ref UARTDRV_ForceTransmit() does not respect flow control.
    @ref UARTDRV_ForceReceive() forces RTS low.

  @ref UARTDRV_Abort() @n
    Abort current transmit or receive operations.

  @ref UARTDRV_FlowControlSet(), UARTDRV_FlowControlGetSelfStatus(), UARTDRV_FlowControlGetPeerStatus() @n
    Set and get flow control status of self or peer device. Note that the return value from
    these two functions depends on the flow control mode set by @ref UARTDRV_FlowControlSet() or
    @ref UARTDRV_Init().

  @ref UARTDRV_FlowControlIgnoreRestrain() @n
    Enables transmission when restrained by flow control.

@n @section uartdrv_fc Flow Control Support

  If UART flow control is not required, make sure EMDRV_UARTDRV_HW_FLOW_CONTROL_ENABLE is set to 0. This reduces
  the code size and complexity of the driver.

  There are two types of flow control supported, hardware and software. To enable any of these,
  set EMDRV_UARTDRV_HW_FLOW_CONTROL_ENABLE to 1 in @ref uartdrv_config.h.

@n @subsection uartdrv_fc_hw Hardware Flow Control

  UART hardware flow control uses two additional pins for flow control handshaking, the clear-to-send (CTS) and ready-to-send (RTS) pins.
  RTS is an output and CTS is an input. These are active low signals. When CTS is high, the UART transmitter should stop sending frames.
  A receiver should set RTS high when it is no longer capable of receiving data.

  To support hardware flow control, the driver includes the GPIOINT driver to emulate a hardware implementation
  of UART CTS/RTS flow control. Some revisions of the USART peripheral does not have CTS/RTS hardware support.

  To enable hardware flow control, perform the following steps:

  -# Set EMDRV_UARTDRV_HW_FLOW_CONTROL_ENABLE to 1.
  -# In the @ref UARTDRV_Init_t passed to @ref UARTDRV_Init(), set @ref UARTDRV_Init_t.fcType = uartdrvFlowControlHw.
  -# Define the pins for CTS and RTS by setting ctsPort, ctsPin, rtsPort and rtsPin in the @ref UARTDRV_Init_t passed to @ref UARTDRV_Init().

  @note Because of the limitations in GPIO interrupt hardware, you cannot select CTS pins in multiple driver instances with the same pin number.
  Ie. pin A0 and B0 cannot serve as CTS pins in two concurrent driver instances.

  RTS is set high whenever there are no RX operations queued. The UART transmitter is halted when the CTS pin goes high. The transmitter completes the
  current frame before halting. DMA transfers are also halted.

@n @subsection uartdrv_fc_sw Software Flow Control

  UART software flow control uses in-band signaling, meaning the receiver sends special flow control characters to the transmitter and thereby removes
  the need for dedicated wires for flow control. The two symbols UARTDRV_FC_SW_XON and UARTDRV_FC_SW_XOFF are defined in @ref uartdrv_config.h.

  To enable support for software flow control, perform the following steps:

  -# Set EMDRV_UARTDRV_HW_FLOW_CONTROL_ENABLE to 1.
  -# In the @ref UARTDRV_Init_t passed to @ref UARTDRV_Init(), set @ref UARTDRV_Init_t.fcType = uartdrvFlowControlSw.

  @note Software flow control is partial only.

  The application must monitor buffers and make decisions on when to send XON/XOFF. XON/XOFF can be sent to the peer using @ref UARTDRV_FlowControlSet().
  If the application implements a specific packet format where the flow control codes may appear only in fixed positions, then the application should not
  use @ref UARTDRV_FlowControlSet(), but implement read and write of XON/XOFF into packet buffers. If the application code fully implements all the flow
  control logic, then EMDRV_UARTDRV_HW_FLOW_CONTROL_ENABLE should be set to 0 to reduce code space.

@n @section uartdrv_example Example
  @verbatim
#include "uartdrv.h"

// Define receive/transmit operation queues
DEFINE_BUF_QUEUE(EMDRV_UARTDRV_MAX_CONCURRENT_RX_BUFS, rxBufferQueueI0);
DEFINE_BUF_QUEUE(EMDRV_UARTDRV_MAX_CONCURRENT_TX_BUFS, txBufferQueueI0);

// Configuration for USART0, location 1
#define MY_UART                                                        \
{                                                                      \
  USART0,                                                              \
  115200,                                                              \
  _USART_ROUTE_LOCATION_LOC1,                                          \
  (USART_Stopbits_TypeDef)USART_FRAME_STOPBITS_ONE,                    \
  (USART_Parity_TypeDef)USART_FRAME_PARITY_NONE,                       \
  (USART_OVS_TypeDef)USART_CTRL_OVS_X16,                               \
  false,                                                               \
  uartdrvFlowControlHw,                                                \
  (GPIO_Port_TypeDef)AF_USART0_CS_PORT(_USART_ROUTE_LOCATION_LOC1),    \
  (GPIO_Port_TypeDef)AF_USART0_CS_PIN(_USART_ROUTE_LOCATION_LOC1),     \
  (GPIO_Port_TypeDef)AF_USART0_CLK_PORT(_USART_ROUTE_LOCATION_LOC1),   \
  (GPIO_Port_TypeDef)AF_USART0_CLK_PIN(_USART_ROUTE_LOCATION_LOC1),    \
  (UARTDRV_Buffer_FifoQueue_t *)&rxBufferQueueI0,                      \
  (UARTDRV_Buffer_FifoQueue_t *)&txBufferQueueI0                       \
}

UARTDRV_HandleData_t handleData;
UARTDRV_Handle_t handle = &handleData;

int main(void)
{
  uint8_t buffer[10];
  UARTDRV_Init_t initDataA0 = MY_UART;

  UARTDRV_Init(handle, &initDataA0);

  // Transmit data using a blocking transmit function
  UARTDRV_Transmit(handle, buffer, 10);
}

  @endverbatim

 * @}**************************************************************************/
