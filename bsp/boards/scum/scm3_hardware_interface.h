void analog_scan_chain_write(unsigned int* scan_bits);
void analog_scan_chain_load(void);
void initialize_2M_DAC(void);
void set_2M_RC_frequency(int coarse1, int coarse2, int coarse3, int fine, int superfine);
void read_counters(unsigned int* count_2M, unsigned int* count_LC, unsigned int* count_32k);
unsigned int flip_lsb8(unsigned int in);
void update_PN31_byte(unsigned int* current_lfsr);
void TX_load_PN_data(unsigned int num_bytes);
void TX_load_counter_data(unsigned int num_bytes);

unsigned char flipChar(unsigned char b);

void init_ldo_control(void);

void set_sys_clk_secondary_freq(unsigned int coarse, unsigned int fine);

void set_asc_bit(unsigned int position);
void clear_asc_bit(unsigned int position);
void GPO_control(unsigned char row1, unsigned char row2, unsigned char row3, unsigned char row4);
void GPI_control(unsigned char row1, unsigned char row2, unsigned char row3, unsigned char row4);
void GPO_enables(unsigned int mask);
void GPI_enables(unsigned int mask);

void set_asc_bit_FPGA(unsigned int position);
void clear_asc_bit_FPGA(unsigned int position);
void GPO_control_FPGA(unsigned char row1, unsigned char row2, unsigned char row3, unsigned char row4);
void GPI_control_FPGA(unsigned char row1, unsigned char row2, unsigned char row3, unsigned char row4);

void analog_scan_chain_write_3B_fromFPGA(unsigned int* scan_bits);
void analog_scan_chain_load_3B_fromFPGA(void);

void set_IF_LDO_voltage(int code);
void radio_init_rx_MF(void);
void radio_init_tx(void);
void set_IF_clock_frequency(int coarse, int fine, int high_range);
void LC_FREQCHANGE_ASC(int coarse, int mid, int fine);
void LC_monotonic_ASC(int LC_code);
void radio_init_divider(unsigned int div_value);
void radio_enable_LO(void);
void radio_disable_all(void);

void set_IF_stg3gm_ASC(unsigned int Igm, unsigned int Qgm);
void set_IF_gain_ASC(unsigned int Igain, unsigned int Qgain);
void set_IF_comparator_trim_I(unsigned int ptrim, unsigned int ntrim);
void set_IF_comparator_trim_Q(unsigned int ptrim, unsigned int ntrim);

void build_channel_table(unsigned int channel_11_LC_code);
unsigned int build_RX_channel_table(unsigned int channel_11_LC_code);
void build_TX_channel_table(unsigned int channel_11_LC_code, unsigned int count_LC_RX_ch11);

unsigned int read_IF_estimate(void);