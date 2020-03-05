// Open Radio
#include "radio_at86rf215.c"

//=========================== defines =========================================

// open radio functions callback types declaration
typedef void                (*radio_powerOn_cb_t)(void);
typedef void                (*radio_reset_cb_t)(void);
typedef void                (*radio_init_cb_t)(void);
typedef void                (*radio_change_size_cb_t)(uint16_t* size);
typedef void                (*radio_change_modulation_cb_t)(registerSetting_t * mod);
typedef void                (*radio_setStartFrameCb_cb_t)(radio_capture_cbt cb);
typedef void                (*radio_setEndFrameCb_cb_t)(radio_capture_cbt cb);
typedef void                (*radio_setFrequency_cb_t)(uint16_t channel, radio_freq_t tx_or_rx);
typedef void                (*radio_rfOn_cb_t)(void);
typedef void                (*radio_rfOff_cb_t)(void);
typedef void                (*radio_getFrequencyOffset_cb_t)(void);
typedef void                (*radio_loadPacket_cb_t)(uint8_t* packet, uint16_t len);
typedef radio_state_t       (*radio_getState_cb_t)(void);
typedef void                (*radio_txEnable_cb_t)(void);
typedef void                (*radio_txNow_cb_t)(void);
typedef void                (*radio_rxEnable_cb_t)(void);
typedef void                (*radio_rxNow_cb_t)(void);
typedef void                (*radio_getReceivedFrame_cb_t)(
                                                    uint8_t* bufRead,
                                                    uint16_t* lenRead,
                                                    uint16_t  maxBufLen,
                                                    int8_t*  rssi,
                                                    uint8_t* lqi,
                                                    bool*    crc
                                                    );

// the template for radio function callbacks
// inspired from release FW_807 bsp/common/openradio.h
typedef struct {
    radio_powerOn_cb_t                  radio_powerOn;
    radio_reset_cb_t                    radio_reset;
    radio_init_cb_t                     radio_init;
    radio_change_size_cb_t              radio_change_size;
    radio_change_modulation_cb_t        radio_change_modulation;
    radio_setStartFrameCb_cb_t          radio_setStartFrameCb;
    radio_setEndFrameCb_cb_t            radio_setEndFrameCb;
    radio_setFrequency_cb_t             radio_setFrequency;
    radio_rfOn_cb_t                     radio_rfOn;
    radio_rfOff_cb_t                    radio_rfOff;
    radio_getFrequencyOffset_cb_t       radio_getFrequencyOffset;
    radio_loadPacket_cb_t               radio_loadPacket;
    radio_getState_cb_t                 radio_getState;
    radio_txEnable_cb_t                 radio_txEnable;
    radio_txNow_cb_t                    radio_txNow;
    radio_rxEnable_cb_t                 radio_rxEnable;
    radio_rxNow_cb_t                    radio_rxNow;
    radio_getReceivedFrame_cb_t         radio_getReceivedFrame;
} radio_functions_t;

// This needs to be shared by all radio, not sure if it is generic enough but will leave it here now. 
typedef struct {
    radio_capture_cbt           startFrame_cb;
    radio_capture_cbt           endFrame_cb;
    radio_state_t               state;
    uint8_t                     rf09_isr;
    uint8_t                     rf24_isr;
    uint8_t                     bb0_isr;
    uint8_t                     bb1_isr;
} radio_vars_t;

radio_vars_t radio_vars;

// global radio selection, will use the slowest by default at initialization. 
uint8_t SELECTED_RADIO      =       FSK_OPTION1_FEC;

//=========================== variables =======================================

//function call back matrix
radio_functions_t dyn_funcs [MAX_RADIOS];

// ================ Bootstrapping ==========

// initializing the lookup table for radio function callbacks
void radio_bootstrap ()
{   
    // FSK_OPTION1_FEC
    dyn_funcs [FSK_OPTION1_FEC].radio_powerOn               =   radio_powerOn_at86rf215; 
    dyn_funcs [FSK_OPTION1_FEC].radio_reset                 =   radio_reset_at86rf215; 
    dyn_funcs [FSK_OPTION1_FEC].radio_init                  =   radio_init_at86rf215; 
    dyn_funcs [FSK_OPTION1_FEC].radio_change_size           =   radio_change_size_at86rf215; 
    dyn_funcs [FSK_OPTION1_FEC].radio_change_modulation     =   radio_change_modulation_at86rf215; 
    dyn_funcs [FSK_OPTION1_FEC].radio_setStartFrameCb       =   radio_setStartFrameCb_at86rf215; 
    dyn_funcs [FSK_OPTION1_FEC].radio_setEndFrameCb         =   radio_setEndFrameCb_at86rf215; 
    dyn_funcs [FSK_OPTION1_FEC].radio_setFrequency          =   radio_setFrequency_at86rf215; 
    dyn_funcs [FSK_OPTION1_FEC].radio_rfOn                  =   radio_rfOn_at86rf215; 
    dyn_funcs [FSK_OPTION1_FEC].radio_rfOff                 =   radio_rfOff_at86rf215; 
    dyn_funcs [FSK_OPTION1_FEC].radio_getFrequencyOffset    =   radio_getFrequencyOffset_at86rf215; 
    dyn_funcs [FSK_OPTION1_FEC].radio_loadPacket            =   radio_loadPacket_at86rf215; 
    dyn_funcs [FSK_OPTION1_FEC].radio_getState              =   radio_getState_at86rf215; 
    dyn_funcs [FSK_OPTION1_FEC].radio_txEnable              =   radio_txEnable_at86rf215; 
    dyn_funcs [FSK_OPTION1_FEC].radio_txNow                 =   radio_txNow_at86rf215; 
    dyn_funcs [FSK_OPTION1_FEC].radio_rxEnable              =   radio_rxEnable_at86rf215; 
    dyn_funcs [FSK_OPTION1_FEC].radio_rxNow                 =   radio_rxNow_at86rf215; 
    dyn_funcs [FSK_OPTION1_FEC].radio_getReceivedFrame      =   radio_getReceivedFrame_at86rf215; 

    //OFDM_OPTION_1_MCS0
    dyn_funcs [OFDM_OPTION_1_MCS0].radio_powerOn               =   radio_powerOn_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS0].radio_reset                 =   radio_reset_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS0].radio_init                  =   radio_init_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS0].radio_change_size           =   radio_change_size_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS0].radio_change_modulation     =   radio_change_modulation_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS0].radio_setStartFrameCb       =   radio_setStartFrameCb_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS0].radio_setEndFrameCb         =   radio_setEndFrameCb_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS0].radio_setFrequency          =   radio_setFrequency_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS0].radio_rfOn                  =   radio_rfOn_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS0].radio_rfOff                 =   radio_rfOff_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS0].radio_getFrequencyOffset    =   radio_getFrequencyOffset_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS0].radio_loadPacket            =   radio_loadPacket_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS0].radio_getState              =   radio_getState_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS0].radio_txEnable              =   radio_txEnable_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS0].radio_txNow                 =   radio_txNow_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS0].radio_rxEnable              =   radio_rxEnable_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS0].radio_rxNow                 =   radio_rxNow_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS0].radio_getReceivedFrame      =   radio_getReceivedFrame_at86rf215; 
    
    //OFDM_OPTION_1_MCS1
    dyn_funcs [OFDM_OPTION_1_MCS1].radio_powerOn               =   radio_powerOn_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS1].radio_reset                 =   radio_reset_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS1].radio_init                  =   radio_init_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS1].radio_change_size           =   radio_change_size_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS1].radio_change_modulation     =   radio_change_modulation_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS1].radio_setStartFrameCb       =   radio_setStartFrameCb_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS1].radio_setEndFrameCb         =   radio_setEndFrameCb_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS1].radio_setFrequency          =   radio_setFrequency_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS1].radio_rfOn                  =   radio_rfOn_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS1].radio_rfOff                 =   radio_rfOff_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS1].radio_getFrequencyOffset    =   radio_getFrequencyOffset_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS1].radio_loadPacket            =   radio_loadPacket_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS1].radio_getState              =   radio_getState_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS1].radio_txEnable              =   radio_txEnable_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS1].radio_txNow                 =   radio_txNow_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS1].radio_rxEnable              =   radio_rxEnable_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS1].radio_rxNow                 =   radio_rxNow_at86rf215; 
    dyn_funcs [OFDM_OPTION_1_MCS1].radio_getReceivedFrame      =   radio_getReceivedFrame_at86rf215;
}


//=========================== public ==========================================

static void radio_read_isr(void);
static void radio_clear_isr(void);

//===== admin
void radio_select (uint8_t radio){
    SELECTED_RADIO = radio;
}

void radio_powerOn(void) {
    dyn_funcs [SELECTED_RADIO].radio_powerOn();
}

void radio_reset(void) {
    dyn_funcs [SELECTED_RADIO].radio_reset();
}

void radio_init (void) {
    dyn_funcs [SELECTED_RADIO].radio_init();
}

void radio_change_size (uint16_t* size){
    dyn_funcs [SELECTED_RADIO].radio_change_size(size);
}

void radio_change_modulation (registerSetting_t * mod){
    dyn_funcs [SELECTED_RADIO].radio_change_modulation(mod);
}

void radio_setStartFrameCb (radio_capture_cbt cb) {
    dyn_funcs [SELECTED_RADIO].radio_setStartFrameCb(cb);
}

void radio_setEndFrameCb (radio_capture_cbt cb) {
    dyn_funcs [SELECTED_RADIO].radio_setEndFrameCb(cb);
}

//===== RF admin

void radio_setFrequency (uint16_t channel, radio_freq_t tx_or_rx) {
    dyn_funcs [SELECTED_RADIO].radio_setFrequency(channel, radio_freq_t tx_or_rx);
}

void radio_rfOn (void) {
    dyn_funcs [SELECTED_RADIO].radio_rfOn();
}

void radio_rfOff (void) {
    dyn_funcs [SELECTED_RADIO].radio_rfOff();
}

int8_t radio_getFrequencyOffset (void){
    return dyn_funcs [SELECTED_RADIO].radio_getFrequencyOffset();
}

//===== TX

void radio_loadPacket (uint8_t* packet, uint16_t len) {
    dyn_funcs [SELECTED_RADIO].radio_getFrequencyOffset(packet, len);
}

radio_state_t radio_getState (void){
    return dyn_funcs [SELECTED_RADIO].radio_getState();
}

void radio_txEnable (void) {
    dyn_funcs [SELECTED_RADIO].radio_txEnable();
}

void radio_txNow (void) {
    dyn_funcs [SELECTED_RADIO].radio_txNow();
}

//===== RX

void radio_rxEnable (void) {
    dyn_funcs [SELECTED_RADIO].radio_rxEnable();
}

void radio_rxNow (void) {
    dyn_funcs [SELECTED_RADIO].radio_rxNow();
}

void radio_getReceivedFrame (
    uint8_t* bufRead,
    uint16_t* lenRead,
    uint16_t  maxBufLen,
    int8_t*  rssi,
    uint8_t* lqi,
    bool*    crc
) {

    dyn_funcs [SELECTED_RADIO].radio_getReceivedFrame (bufRead, lenRead, maxBufLen, rssi, lqi, crc );
}

//=========================== private =========================================
// Will figure that out later

void radio_read_isr(void){
    uint8_t flags[4];
    at86rf215_read_isr(flags, ATMEL_FREQUENCY_TYPE);

    radio_vars.rf09_isr = flags[0];
    radio_vars.rf24_isr = flags[1];
    radio_vars.bb0_isr = flags[2];
    radio_vars.bb1_isr = flags[3];
}

//=========================== callbacks =======================================

//=========================== interrupt handlers ==============================
void radio_isr(void) {

    PORT_TIMER_WIDTH capturedTime;
    // kick_scheduler_t result = DO_NOT_KICK_SCHEDULER;

    debugpins_isr_set();

    GPIOPinIntClear(AT86RF215_IRQ_BASE, AT86RF215_IRQ_PIN);

    // capture the time
    capturedTime = sctimer_readCounter();
    //get isr that happened from radio
    radio_read_isr();

    if (radio_vars.bb0_isr & IRQS_RXFS_MASK){
        radio_vars.state = RADIOSTATE_RECEIVING;
        if (radio_vars.startFrame_cb!=NULL) {
            // call the callback
            radio_vars.startFrame_cb(capturedTime);
            // kick the OS
            // result = KICK_SCHEDULER;
        } else {
            //while(1);
        }
    } else {
        if ((radio_vars.bb0_isr & IRQS_TXFE_MASK)){
            radio_vars.state = RADIOSTATE_TXRX_DONE;
            if (radio_vars.endFrame_cb!=NULL) {
                // call the callback
                radio_vars.endFrame_cb(capturedTime);
                // kick the OS
                // result = KICK_SCHEDULER;
            } else {
                //while(1);
            }
        }  else {
            if ((radio_vars.bb0_isr & IRQS_RXFE_MASK)){
                radio_vars.state = RADIOSTATE_TXRX_DONE;
                if (radio_vars.endFrame_cb!=NULL) {
                    // call the callback
                    radio_vars.endFrame_cb(capturedTime);
                    // kick the OS
                    //result = KICK_SCHEDULER;
                } else {
                    // while(1);
                }
            }
        }
    }
    radio_clear_isr();
    debugpins_isr_clr();
}

port_INLINE void radio_clear_isr(){
    radio_vars.rf09_isr = 0;
    radio_vars.rf24_isr = 0;
    radio_vars.bb0_isr = 0;
    radio_vars.bb1_isr = 0;
}
