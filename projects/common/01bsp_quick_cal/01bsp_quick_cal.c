/**
\brief This program shows the use of the "quick_cal" bsp module.

Since the bsp modules for different platforms have the same declaration, you
can use this project with any platform.

\author Tengfei Chang <tengfei.chang@inria.fr>, December 2019.
*/

#include "board.h"
#include "radio.h"
#include "leds.h"
#include "sctimer.h"
#include "uart.h"
#include "eui64.h"
#include "debugpins.h"

//=========================== defines =========================================

#define MAX_PKT_BUFFER          125+LENGTH_CRC  ///< maximum packet buffer size is 127 bytes
#define TARGET_PKT_LEN          2+LENGTH_CRC    ///< frame length sent by mote
#define MAGIC_BYTE              0x0FFF            ///< byte sent in the packets

#define NUM_PKT_PER_SLOT        (2*32*32)       ///< slot duration = SUB_SLOT_DURATION *
                                                            // NUM_PKT_PER_SLOT
#define CAL_STEPS               2
#define NUM_CHANNELS            16
#define SLOTFRAME_LEN           16
#define SYNC_CHANNEL            11

// timing
#define TXOFFSET                13              ///< measured, 382us
#define SLOT_DURATION           (0x17fff)       ///< 0xffff     = 2s@32kHz
#define SUB_SLOT_DURATION       20              ///< 32         = 1ms@32kHz

// 16 mote eui64

#define OTBOX08_MOTE_1          0xb5b3  // COM91: b5-b3
#define OTBOX08_MOTE_2          0xb5d0  // COM89: b5-d0
#define OTBOX08_MOTE_3          0xb595  // 00-12-4b-00-14-b5-b5-95
#define OTBOX08_MOTE_4          0xb5f3  // 00-12-4b-00-14-b5-b5-f3

#define OTBOX09_MOTE_1          0xb5e7  // 00-12-4b-00-14-b5-b5-e7
#define OTBOX09_MOTE_2          0xb5d8  // 00-12-4b-00-14-b5-b5-d8
#define OTBOX09_MOTE_3          0xb55b  // 00-12-4b-00-14-b5-b5-5b
#define OTBOX09_MOTE_4          0xb558  // 00-12-4b-00-14-b5-b5-58

#define OTBOX14_MOTE_1          0xb638  // 00-12-4b-00-14-b5-b6-38
#define OTBOX14_MOTE_2          0xb5bf  // 00-12-4b-00-14-b5-b5-bf
#define OTBOX14_MOTE_3          0xb588  // 00-12-4b-00-14-b5-b5-88
#define OTBOX14_MOTE_4          0xb5a3  // 00-12-4b-00-14-b5-b5-a3

#define OTBOX19_MOTE_1          0xb61a  // 00-12-4b-00-14-b5-b6-1a
#define OTBOX19_MOTE_2          0xb60b  // 00-12-4b-00-14-b5-b6-0b
#define OTBOX19_MOTE_3          0xb565  // 00-12-4b-00-14-b5-b5-65
#define OTBOX19_MOTE_4          0xb648  // 00-12-4b-00-14-b5-b6-48

// debugging

#define UART_BUFFER_LEN         100

//=========================== variables =======================================

typedef struct{
    uint8_t  channel;
    uint16_t short_address;
}ch_motes_t;

typedef enum{
    T_IDLE              = 0,
    T_TX                = 1,
    T_RX                = 2,
}type_t;

typedef enum{
    S_IDLE              = 0,
    S_LISTENING         = 1,
    S_RECEIVING         = 2,
    S_RXPROC            = 3,
    S_DATA_SENDING      = 4,
    S_ACK_SENDING       = 5,
    S_DATA_SENDDONE     = 6,
    S_ACK_SENDDONE      = 7,
    S_DATA_SEND         = 8,
    S_ACK_SEND          = 9,
}state_t;

typedef struct{
    uint8_t     packet[MAX_PKT_BUFFER];
    uint8_t     packet_len;
     int8_t     rxpk_rssi;
    uint8_t     rxpk_lqi;
       bool     rxpk_crc;

    uint8_t     myId[8];
    ch_motes_t  ch_motes[NUM_CHANNELS];
    uint8_t     myChannel;

    bool        isTimeMaster;
    bool        isSync;
    uint8_t     currentSlotOffset;
    uint16_t    seqNum;
    uint32_t    slotRerference;
    type_t      type;
    state_t     state;
    uint32_t    lastCaptureTime;
}app_vars_t;

// debugging

typedef enum{
    D_ERROR     = 'E',
    D_INFO      = 'I',
}debug_type_t;

typedef struct{
    uint8_t     uart_to_send[UART_BUFFER_LEN];
    uint8_t     index;
}debug_vars_t;

app_vars_t      app_vars;
debug_vars_t    debug_vars;

const uint8_t debuginfo[] = "ack sent";

//=========================== prototypes ======================================

void     cb_startFrame(PORT_TIMER_WIDTH timestamp);
void     cb_endFrame(PORT_TIMER_WIDTH timestamp);
void     cb_slot_timer(void);
void     cb_sub_slot_timer(void);

void     cb_uart_tx_done(void);
uint8_t  cb_uart_rx(void);

uint8_t  get_mychannel(uint8_t* myId);
void     synchronize(uint32_t capturedTime, uint8_t pkt_channel, uint16_t pkt_seqNum);

// debugging
void     debug_output(debug_type_t type, uint8_t* buffer, uint8_t length);
uint8_t  int_to_char(uint8_t temp);

//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int mote_main(void) {
    uint16_t temp;
    uint8_t  myId[8];

    // clear local variables
    memset(&app_vars,0,sizeof(app_vars_t));

    // initialize board
    board_init();

    // initialize the channel mote assignment
    app_vars.ch_motes[0].channel        = 11;
    app_vars.ch_motes[0].short_address  = OTBOX08_MOTE_1;
    app_vars.ch_motes[1].channel        = 12;
    app_vars.ch_motes[1].short_address  = OTBOX08_MOTE_2;
    app_vars.ch_motes[2].channel        = 13;
    app_vars.ch_motes[2].short_address  = OTBOX08_MOTE_3;
    app_vars.ch_motes[3].channel        = 14;
    app_vars.ch_motes[3].short_address  = OTBOX08_MOTE_4;

    app_vars.ch_motes[4].channel        = 15;
    app_vars.ch_motes[4].short_address  = OTBOX09_MOTE_1;
    app_vars.ch_motes[5].channel        = 16;
    app_vars.ch_motes[5].short_address  = OTBOX09_MOTE_2;
    app_vars.ch_motes[6].channel        = 17;
    app_vars.ch_motes[6].short_address  = OTBOX09_MOTE_3;
    app_vars.ch_motes[7].channel        = 18;
    app_vars.ch_motes[7].short_address  = OTBOX09_MOTE_4;

    app_vars.ch_motes[8].channel        = 19;
    app_vars.ch_motes[8].short_address  = OTBOX14_MOTE_1;
    app_vars.ch_motes[9].channel        = 20;
    app_vars.ch_motes[9].short_address  = OTBOX14_MOTE_2;
    app_vars.ch_motes[10].channel       = 21;
    app_vars.ch_motes[10].short_address = OTBOX14_MOTE_3;
    app_vars.ch_motes[11].channel       = 22;
    app_vars.ch_motes[11].short_address = OTBOX14_MOTE_4;

    app_vars.ch_motes[12].channel       = 23;
    app_vars.ch_motes[12].short_address = OTBOX19_MOTE_1;
    app_vars.ch_motes[13].channel       = 24;
    app_vars.ch_motes[13].short_address = OTBOX19_MOTE_2;
    app_vars.ch_motes[14].channel       = 25;
    app_vars.ch_motes[14].short_address = OTBOX19_MOTE_3;
    app_vars.ch_motes[15].channel       = 26;
    app_vars.ch_motes[15].short_address = OTBOX19_MOTE_4;

    // get eui64 and calculate channel assignment
    eui64_get(myId);
    app_vars.myChannel = get_mychannel(myId);

    // the mote assigned with SYNC_CHANNEL is the time reference
    if (app_vars.myChannel==SYNC_CHANNEL) {
        app_vars.isSync = TRUE;
        app_vars.isTimeMaster = TRUE;
        app_vars.currentSlotOffset = SLOTFRAME_LEN-1;
    }

    // setup UART
    uart_setCallbacks(cb_uart_tx_done,cb_uart_rx);
    uart_enableInterrupts();

    // add callback functions radio
    radio_setStartFrameCb(cb_startFrame);
    radio_setEndFrameCb(cb_endFrame);

    // prepare packet
    temp = ((app_vars.myChannel-SYNC_CHANNEL) << 4) | ((app_vars.seqNum & 0x0f00)>>8);
    app_vars.packet[0] = temp;

    temp = app_vars.seqNum & 0x00ff;
    app_vars.packet[1] = temp;


    // start bsp timer
    sctimer_set_callback(cb_slot_timer);
    sctimer_setCompare(sctimer_readCounter()+SLOT_DURATION);
    sctimer_enable();

    while (1) {
        board_sleep();
    }
}

//=========================== help ============================================

uint8_t get_mychannel(uint8_t* address){
    uint16_t short_address;
    uint8_t  channel;
    short_address = ((address[6] & 0xffff)<<8) | (address[7] & 0xffff);
    switch(short_address){
    case OTBOX08_MOTE_1:
        channel = 11;
        break;
    case OTBOX08_MOTE_2:
        channel = 12;
        break;
    case OTBOX08_MOTE_3:
        channel = 13;
        break;
    case OTBOX08_MOTE_4:
        channel = 14;
        break;
    case OTBOX09_MOTE_1:
        channel = 15;
        break;
    case OTBOX09_MOTE_2:
        channel = 16;
        break;
    case OTBOX09_MOTE_3:
        channel = 17;
        break;
    case OTBOX09_MOTE_4:
        channel = 18;
        break;
    case OTBOX14_MOTE_1:
        channel = 19;
        break;
    case OTBOX14_MOTE_2:
        channel = 20;
        break;
    case OTBOX14_MOTE_3:
        channel = 21;
        break;
    case OTBOX14_MOTE_4:
        channel = 22;
        break;
    case OTBOX19_MOTE_1:
        channel = 23;
        break;
    case OTBOX19_MOTE_2:
        channel = 24;
        break;
    case OTBOX19_MOTE_3:
        channel = 25;
        break;
    case OTBOX19_MOTE_4:
        channel = 26;
        break;
    default:
        channel = 0;
    }

    return channel;
}

void synchronize(uint32_t capturedTime, uint8_t pkt_channel, uint16_t pkt_seqNum){

    uint32_t slot_boudary;

    // synchronize currentslotoffset
    app_vars.currentSlotOffset = pkt_channel-SYNC_CHANNEL;

    // synchronize to slot boudary
    sctimer_disable();
    slot_boudary = capturedTime - pkt_seqNum*SUB_SLOT_DURATION - TXOFFSET;
    sctimer_set_callback(cb_slot_timer);
    sctimer_setCompare(slot_boudary+SLOT_DURATION);

    app_vars.isSync = TRUE;

    leds_sync_on();
}

// debugging

uint8_t int_to_char(uint8_t temp){

    uint8_t returnVal;

    if (temp>0x09){
        returnVal = (temp-0x0a)+'a';
    } else {
        returnVal = (temp)+'0';
    }

    return returnVal;
}

void debug_output(debug_type_t type, uint8_t* buffer, uint8_t length){

    uint8_t temp;
    uint8_t i;
    uint8_t len;

    len = 0;
    debug_vars.uart_to_send[len++]        = type;

    for (i=0;i<length;i++){
        temp = (buffer[i] & 0xf0)>>4;
        debug_vars.uart_to_send[len++] = int_to_char(temp);
        temp = buffer[i] & 0x0f;
        debug_vars.uart_to_send[len++] = int_to_char(temp);
    }

    // add ending chars
    debug_vars.uart_to_send[len++] = '\r';
    debug_vars.uart_to_send[len++] = '\n';

    // write to uart
    debug_vars.index = 0;
    uart_writeByte(debug_vars.uart_to_send[debug_vars.index]);
}

//=========================== callbacks =======================================

void cb_startFrame(PORT_TIMER_WIDTH timestamp) {

    app_vars.lastCaptureTime = timestamp;

    switch(app_vars.type){
    case T_TX:
        switch(app_vars.state) {
        case S_DATA_SEND:
            app_vars.state = S_DATA_SENDING;
            break;
        case S_ACK_SEND:
            app_vars.state = S_ACK_SENDING;
            break;
        default:
            // wrong state

            debug_output(D_ERROR, (uint8_t*)(&app_vars.state), 1);
            return;
        }
        break;
    case T_RX:
        switch(app_vars.state){
        case S_LISTENING:
            app_vars.state = S_RECEIVING;
            break;
        default:
            // wrong state

            debug_output(D_ERROR, (uint8_t*)(&app_vars.state), 1);
            return;
        }
        break;
    }
}

void cb_endFrame(PORT_TIMER_WIDTH timestamp) {

    bool        isValidFrame;
    uint8_t     pkt_channel;
    uint16_t    pkt_seqNum;

    int8_t      freq_offset;

    // turn radio off first
    radio_rfOff();

    switch(app_vars.type){
    case T_TX:

        switch(app_vars.state){
        case S_DATA_SENDING:
            app_vars.state = S_DATA_SENDDONE;

            // nothing need to do

        break;
        case S_ACK_SENDING:
            app_vars.state = S_ACK_SENDDONE;

            // set radio back to listen on myChannel

            radio_rfOn();
            radio_setFrequency(app_vars.myChannel);
            radio_rxEnable();
            radio_rxNow();

            app_vars.type = T_RX;

            app_vars.state = S_LISTENING;

        break;
        default:
            // wrong status

            debug_output(D_ERROR, (uint8_t*)(&app_vars.state), 1);
            return;
        }

    break;
    case T_RX:

        // received frames
        switch(app_vars.state){
        case S_RECEIVING:
            app_vars.state = S_RXPROC;
        break;
        default:
            // wrong state

            debug_output(D_ERROR, (uint8_t*)(&app_vars.state), 1);
            return;
        }

        memset(app_vars.packet,0,MAX_PKT_BUFFER);
        app_vars.rxpk_crc = 0;

        // get packet from radio
        radio_getReceivedFrame(
            app_vars.packet,
            &app_vars.packet_len,
            sizeof(app_vars.packet),
            &app_vars.rxpk_rssi,
            &app_vars.rxpk_lqi,
            &app_vars.rxpk_crc
        );

        // check the frame is valid or not

        isValidFrame = TRUE;

        if (app_vars.rxpk_crc && app_vars.packet_len == TARGET_PKT_LEN){

            pkt_channel = SYNC_CHANNEL+((app_vars.packet[0] & 0xf0)>>4);
            pkt_seqNum  = ((uint16_t)(app_vars.packet[0] & 0x0f))<<8 |
                           (uint16_t)(app_vars.packet[1]);

            if (
                pkt_seqNum >= NUM_PKT_PER_SLOT && 
                pkt_seqNum != MAGIC_BYTE
            ){
                isValidFrame = FALSE;
            }

            if (app_vars.isSync){
                if ((pkt_channel - SYNC_CHANNEL)!= app_vars.currentSlotOffset){
                    // maybe received from other channel: abnormal behavior of cc2538

                    isValidFrame = FALSE;
                }
            } else {
                if (pkt_channel != SYNC_CHANNEL){
                    isValidFrame = FALSE;
                }
            }
        } else {
            isValidFrame = FALSE;
        }

        if (isValidFrame){

            if (app_vars.isSync){
                if (app_vars.currentSlotOffset==0){
                    
                    // received from time reference, re-synchronize
                    
                    if (
                        app_vars.isTimeMaster == FALSE && 
                        pkt_seqNum            != MAGIC_BYTE
                    ) {
                        synchronize(app_vars.lastCaptureTime, pkt_channel, pkt_seqNum);
                        
                        // switch to listen on my channel
                        radio_rfOn();
                        radio_setFrequency(app_vars.myChannel);
                        radio_rxEnable();
                        radio_rxNow();
                        
                        app_vars.state = S_LISTENING;
                        
                    } else {
                        if (
                            app_vars.isTimeMaster == FALSE && 
                            pkt_seqNum            == MAGIC_BYTE
                        ){
                            // received from SCuM, prepare Ack to send back

                            // read the freq_offset
                            freq_offset = radio_getFrequencyOffset();

                            radio_rfOn();
                            radio_setFrequency(app_vars.myChannel);

                            // the ack use freq_offset as second byte
                            app_vars.packet[1] = (uint8_t)freq_offset;
                            radio_loadPacket(app_vars.packet, TARGET_PKT_LEN);
                            radio_txEnable();
                            radio_txNow();

                            app_vars.type  = T_TX;
                            app_vars.state = S_ACK_SEND;
                        }
                    }
                } else {
                    
                    if (pkt_seqNum  == MAGIC_BYTE){
                        
                        // received from SCuM, prepare ack to send back

                        // read the freq_offset
                        freq_offset = radio_getFrequencyOffset();

                        radio_rfOn();
                        radio_setFrequency(app_vars.myChannel);

                        // the ack use freq_offset as second byte
                        app_vars.packet[1] = (uint8_t)freq_offset;
                        radio_loadPacket(app_vars.packet, TARGET_PKT_LEN);
                        radio_txEnable();
                        radio_txNow();

                        app_vars.type  = T_TX;
                        app_vars.state = S_ACK_SEND;
                    } else {
                        
                        // not sent by SCuM, keep listening
                        
                        radio_rfOn();
                        radio_setFrequency(app_vars.myChannel);
                        radio_rxEnable();
                        radio_rxNow();

                        app_vars.state = S_LISTENING;
                    }
                }
            } else {
                // received from timer reference, synchronize

                synchronize(app_vars.lastCaptureTime, pkt_channel, pkt_seqNum);
            }

        } else {

            // continuous to listen on myChannel

            radio_rfOn();
            if (app_vars.isSync){
                radio_setFrequency(app_vars.myChannel);
            } else {
                radio_setFrequency(SYNC_CHANNEL);
            }
            radio_rxEnable();
            radio_rxNow();

            app_vars.state = S_LISTENING;
        }
    break;
    default:
        // wrong type

        leds_error_blink();
    }
}

void cb_slot_timer(void) {

    debugpins_slot_toggle();

    // schedule next slot timer

    app_vars.slotRerference = sctimer_readCounter();

    sctimer_setCompare(app_vars.slotRerference+SLOT_DURATION);
    app_vars.currentSlotOffset = \
        (app_vars.currentSlotOffset + 1) % SLOTFRAME_LEN;

    // initial seqNum
    app_vars.seqNum = 0;

    // prepare radio
    radio_rfOn();

    if (app_vars.isSync){

        leds_sync_on();

        if ((app_vars.myChannel - SYNC_CHANNEL)==app_vars.currentSlotOffset){

            // this is my slot

            // slot to tx
            app_vars.type = T_TX;

            radio_setFrequency(app_vars.myChannel);

            // change sctimer callback to sub_slot cb
            sctimer_disable();
            sctimer_set_callback(cb_sub_slot_timer);
            
            app_vars.packet[0] = ((app_vars.myChannel-SYNC_CHANNEL) << 4) & 0xf0;
            app_vars.packet[1] = 0;

            // call the callback first time directly
            cb_sub_slot_timer();
        } else {

            // this is NOT my slot

            // slot to rx
            app_vars.type = T_RX;

            if (app_vars.currentSlotOffset==0){

                radio_setFrequency(SYNC_CHANNEL);
            } else {

                radio_setFrequency(app_vars.myChannel);
            }

            // start to listen
            radio_rxEnable();
            radio_rxNow();
            app_vars.state = S_LISTENING;
        }

    } else {

        leds_sync_off();

        // slot to rx
        app_vars.type = T_RX;

        switch(app_vars.state){
        case S_IDLE:
            radio_setFrequency(SYNC_CHANNEL);

            // start to listen
            radio_rxEnable();
            radio_rxNow();

            app_vars.state = S_LISTENING;

        break;
        case S_LISTENING:

        break;
        case S_RECEIVING:

        break;
        case S_RXPROC:

        break;
        default:
            // wrong state

            debug_output(D_ERROR, (uint8_t*)(&app_vars.state), 1);
        }

    }
}

void cb_sub_slot_timer(void) {

    uint8_t temp;

    debugpins_fsm_toggle();

    // schedule next sub-slot
    sctimer_setCompare(
        app_vars.slotRerference + (app_vars.seqNum+1) * SUB_SLOT_DURATION
    );

    // load pkt and transmit

    radio_rfOn();
    radio_txEnable();
    app_vars.packet_len = TARGET_PKT_LEN;
    radio_loadPacket(app_vars.packet,app_vars.packet_len);
    radio_txNow();

    app_vars.state = S_DATA_SEND;

    // update the seqNum in payload

    // if (app_vars.seqNum<NUM_PKT_PER_SLOT/CAL_STEPS) {
    if (app_vars.seqNum<(SLOT_DURATION/SUB_SLOT_DURATION-2)) {
        app_vars.seqNum += 1;
         // prepare packet
        temp = ((app_vars.myChannel-SYNC_CHANNEL) << 4) | ((app_vars.seqNum & 0x0f00)>>8);
        app_vars.packet[0] = temp;

        temp = app_vars.seqNum & 0x00ff;
        app_vars.packet[1] = temp;
    } else {
        sctimer_disable();
        sctimer_set_callback(cb_slot_timer);
        sctimer_setCompare(app_vars.slotRerference+SLOT_DURATION);
        radio_rfOff();
    }
}

void cb_uart_tx_done(void) {
    debug_vars.index++;
    if (debug_vars.uart_to_send[debug_vars.index-1]!='\n'){
        uart_writeByte(debug_vars.uart_to_send[debug_vars.index]);
    } else {
        debug_vars.index = 0;
    }
}

uint8_t cb_uart_rx(void) {
    uint8_t byte;

    // toggle LED
    leds_error_toggle();

    // read received byte
    byte = uart_readByte();

    // echo that byte over serial
    uart_writeByte(byte);

    return 0;
}
