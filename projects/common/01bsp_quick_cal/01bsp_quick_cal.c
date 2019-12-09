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

#define LENGTH_PACKET           2+LENGTH_CRC    ///< maximum length is 127 bytes
#define SLOT_DURATION           (0xffff>>1)     ///< 0xffff     = 2s@32kHz
#define SUB_SLOT_DURATION       20              ///< 32         = 1ms@32kHz
#define ID                      0x99            ///< byte sent in the packets

#define NUM_PKT_SLOT            (2*32*32)       ///< slot duration = SUB_SLOT_DURATION *
                                                            // num_pkt_slot
#define CAL_STEPS               2
#define NUM_CHANNELS            16
#define SLOTFRAME_LEN           16
#define SYNC_CHANNEL            11

#define DEBUGGING

// 16 mote eui64

#define OTBOX08_MOTE_1          0xb5f3
#define OTBOX08_MOTE_2          0xb5d0
#define OTBOX08_MOTE_3          0xb595
#define OTBOX08_MOTE_4          0xb5d1

#define OTBOX09_MOTE_1          0xb5e7
#define OTBOX09_MOTE_2          0xb5d8
#define OTBOX09_MOTE_3          0xb55b
#define OTBOX09_MOTE_4          0xb558

#define OTBOX14_MOTE_1          0xb638
#define OTBOX14_MOTE_2          0xb5bf
#define OTBOX14_MOTE_3          0xb588
#define OTBOX14_MOTE_4          0xb5a3

#define OTBOX19_MOTE_1          0xb61a
#define OTBOX19_MOTE_2          0xb60b
#define OTBOX19_MOTE_3          0xb565
#define OTBOX19_MOTE_4          0xb648

//=========================== variables =======================================

typedef struct{
    uint8_t  channel;
    uint16_t short_address;
}ch_motes_t;

typedef struct{
    uint8_t     myId[8];
    ch_motes_t  ch_motes[NUM_CHANNELS];
    uint8_t     packet[LENGTH_PACKET];
    uint8_t     packet_len;
    uint8_t     channel;
    bool        isSync;
    uint8_t     currentSlotOffset;
    uint16_t    seqNum;
    uint32_t    slotRerference;
}app_vars_t;

app_vars_t app_vars;

//=========================== prototypes ======================================

void     cb_startFrame(PORT_TIMER_WIDTH timestamp);
void     cb_endFrame(PORT_TIMER_WIDTH timestamp);
void     cb_slot_timer(void);
void     cb_sub_slot_timer(void);

void     cb_uart_tx_done(void);
uint8_t  cb_uart_rx(void);

uint8_t  get_mychannel(uint8_t* myId);

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
    app_vars.channel = get_mychannel(myId);

#ifdef DEBUGGING
    app_vars.channel = SYNC_CHANNEL;
    app_vars.currentSlotOffset = SLOTFRAME_LEN - 1 ;
#endif

    // the mote assigned with SYNC_CHANNEL is the time reference
    if (app_vars.channel==SYNC_CHANNEL) {
        app_vars.isSync = TRUE;
    }

    // setup UART
    uart_setCallbacks(cb_uart_tx_done,cb_uart_rx);
    uart_enableInterrupts();

    // add callback functions radio
    radio_setStartFrameCb(cb_startFrame);
    radio_setEndFrameCb(cb_endFrame);

    // prepare packet
    temp = (app_vars.channel << 4) | ((app_vars.seqNum & 0x0f00)>>8);
    app_vars.packet[0] = temp;

    temp = app_vars.seqNum & 0x00ff;
    app_vars.packet[1] = temp;


    // start bsp timer
    sctimer_set_callback(cb_slot_timer);
    sctimer_setCompare(sctimer_readCounter()+SLOT_DURATION);
    sctimer_enable();

    // prepare radio
    radio_rfOn();
    radio_setFrequency(app_vars.channel);
    radio_rfOff();

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

//=========================== callbacks =======================================

void cb_startFrame(PORT_TIMER_WIDTH timestamp) {

}

void cb_endFrame(PORT_TIMER_WIDTH timestamp) {

}

void cb_slot_timer(void) {

    debugpins_slot_toggle();

    app_vars.slotRerference = sctimer_readCounter();

    sctimer_setCompare(app_vars.slotRerference+SLOT_DURATION);
    app_vars.currentSlotOffset = \
        (app_vars.currentSlotOffset + 1) % SLOTFRAME_LEN;

    // check the slot offset to decide to tx or rx:
    //  - if channel == current slotoffset: tx
    //  - else: rx

    if ((app_vars.channel - SYNC_CHANNEL)==app_vars.currentSlotOffset){

        // slot to tx

        // initial seqNum
        app_vars.seqNum = 0;

        radio_txEnable();

        // change sctimer callback to sub_slot cb

        sctimer_disable();
        sctimer_set_callback(cb_sub_slot_timer);

        // call the callback first time directly
        cb_sub_slot_timer();

    } else {
        // slot to rx

        radio_rxEnable();
    }
}

void cb_sub_slot_timer(void) {

    uint8_t temp;

    debugpins_fsm_toggle();

    // schedule next sub-slot
    sctimer_setCompare(sctimer_readCounter()+SUB_SLOT_DURATION);

    // load pkt and transmit

    app_vars.packet_len = sizeof(app_vars.packet);
    radio_loadPacket(app_vars.packet,app_vars.packet_len);
    radio_txNow();

    // update the seqNum in payload

    if (app_vars.seqNum<NUM_PKT_SLOT/CAL_STEPS) {
        app_vars.seqNum += 1;
         // prepare packet
        temp = (app_vars.channel << 4) | ((app_vars.seqNum & 0x0f00)>>8);
        app_vars.packet[0] = temp;

        temp = app_vars.seqNum & 0x00ff;
        app_vars.packet[1] = temp;
    } else {
        sctimer_disable();
        sctimer_set_callback(cb_slot_timer);
        sctimer_setCompare(app_vars.slotRerference+SLOT_DURATION);
    }
}

void cb_uart_tx_done(void) {

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
