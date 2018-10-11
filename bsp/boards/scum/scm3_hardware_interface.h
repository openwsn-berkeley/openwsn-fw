unsigned int flip_lsb8(unsigned int in);
void analog_scan_chain_write(unsigned int* scan_bits);
void analog_scan_chain_load(void);

int approximate_2M_counter(int new_fine_code, int new_superfine_code);
void set_2M_RC_frequency(int coarse1, int coarse2, int coarse3, int fine, int superfine);
void print_2MHz_DAC(void);
void initialize_2M_DAC(void);

int valid_2M_read(int counter);
int is_2M_within_cal_window(void);

int cal_2M_RC(void);
void LC_FREQCHANGE(char coarse, char mid, char fine);

bool is_2M_high(void);
void execute_func( void (*f) (void));
void disable_counters(void);
void reset_counters(void);
void enable_counters(void);


void initialize_ASC(void);
void read_counters(unsigned int* count_2M, unsigned int* count_LC, unsigned int* count_32k, unsigned int* count_ringDiv8);
void read_2M_counter(unsigned int* count_2M);

void add_sample_to_buffer(int new_sample, int offset);
double compute_2M_rolling_average(void);

void update_PN31_byte(unsigned int* current_lfsr);
void TX_load_PN_data(unsigned int num_bytes);
void TX_load_counter_data(unsigned int num_bytes);
