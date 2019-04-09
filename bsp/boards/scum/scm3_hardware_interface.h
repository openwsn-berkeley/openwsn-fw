void analog_scan_chain_write(unsigned int* scan_bits);
void analog_scan_chain_load(void);
void initialize_2M_DAC(void);
void set_2M_RC_frequency(int coarse1, int coarse2, int coarse3, int fine, int superfine);
void read_counters(unsigned int* count_2M, unsigned int* count_LC, unsigned int* count_32k);
unsigned int flip_lsb8(unsigned int in);
void update_PN31_byte(unsigned int* current_lfsr);
void TX_load_PN_data(unsigned int num_bytes);
void TX_load_counter_data(unsigned int num_bytes);
