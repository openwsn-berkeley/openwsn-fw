/**
    \brief Definition of the nrf52480 ADC driver.
    \author Frank Senf <frank.senf@imms.de>, July 2018.
*/


#include "sdk/components/boards/boards.h"
#include "nrf_drv_saadc.h"
#include "nrf_temp.h"

#include "adc_sensor.h"


//=========================== defines =========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototype =======================================

static void saadc_callback(nrf_drv_saadc_evt_t const* p_event);


//=========================== public ==========================================

bool adc_sens_init(void) {
    ret_code_t retVal;

    nrfx_saadc_config_t saadc_config = NRFX_SAADC_DEFAULT_CONFIG;
    nrf_saadc_channel_config_t channel_config = NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(SAADC_CH_PSELP_PSELP_VDD);

    retVal = nrf_drv_saadc_init(&saadc_config, saadc_callback);
    if (NRFX_SUCCESS != retVal) {
        return false;
    }

    retVal = nrf_drv_saadc_channel_init(0, &channel_config);
    if (NRFX_SUCCESS != retVal) {
        nrf_drv_saadc_uninit();
        return false;
    }

    retVal = nrfx_saadc_calibrate_offset();
    if (NRFX_SUCCESS != retVal) {
        while (nrfx_saadc_is_busy()) {}
    }

    nrf_temp_init();

    return true;
}

uint16_t adc_sens_read_battery(void) {

    nrf_saadc_value_t value = 0;

    nrf_drv_saadc_sample_convert(0, &value);

    return (uint16_t) value;
}

float adc_sens_convert_battery(uint16_t raw) {

    float converted = raw;

    converted *= 600;	// 0.6V internal reference
    converted /= 1024;	// 10 bit resolution
    converted *= 6;	// 1/6 prescaling
    // in volts
    converted /= 1000;

    return converted;
}

uint16_t adc_sens_read_temperature(void) {

    int32_t cpu_temp_raw;

    NRF_TEMP->TASKS_START = 1;
    while (NRF_TEMP->EVENTS_DATARDY==0) {}
    NRF_TEMP->EVENTS_DATARDY = 0;
    cpu_temp_raw = nrf_temp_read();
    NRF_TEMP->TASKS_STOP = 1;

    return cpu_temp_raw;
}

float adc_sens_convert_temperature(uint16_t cpu_temp_raw) {

    float cpu_temp = cpu_temp_raw;
    cpu_temp /= 4;

    return cpu_temp;
}


//=========================== private =========================================

static void saadc_callback(nrf_drv_saadc_evt_t const* p_event) {
}
