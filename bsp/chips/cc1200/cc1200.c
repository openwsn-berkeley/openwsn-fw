/**
 * Author: Pere Tuset (peretuset@openmote.com)
           Jonathan Muñon (jonathan.munoz@inria.fr)
 * Date:   May 2016
 * Description: CC1200-specific definition of the "radio" bsp module.
 */

#include <string.h>

#include "cc1200.h"
#include "cc1200_regs.h"
#include "cc1200_arch.h"

//=========================== defines =========================================

//=========================== variables =======================================

extern cc1200_rf_cfg_t cc1200_rf_cfg;

//=========================== prototypes ======================================

static uint8_t cc1200_strobe(uint8_t strobe);
static uint8_t cc1200_single_read(uint16_t address);
static void cc1200_burst_read(uint16_t address, uint8_t *data, uint8_t length);
static uint8_t cc1200_single_write(uint16_t address, uint8_t value);
static void cc1200_burst_write(uint16_t address, const uint8_t *data, uint8_t length);
static uint8_t cc1200_state(void);
static void cc1200_write_register_settings(const cc1200_register_settings_t* settings, uint16_t size);

//=========================== public ==========================================

/**
 * Initialize the CC1200.
 */
void cc1200_init(void) {
  // Initialize the arch driver
  cc1200_arch_init();

  cc1200_arch_gpio0_setup(false);
  cc1200_arch_gpio0_enable();

  cc1200_arch_gpio2_setup(false);
  cc1200_arch_gpio2_enable();

  cc1200_arch_gpio3_setup(false);
  cc1200_arch_gpio3_enable();

  cc1200_configure();
}

/**
 * Turn the CC1200 on.
 */
bool cc1200_on(void) {
  /* Don't turn on if we are on already */
  if(!(rf_flags & RF_ON)) {
    /* Wake-up procedure. Wait for GPIO0 to de-assert (CHIP_RDYn) */
    cc1200_arch_spi_select();
    // BUSYWAIT_UNTIL((cc1200_arch_gpio0_read_pin() == 0), RTIMER_SECOND / 100);
    // RF_ASSERT((cc1200_arch_gpio0_read_pin() == 0));
    cc1200_arch_spi_deselect();

    // rf_flags |= RF_ON;

    /* Radio is IDLE now, re-configure GPIO0 (modified inside off()) */
    // cc1200_single_write(CC1200_IOCFG0, GPIO0_IOCFG);

    /* Turn on RX */
    // idle_calibrate_rx();
  }

  return true;
}

/**
 * Turn the radio off.
 */
bool cc1200_off(void) {
  /* Don't turn off if we are off already */
  if (rf_flags & RF_ON) {
    cc1200_idle();

    cc1200_single_write(CC1200_IOCFG0, CC1200_IOCFG_RXFIFO_CHIP_RDY_N);

    cc1200_strobe(CC1200_SPWD);

    /* Clear all but the initialized flag */
    rf_flags = RF_INITIALIZED;
  }

  return true;
}

/**
 * Put the radio in IDLE mode.
 */
void cc1200_idle(void) {
  uint8_t state;

  state = cc1200_state();

  if (state == STATE_IDLE) {
    return;
  } else if(state == STATE_RX_FIFO_ERR) {
    cc1200_strobe(CC1200_SFRX);
  } else if(state == STATE_TX_FIFO_ERR) {
    cc1200_strobe(CC1200_SFTX);
  }

  cc1200_strobe(CC1200_SIDLE);
}

/**
 * Reset the radio.
 */
void cc1200_reset(void) {
  cc1200_arch_spi_select();

  cc1200_arch_spi_rw_byte(CC1200_SRES);
  // clock_delay(100);

  cc1200_arch_spi_deselect();
}

/**
 * Configure the radio.
 */
void cc1200_configure(void) {
    cc1200_write_register_settings(cc1200_rf_cfg.register_settings,
                                   cc1200_rf_cfg.size_of_register_settings);
}

//====================== private =========================

/**
 * Send a command strobe.
 */
static uint8_t cc1200_strobe(uint8_t strobe) {
  uint8_t ret;

  cc1200_arch_spi_select();
  ret = cc1200_arch_spi_rw_byte(strobe);
  cc1200_arch_spi_deselect();

  return ret;
}

/**
 * Read a single byte from the specified address.
 */
static uint8_t cc1200_single_read(uint16_t address) {
  uint8_t ret;

  cc1200_arch_spi_select();

  if (CC1200_IS_EXTENDED_ADDR(address)) {
    cc1200_arch_spi_rw_byte(CC1200_EXTENDED_READ_CMD);
    cc1200_arch_spi_rw_byte((uint8_t)address);
  } else {
    cc1200_arch_spi_rw_byte(address | CC1200_READ_BIT);
  }
  ret = cc1200_arch_spi_rw_byte(0);

  cc1200_arch_spi_deselect();

  return ret;
}

/**
 * Read a burst of bytes starting at the specified address.
 */
static void cc1200_burst_read(uint16_t address, uint8_t *data, uint8_t length) {
  cc1200_arch_spi_select();

  if (CC1200_IS_EXTENDED_ADDR(address)) {
    cc1200_arch_spi_rw_byte(CC1200_EXTENDED_BURST_READ_CMD);
    cc1200_arch_spi_rw_byte((uint8_t)address);
  } else {
    cc1200_arch_spi_rw_byte(address | CC1200_READ_BIT | CC1200_BURST_BIT);
  }
  cc1200_arch_spi_rw(data, NULL, length);

  cc1200_arch_spi_deselect();
}

/**
 * Write a single byte to the specified address.
 */
static uint8_t cc1200_single_write(uint16_t address, uint8_t value) {
  uint8_t ret;

  cc1200_arch_spi_select();

  if (CC1200_IS_EXTENDED_ADDR(address)) {
    cc1200_arch_spi_rw_byte(CC1200_EXTENDED_WRITE_CMD);
    cc1200_arch_spi_rw_byte((uint8_t)address);
  } else {
    cc1200_arch_spi_rw_byte(address | CC1200_WRITE_BIT);
  }
  ret = cc1200_arch_spi_rw_byte(value);

  cc1200_arch_spi_deselect();

  return ret;
}

/**
 * Write a burst of bytes starting at the specified address.
 */
static void cc1200_burst_write(uint16_t address, const uint8_t *data, uint8_t length) {
  cc1200_arch_spi_select();

  if (CC1200_IS_EXTENDED_ADDR(address)) {
    cc1200_arch_spi_rw_byte(CC1200_EXTENDED_BURST_WRITE_CMD);
    cc1200_arch_spi_rw_byte((uint8_t)address);
  } else {
    cc1200_arch_spi_rw_byte(address | CC1200_WRITE_BIT | CC1200_BURST_BIT);
  }

  cc1200_arch_spi_rw(NULL, data, length);

  cc1200_arch_spi_deselect();
}

static uint8_t cc1200_state(void) {
#if STATE_USES_MARC_STATE
  return single_read(CC1200_MARCSTATE) & 0x1f;
#else
  return cc1200_strobe(CC1200_SNOP) & 0x70;
#endif
}

/**
 * Write a list of register settings.
 */
void cc1200_write_register_settings(const cc1200_register_settings_t* settings, uint16_t size) {
  uint16_t i = size / sizeof(cc1200_register_settings_t);

  if (settings != NULL) {
    while (i--) {
      cc1200_single_write(settings->address, settings->value);
      settings++;
    }
  }
}

//====================== callbacks =======================

void cc1200_gpio0_interrupt(void) {

}

void cc1200_gpio2_interrupt(void) {

}

void cc1200_gpio3_interrupt(void) {

}
