/**
  ******************************************************************************
  * @file    usb_def.h
  * @author  MCD Application Team
  * @version V4.1.0
  * @date    26-May-2017
  * @brief   Definitions related to USB Core
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_DEF_H
#define __USB_DEF_H

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
typedef enum _RECIPIENT_TYPE
{
  DEVICE_RECIPIENT,     /* Recipient device */
  INTERFACE_RECIPIENT,  /* Recipient interface */
  ENDPOINT_RECIPIENT,   /* Recipient endpoint */
  OTHER_RECIPIENT
} RECIPIENT_TYPE;


typedef enum _STANDARD_REQUESTS
{
  GET_STATUS = 0,
  CLEAR_FEATURE,
  RESERVED1,
  SET_FEATURE,
  RESERVED2,
  SET_ADDRESS,
  GET_DESCRIPTOR,
  SET_DESCRIPTOR,
  GET_CONFIGURATION,
  SET_CONFIGURATION,
  GET_INTERFACE,
  SET_INTERFACE,
  TOTAL_sREQUEST,  /* Total number of Standard request */
  SYNCH_FRAME = 12
} STANDARD_REQUESTS;

/* Definition of "USBwValue" */
typedef enum _DESCRIPTOR_TYPE
{
  DEVICE_DESCRIPTOR = 1,
  CONFIG_DESCRIPTOR,
  STRING_DESCRIPTOR,
  INTERFACE_DESCRIPTOR,
  ENDPOINT_DESCRIPTOR,
  DEVICE_BOS_DESCRIPTOR = 0xF
} DESCRIPTOR_TYPE;

/* Feature selector of a SET_FEATURE or CLEAR_FEATURE */
typedef enum _FEATURE_SELECTOR
{
  ENDPOINT_STALL,
  DEVICE_REMOTE_WAKEUP
} FEATURE_SELECTOR;

/* Exported constants --------------------------------------------------------*/
/* Definition of "USBbmRequestType" */
#define REQUEST_TYPE      0x60  /* Mask to get request type */
#define STANDARD_REQUEST  0x00  /* Standard request */
#define CLASS_REQUEST     0x20  /* Class request */
#define VENDOR_REQUEST    0x40  /* Vendor request */

#define RECIPIENT         0x1F  /* Mask to get recipient */

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#endif /* __USB_DEF_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
