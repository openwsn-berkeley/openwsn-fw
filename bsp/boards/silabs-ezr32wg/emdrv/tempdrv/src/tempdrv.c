/***************************************************************************//**
 * @file tempdrv.c
 * @brief TEMPDRV API implementation.
 * @version 4.2.1
 *******************************************************************************
 * @section License
 * <b>(C) Copyright 2014 Silicon Labs, http://www.silabs.com</b>
 *******************************************************************************
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Silicon Labs has no
 * obligation to support this Software. Silicon Labs is providing the
 * Software "AS IS", with no express or implied warranties of any kind,
 * including, but not limited to, any implied warranties of merchantability
 * or fitness for any particular purpose or warranties against infringement
 * of any proprietary rights of a third party.
 *
 * Silicon Labs will not be liable for any consequential, incidental, or
 * special damages, or any other relief, or for any claim by any third party,
 * arising from your use of this Software.
 *
 ******************************************************************************/

#include "em_device.h"
#include "em_system.h"
#include "em_emu.h"
#include "string.h"

#include "tempdrv.h"

typedef struct
{
  TEMPDRV_Callback_t callback;    ///< Callback function
  uint8_t temp;                   ///< Limit temperature (EMU value)
} TEMPDRV_CallbackSet_t;

#ifdef _EFR_DEVICE
#define TEMPDRV_INT_CALLBACK_DEPTH 1
#define TEMPDRV_
#else
#define TEMPDRV_INT_CALLBACK_DEPTH 0
#endif

#define TEMPDRV_CALLBACK_DEPTH (TEMPDRV_INT_CALLBACK_DEPTH + TEMPDRV_CUSTOM_CALLBACK_DEPTH)
#define TEMPDRV_CUSTOM_CALLBACK_INDEX TEMPDRV_INT_CALLBACK_DEPTH

static TEMPDRV_CallbackSet_t tempdrvHighCallbacks[TEMPDRV_CALLBACK_DEPTH];
static TEMPDRV_CallbackSet_t tempdrvLowCallbacks[TEMPDRV_CALLBACK_DEPTH];
static TEMPDRV_CallbackSet_t nullCallback = {NULL, 0};
static TEMPDRV_CallbackSet_t *highCallback;
static TEMPDRV_CallbackSet_t *lowCallback;

static bool TEMPDRV_InitState = false;
static bool TEMPDRV_EnableState = false;

static Ecode_t convertEMUtoTEMP(uint8_t emu, int8_t *temp);
static Ecode_t convertTEMPtoEMU(int8_t temp, uint8_t *emu);
static void updateInterrupts();

// Calibration values to be initialized in TEMPDRV_Init
static int32_t calibrationEMU;
static int32_t calibrationTEMP;
// Fallback calibration values in case DI calibration data not present
static uint8_t fallbackEMU = 0x90;
static uint8_t fallbackTEMP = 25;

#if (EMU_CUSTOM_IRQ_HANDLER == false)
/***************************************************************************//**
 * @brief EMU Interrupt Handler
 ******************************************************************************/
void EMU_IRQHandler(void)
{
  TEMPDRV_IRQHandler();
}
#endif

/***************************************************************************//**
 * @brief TEMPDRV Interrupt Handler
 ******************************************************************************/
void TEMPDRV_IRQHandler(void)
{
  uint32_t flags = EMU_IntGetEnabled();
  // EMU and TEMP are inversely proportional
  if (flags & EMU_IF_TEMPHIGH)
  {
    // High EMU interrupt = Low temp limit
    if (lowCallback->callback != NULL)
    {
      lowCallback->callback(TEMPDRV_GetTemp(), TEMPDRV_LIMIT_LOW);
    }
    memset(lowCallback,0,sizeof(TEMPDRV_CallbackSet_t));
    EMU_IntClear(EMU_IFC_TEMPHIGH);
  }
  else if (flags & EMU_IF_TEMPLOW)
  {
    // Low EMU interrupt = high temp limit
    if (highCallback->callback != NULL)
    {
      highCallback->callback(TEMPDRV_GetTemp(), TEMPDRV_LIMIT_HIGH);
    }
    memset(highCallback,0,sizeof(TEMPDRV_CallbackSet_t));
    EMU_IntClear(EMU_IFC_TEMPLOW);
  }
  updateInterrupts();
}

/* Errata */
#ifdef _EFR_DEVICE
typedef enum ErrataState 
{
  ERRATA_LOW     = 0,
  ERRATA_MIDLOW  = 1,
  ERRATA_MIDHIGH = 2,
  ERRATA_HIGH    = 3
} ErrataState_t;
ErrataState_t errataState;
static void errataCallback(int8_t temp, TEMPDRV_LimitType_t limit);
static TEMPDRV_CallbackSet_t errataLowTemp[4] = {{NULL,0},
                                                 {errataCallback,155},
                                                 {errataCallback,140},
                                                 {errataCallback,129}};
static TEMPDRV_CallbackSet_t errataHighTemp[4] = {{errataCallback,150},
                                                  {errataCallback,135},
                                                  {errataCallback,124},
                                                  {NULL,0}};

/***************************************************************************//**
 * @brief Errata Callback- handles hysteresis temperature thresholds
 ******************************************************************************/
static void errataCallback(int8_t temp, TEMPDRV_LimitType_t limit)
{
  volatile uint32_t *reg;
  if (limit == TEMPDRV_LIMIT_HIGH)
  {
    errataState++;
    tempdrvHighCallbacks[0] = errataHighTemp[errataState];
    tempdrvLowCallbacks[0] = errataLowTemp[errataState];
  }
  else if (limit == TEMPDRV_LIMIT_LOW)
  {
    errataState--;
    tempdrvHighCallbacks[0] = errataHighTemp[errataState];
    tempdrvLowCallbacks[0] = errataLowTemp[errataState];
  }
  bool lock = false;
  if (EMU->LOCK == EMU_LOCK_LOCKKEY_LOCKED)
  {
    lock = true;
    EMU->LOCK = EMU_LOCK_LOCKKEY_UNLOCK;
  }
  reg = (volatile uint32_t *) 0x400E3190;
  *reg = 0xADE8;
  switch (errataState)
  {
    case ERRATA_LOW:
    case ERRATA_MIDLOW:
    case ERRATA_MIDHIGH:
      reg = (volatile uint32_t *) 0x400E3164;
      *reg |= 0x70;
      reg = (volatile uint32_t *) 0x400E319C;
      *reg |= 0x8;
      reg = (volatile uint32_t *) 0x400E403C;
      *reg &= ~(0xC00);
      *reg |= (errataState & 0x2) << 10;
      reg = (volatile uint32_t *) 0x400E319C;
      *reg &= ~(0x8);
      break;
    case ERRATA_HIGH:
    default:
      reg = (volatile uint32_t *) 0x400E3164;
      *reg &= ~(0x70);
      break;
  }
  reg = (volatile uint32_t *) 0x400E3190;
  *reg = 0;
  if (lock == true)
  {
    EMU->LOCK = EMU_LOCK_LOCKKEY_LOCK;
  }
}

/***************************************************************************//**
 * @brief Errata Initialization- sets first callbacks based on current temp
 ******************************************************************************/
static void errataInit()
{
  SYSTEM_ChipRevision_TypeDef rev;
  SYSTEM_ChipRevisionGet(&rev);

  if (rev.major == 0x01)
  {
    /* Rev A temp errata handling */
    if (EMU->TEMP > errataHighTemp[0].temp)
    {
      errataState = ERRATA_LOW;
    }
    else if (EMU->TEMP < errataLowTemp[3].temp)
    {
      errataState = ERRATA_HIGH;
    }
    else if (EMU->TEMP <= errataHighTemp[1].temp)
    {
      errataState = ERRATA_MIDHIGH;
    }
    else if (EMU->TEMP > errataHighTemp[1].temp)
    {
      errataState = ERRATA_MIDLOW;
    }
    tempdrvHighCallbacks[0] = errataHighTemp[errataState];
    tempdrvLowCallbacks[0] = errataLowTemp[errataState];
  }
}
#else // _EFR_DEVICE

#define errataInit()

#endif // _EFR_DEVICE

/* Internal Functions */
/***************************************************************************//**
 * @brief Find an empty spot for callback in set
 *
 * @param[in] set Callback set to search
 *
 * @return
 *    @ref index of empty space if found, -1 if none
 ******************************************************************************/
static int8_t findCallbackSpace(TEMPDRV_CallbackSet_t *set)
{
  uint8_t index;
  for (index = TEMPDRV_CUSTOM_CALLBACK_INDEX; index < TEMPDRV_CALLBACK_DEPTH; index++)
  {
    if (set[index].callback==NULL)
    {
      return index;
    }
  }
  // no empty space, return -1
  return -1;
}

/***************************************************************************//**
 * @brief Attempt to add a callback to the set, error if no space or invalid temp
 *
 * @param[in] set Callback set to add callback to
 *
 * @param[in] temp Temperature to register callback at
 *
 * @param[in] callback Callback function
 *
 * @return
 *    @ref ECODE_EMDRV_TEMPDRV_OK on success.
 ******************************************************************************/
static Ecode_t addCallback(TEMPDRV_CallbackSet_t *set, 
                           int8_t temp, 
                           TEMPDRV_Callback_t callback)
{
  int8_t index = findCallbackSpace(set);
  if (index < 0)
  {
    return ECODE_EMDRV_TEMPDRV_NO_SPACE;
  }
  Ecode_t status = convertTEMPtoEMU(temp, &set[index].temp);
  if (status!=ECODE_EMDRV_TEMPDRV_OK)
  {
    return status;
  }
  set[index].callback = callback;
  updateInterrupts();
  return ECODE_EMDRV_TEMPDRV_OK;
}

/***************************************************************************//**
 * @brief Remove a callback from the set
 *
 * @param[in] set Callback set to remove callback from
 *
 * @param[in] callback Callback function
 *
 * @return
 *    @ref True on success.
 ******************************************************************************/
static bool removeCallback(TEMPDRV_CallbackSet_t *set, 
                           TEMPDRV_Callback_t callback)
{
  bool found = false;
  uint8_t index;
  for (index = TEMPDRV_CUSTOM_CALLBACK_INDEX; index < TEMPDRV_CALLBACK_DEPTH; index++)
  {
    if (set[index].callback == callback)
    {
      set[index].callback = NULL;
      set[index].temp = 0;
      found = true;
    }
  }
  updateInterrupts();
  return found;
}

/***************************************************************************//**
 * @brief Check if another callback has registered the same temperature
 *
 * @param[in] set Callback set to search
 *
 * @param[in] temp Temperature to match
 *
 * @return
 *    @ref True if duplicate is found.
 ******************************************************************************/
static bool checkForDuplicates(TEMPDRV_CallbackSet_t *set, int8_t temp)
{
  uint8_t index;
  uint8_t emu;
  convertTEMPtoEMU(temp, &emu);
  for (index = TEMPDRV_CUSTOM_CALLBACK_INDEX; index < TEMPDRV_CALLBACK_DEPTH; index++)
  {
    // filter out only entries with valid callbacks
    if (set[index].callback!=NULL)
    {
      // if duplicate temperature, return true
      if (set[index].temp == emu)
      {
        return true;
      }
    }
  }
  // return false if no duplicate temperatures found
  return false;
}

/***************************************************************************//**
 * @brief Convert EMU value to degrees Celsius
 *
 * @param[in] emu EMU value to convert
 *
 * @param[in] temp Pointer to store temperature
 *
 * @return
 *    @ref ECODE_EMDRV_TEMPDRV_OK on success.
 ******************************************************************************/
static Ecode_t convertEMUtoTEMP(uint8_t emu, int8_t *temp)
{
  int32_t res = (int32_t) calibrationTEMP - ((emu * 8) / 5);
  // Cap conversion results at int8_t bounds and report error
  if (res < -128)
  {
    *temp = -128;
    return ECODE_EMDRV_TEMPDRV_TEMP_UNDER;
  }
  else if (res > 127)
  {
    *temp = 127;
    return ECODE_EMDRV_TEMPDRV_TEMP_OVER;
  }
  *temp = (int8_t) res;
  return ECODE_EMDRV_TEMPDRV_OK;
}

/***************************************************************************//**
 * @brief Convert degrees Celsius to EMU value
 *
 * @param[in] temp Temperature to convert
 *
 * @param[in] emu Pointer to store EMU value
 *
 * @return
 *    @ref ECODE_EMDRV_TEMPDRV_OK on success.
 ******************************************************************************/
static Ecode_t convertTEMPtoEMU(int8_t temp, uint8_t *emu)
{
  int32_t res = (int32_t) calibrationEMU -  ((temp * 5) >> 3);
  // Cap conversion results at uint8_t bounds and report error
  if (res>255)
  {
    *emu = 255;
    return ECODE_EMDRV_TEMPDRV_TEMP_UNDER;
  }
  else if (res < 0)
  {
    *emu = 0;
    return ECODE_EMDRV_TEMPDRV_TEMP_OVER;
  }
  *emu = (int8_t) res;
  return ECODE_EMDRV_TEMPDRV_OK;
}

/***************************************************************************//**
 * @brief Turn off interrupts
 ******************************************************************************/
static void disableInterrupts()
{
  EMU_IntClear (EMU_IFC_TEMPLOW | EMU_IFC_TEMPHIGH);
  EMU_IntDisable (EMU_IFC_TEMPLOW | EMU_IFC_TEMPHIGH);
}

/***************************************************************************//**
 * @brief Update interrupts based on active callbacks
 ******************************************************************************/
static void updateInterrupts()
{
  // Find lowest temperature active high callback
  uint8_t index;
  for (index = 0; index < TEMPDRV_CALLBACK_DEPTH; index++)
  {
    // filter out only entries with valid callbacks
    if (tempdrvHighCallbacks[index].callback!=NULL)
    {
      if (highCallback->callback == NULL)
      {
        highCallback = &tempdrvHighCallbacks[index];
      }
      else
      {
        if (tempdrvHighCallbacks[index].temp>highCallback->temp)
        {
          highCallback = &tempdrvHighCallbacks[index];
        }
      }
    }
  }
  // If active callback, set and enable interrupt
  if (highCallback->callback!=NULL)
  {
    // EMU and TEMP are inversely proportional
    EMU->TEMPLIMITS &= ~_EMU_TEMPLIMITS_TEMPLOW_MASK;
    EMU->TEMPLIMITS |= highCallback->temp << _EMU_TEMPLIMITS_TEMPLOW_SHIFT;
    EMU_IntEnable (EMU_IEN_TEMPLOW);
  }
  else
  {
    EMU_IntDisable (EMU_IEN_TEMPLOW);
  }

  // Find highest temperature active low callback
  for (index = 0; index < TEMPDRV_CALLBACK_DEPTH; index++)
  {
    // filter out only entries with valid callbacks
    if (tempdrvLowCallbacks[index].callback!=NULL)
    {
      if (lowCallback->callback == NULL)
      {
        lowCallback = &tempdrvLowCallbacks[index];
      }
      else
      {
        if (tempdrvLowCallbacks[index].temp<lowCallback->temp)
        {
          lowCallback = &tempdrvLowCallbacks[index];
        }
      }
    }
  }
  // If active callback, set and enable interrupt
  if (lowCallback->callback!=NULL)
  {
    // EMU and TEMP are inversely proportional
    EMU->TEMPLIMITS &= ~_EMU_TEMPLIMITS_TEMPHIGH_MASK;
    EMU->TEMPLIMITS |= lowCallback->temp << _EMU_TEMPLIMITS_TEMPHIGH_SHIFT;
    EMU_IntEnable (EMU_IEN_TEMPHIGH);
  }
  else
  {
    EMU_IntDisable (EMU_IEN_TEMPHIGH);
  }
}

/* Official API */
/***************************************************************************//**
 * @brief
 *    Initialize the TEMP driver.
 *
 * @return
 *    @ref ECODE_EMDRV_TEMPDRV_OK on success.
 ******************************************************************************/
Ecode_t TEMPDRV_Init()
{
  uint8_t *DItemp, *DIemu;

  // Flag up
  TEMPDRV_InitState = true;

  // reset stack state by erasing callbacks
  memset (tempdrvHighCallbacks, 0, sizeof (TEMPDRV_CallbackSet_t) * TEMPDRV_CALLBACK_DEPTH);
  memset (tempdrvLowCallbacks, 0, sizeof (TEMPDRV_CallbackSet_t) * TEMPDRV_CALLBACK_DEPTH);
  highCallback = &nullCallback;
  lowCallback = &nullCallback;

  // Retrieve calibration data from DI page
  DItemp = (uint8_t *) 0x0FE0810B;
  DIemu = (uint8_t *) 0x0FE08204;
  if ((*DItemp == 0xFF ) || (*DIemu == 0xFF))
  {
    // Missing DI page calibration data, substitute fixed values
    DItemp = &fallbackTEMP;
    DIemu = &fallbackEMU;

  }

  // calculate conversion offsets. Based on assumed slope of 5/8
  calibrationEMU = (*DIemu) + ((5*(*DItemp))>>3);
  calibrationTEMP = (*DItemp) + (8*(*DIemu)/5);

  errataInit();

  disableInterrupts();
  NVIC_ClearPendingIRQ(EMU_IRQn);
  NVIC_EnableIRQ(EMU_IRQn);
  updateInterrupts();

  return ECODE_EMDRV_TEMPDRV_OK;
}

/***************************************************************************//**
 * @brief
 *    De-initialize the TEMP driver.
 *
 * @return
 *    @ref ECODE_EMDRV_TEMPDRV_OK on success.
 ******************************************************************************/
Ecode_t TEMPDRV_DeInit()
{
  TEMPDRV_InitState = false;
  NVIC_DisableIRQ(EMU_IRQn);
  NVIC_ClearPendingIRQ(EMU_IRQn);
  disableInterrupts();
  memset (tempdrvHighCallbacks, 0, sizeof (TEMPDRV_CallbackSet_t) * TEMPDRV_CALLBACK_DEPTH);
  memset (tempdrvLowCallbacks, 0, sizeof (TEMPDRV_CallbackSet_t) * TEMPDRV_CALLBACK_DEPTH);
  return ECODE_EMDRV_TEMPDRV_OK;
}

/***************************************************************************//**
 * @brief
 *    Enable the TEMP driver.
 *
 * @param[in] enable Boolean to enable or disable the TEMP driver
 *
 * @return
 *    @ref ECODE_EMDRV_TEMPDRV_OK on success.
 ******************************************************************************/
Ecode_t TEMPDRV_Enable(bool enable)
{
  if (TEMPDRV_EnableState != enable)
  {
    TEMPDRV_EnableState = enable;
    if (enable)
    {
      updateInterrupts();
    }
    else
    {
      disableInterrupts();
    }
  }
  return ECODE_EMDRV_TEMPDRV_OK;
}

/***************************************************************************//**
 * @brief
 *    Get the number of active callbacks for a limit.
 *
 * @param[in] limit Limit type, refer to @ref TEMPDRV_LimitType_t.
 *
 * @return
 *    Number of active callbacks
 ******************************************************************************/
uint8_t TEMPDRV_GetActiveCallbacks(TEMPDRV_LimitType_t limit)
{
  TEMPDRV_CallbackSet_t *set;

  if (limit == TEMPDRV_LIMIT_HIGH)
  {
    // Define callback set
    set = tempdrvHighCallbacks;
  }
  else if (limit == TEMPDRV_LIMIT_LOW)
  {
    // Define callback set
    set = tempdrvLowCallbacks;
  }
  else
  {
    // Invalid limit
    return 0;
  }
  uint8_t index, count=0;
  for (index = TEMPDRV_CUSTOM_CALLBACK_INDEX; index < TEMPDRV_CALLBACK_DEPTH; index++)
  {
    // filter out only entries with valid callbacks
    if (set[index].callback!=NULL)
    {
      count++;
    }
  }
  return count;
}

/***************************************************************************//**
 * @brief
 *    Get the current temperature.
 *
 * @return
 *    Current temperature in degrees Celsius.
 ******************************************************************************/
int8_t TEMPDRV_GetTemp()
{
  int8_t ret;
  convertEMUtoTEMP(EMU->TEMP,&ret);
  return ret;
}

/***************************************************************************//**
 * @brief
 *    Register a callback in the TEMP driver
 *
 * @param[in] temp Temperature to trigger on.
 *
 * @param[in] limit Limit type, refer to @ref TEMPDRV_LimitType_t.
 * 
 * @param[in] callback Callback to call when temperature threshold reached.
 *
 * @return
 *    @ref ECODE_EMDRV_TEMPDRV_OK on success.
 ******************************************************************************/
Ecode_t TEMPDRV_RegisterCallback(int8_t temp, 
                                 TEMPDRV_LimitType_t limit, 
                                 TEMPDRV_Callback_t callback)
{
  TEMPDRV_CallbackSet_t *set;
  if (TEMPDRV_InitState == false)
  {
    return ECODE_EMDRV_TEMPDRV_NO_INIT;
  }
  // cannot register null callback
  if (callback == NULL)
  {
    return ECODE_EMDRV_TEMPDRV_PARAM_ERROR;
  }
  if (limit == TEMPDRV_LIMIT_HIGH)
  {
    // current temperature is already higher than requested temperature
    if (TEMPDRV_GetTemp() > temp)
    {
      return ECODE_EMDRV_TEMPDRV_BAD_LIMIT;
    }
    // Define callback set
    set = tempdrvHighCallbacks;
  }
  else if (limit == TEMPDRV_LIMIT_LOW)
  {
    // current temperature is already lower than requested temperature
    if (TEMPDRV_GetTemp() < temp)
    {
      return ECODE_EMDRV_TEMPDRV_BAD_LIMIT;
    }
    // Define callback set
    set = tempdrvLowCallbacks;
  }
  else
  {
    // Invalid limit
    return ECODE_EMDRV_TEMPDRV_PARAM_ERROR;
  }

  // Cannot register duplicate temperature callback
  if (checkForDuplicates(set, temp) == true)
  {
    return ECODE_EMDRV_TEMPDRV_DUP_TEMP;
  }

  return addCallback(set, temp, callback);
}

/***************************************************************************//**
 * @brief
 *    Unregister a callback in the TEMP driver.
 *
 * @param[in] callback Callback to unregister.
 *
 * @return
 *    @ref ECODE_EMDRV_TEMPDRV_OK on success.
 ******************************************************************************/
Ecode_t TEMPDRV_UnregisterCallback(TEMPDRV_Callback_t callback)
{
  // cannot register null callback
  if (callback == NULL)
  {
    return ECODE_EMDRV_TEMPDRV_PARAM_ERROR;
  }
  if (removeCallback(tempdrvHighCallbacks,callback) == false &&
      removeCallback(tempdrvLowCallbacks,callback) == false)
  {
    return ECODE_EMDRV_TEMPDRV_NO_CALLBACK;
  }
  return ECODE_EMDRV_TEMPDRV_OK;
}

/******** THE REST OF THE FILE IS DOCUMENTATION ONLY !**********************//**
 * @{

@page tempdrv_doc TEMPDRV temperature sensor driver

  The source files for the TEMP driver library resides in the
  emdrv/tempdrv folder, and are named tempdrv.c and tempdrv.h.

  @li @ref tempdrv_intro
  @li @ref tempdrv_conf
  @li @ref tempdrv_api

@n @section tempdrv_intro Introduction

  The TEMP driver supports temperature capabilities on the EFR32 EMU peripheral,
  but not the ADC peripheral. TEMPDRV provides an interface to course grained, 
  low power interrupts on temperature change.

@n @section tempdrv_conf Configuration Options

  Some properties of the TEMPDRV driver are compile-time configurable. These
  properties are set in a file named @ref tempdrv_config.h. A template for this
  file, containing default values, resides in the emdrv/config folder.
  To configure TEMPDRV for your application, provide your own configuration file. 
  These are the available configuration parameters with default values defined.
  @verbatim

  // Callback table depth (for high and low callbacks each)
  #define TEMPDRV_CALLBACK_DEPTH 5

  // Allow temperature sensor to wake the device up from EM4
  #define TEMPDRV_EM4WAKEUP false

  // Allow TEMPDRV to define the EMU_IRQ_Handler. Enable if EMU_IRQ_Handler is 
  // defined elsewhere.
  #define EMU_CUSTOM_IRQ_HANDLER false
  @endverbatim

  Callback table depth determines the number of concurrent callbacks that can be
  registered at a single time. The depth applies to each limit, so depth of 5
  allows up to 5 high and 5 low callbacks to be registered.
  There are no run-time configuration options for TEMPDRV. 

@n @section tempdrv_api The API

  This section contain brief descriptions of the functions in the API. You will
  find detailed information on input and output parameters and return values by
  clicking on the hyperlinked function names. Most functions return an error
  code, @ref ECODE_EMDRV_TEMPDRV_OK is returned on success,
  see @ref ecode.h and @ref tempdrv.h for other error codes.

  Your application code must include one header file: @em tempdrv.h.

  @ref TEMPDRV_Init(), @ref TEMPDRV_DeInit() @n
    These functions initializes or deinitializes the TEMPDRV driver. This will 
    erase any registered callbacks and disabled all interrupts. Typically
    @htmlonly TEMPDRV_Init() @endhtmlonly is called once in your startup code.

  @ref TEMPDRV_Enable() @n
    Enable or disable the temperature driver without losing any registered
    callbacks.

  @ref TEMPDRV_GetTemp() @n
    Get the current temperature in degrees Celsius. This measurement is based on
    a conversion from the EMU temperature sensor and calibration data that is
    stored in the DI page.

  @ref TEMPDRV_RegisterCallback(), @ref TEMPDRV_UnregisterCallback() @n
    Callbacks can be registered for rising or falling thresholds and will called
    as soon as the temperature matches the specified threshold. Multiple 
    callbacks at the same temperature are not permitted, nor are mismatches
    between temperature and limit (e. g temperature is lower than current but
    the limit is set to high). Additionally, unregistering a callback will remove 
    all entries of matching callbacks.

@n @section tempdrv_example Example
  @verbatim
#include "tempdrv.h"

boolean flag = false;

void callback(int8_t temp, TEMPDRV_LimitType_t limit)
{
  flag = true;
}

int main(void)
{
  TEMPDRV_Init();

  // Register a callback at 10 degrees above current temperature
  TEMPDRV_RegisterCallback(TEMPDRV_GetTemp()+10, TEMPDRV_LIMIT_HIGH, callback);

  while (flag==false) {};
}
  @endverbatim

 * @}**************************************************************************/
