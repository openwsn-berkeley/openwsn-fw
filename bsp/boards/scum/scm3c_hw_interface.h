#ifndef __SCM3C_HW_INTERFACE_H
#define __SCM3C_HW_INTERFACE_H

#include <stdint.h>

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

//==== admin
void scm3c_hw_interface_init(void);

//==== get functions
uint32_t scm3c_hw_interface_get_HF_CLOCK_fine(void);
uint32_t scm3c_hw_interface_get_HF_CLOCK_coarse(void);
uint32_t scm3c_hw_interface_get_RC2M_coarse(void);
uint32_t scm3c_hw_interface_get_RC2M_fine(void);
uint32_t scm3c_hw_interface_get_RC2M_superfine(void);
uint32_t scm3c_hw_interface_get_IF_clk_target(void);
uint32_t scm3c_hw_interface_get_IF_coarse(void);
uint32_t scm3c_hw_interface_get_IF_fine(void);

//===== set function

void scm3c_hw_interface_set_HF_CLOCK_fine(uint32_t value);
void scm3c_hw_interface_set_HF_CLOCK_coarse(uint32_t value);
void scm3c_hw_interface_set_RC2M_coarse(uint32_t value);
void scm3c_hw_interface_set_RC2M_fine(uint32_t value);
void scm3c_hw_interface_set_RC2M_superfine(uint32_t value);
void scm3c_hw_interface_set_IF_clk_target(uint32_t value);
void scm3c_hw_interface_set_IF_coarse(uint32_t value);
void scm3c_hw_interface_set_IF_fine(uint32_t value);

void scm3c_hw_interface_set_asc(uint32_t* asc_profile);

//==== from scm3c_hardware_interface.h
unsigned reverse(unsigned x);
unsigned int crc32c(unsigned char *message, unsigned int length);
unsigned char flipChar(unsigned char b);
void init_ldo_control(void);
unsigned int sram_test(unsigned int * baseAddress, unsigned int num_dwords);
void radio_init_rx_MF(void);
void radio_init_rx_ZCC(void);
void radio_init_tx(void);
void radio_init_divider(unsigned int div_value);
void radio_disable_all(void);
void GPO_control(uint8_t row1, uint8_t row2, uint8_t row3, uint8_t row4);
void GPI_control(uint8_t row1, uint8_t row2, uint8_t row3, uint8_t row4);
unsigned int read_IF_estimate(void);
unsigned int read_LQI(void);
unsigned int read_RSSI(void);
void set_IF_clock_frequency(int coarse, int fine, int high_range);
void GPO_enables(unsigned int mask);
void GPI_enables(unsigned int mask);
void set_IF_LDO_voltage(int code);
void set_VDDD_LDO_voltage(int code);
void set_AUX_LDO_voltage(int code);
void set_ALWAYSON_LDO_voltage(int code);
void radio_enable_PA(void);
void radio_enable_LO(void);
void radio_enable_RX(void);
void read_counters_3B(unsigned int* count_2M, unsigned int* count_LC, unsigned int* count_adc);
void packet_test_loop(unsigned int num_packets);
void set_IF_stg3gm_ASC(unsigned int Igm, unsigned int Qgm);
void set_IF_gain_ASC(unsigned int Igain, unsigned int Qgain);
void set_zcc_demod_threshold(unsigned int thresh);
void set_IF_comparator_trim_I(unsigned int ptrim, unsigned int ntrim);
void set_IF_comparator_trim_Q(unsigned int ptrim, unsigned int ntrim);
void set_IF_ZCC_clkdiv(unsigned int div_value);
void set_IF_ZCC_early(unsigned int early_value);
void initialize_mote(void);
void set_sys_clk_secondary_freq(unsigned int coarse, unsigned int fine);
unsigned int estimate_temperature_2M_32k(void);

//==== from scm3_hardware_interface.h

// Functions written by Brad, originally for 3B
void analog_scan_chain_write(void);
void analog_scan_chain_load(void);
void initialize_2M_DAC(void);
void set_2M_RC_frequency(int coarse1, int coarse2, int coarse3, int fine, int superfine);
void read_counters(unsigned int* count_2M, unsigned int* count_LC, unsigned int* count_32k);
unsigned int flip_lsb8(unsigned int in);
void update_PN31_byte(unsigned int* current_lfsr);
void set_asc_bit(unsigned int position);
void clear_asc_bit(unsigned int position);

//==== from bucket_o_functions.h

void enable_polyphase_ASC(void);
void disable_polyphase_ASC(void);
void disable_div_power_ASC(void);
void enable_div_power_ASC(void);
void ext_clk_ble_ASC(void);
void int_clk_ble_ASC(void);
void enable_1mhz_ble_ASC(void);
void disable_1mhz_ble_ASC(void);
void set_LC_current(unsigned int current);
void set_PA_supply(unsigned int code);
void set_LO_supply(unsigned int code, unsigned char panic);
void set_DIV_supply(unsigned int code, unsigned char panic);
void prescaler(int code);
void LC_monotonic(int LC_code);
void LC_FREQCHANGE(int coarse, int mid, int fine);
void divProgram(unsigned int div_ratio, unsigned int reset, unsigned int enable);

#endif
