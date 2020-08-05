/**
\brief This program shows the use of the "radio" bsp module.

Since the bsp modules for different platforms have the same declaration, you
can use this project with any platform.

After loading this program, your board will switch on its radio on frequency
CHANNEL.

While receiving a packet (i.e. from the start of frame event to the end of
frame event), it will turn on its sync LED.

Every TIMER_PERIOD, it will also send a packet containing LENGTH_PACKET bytes
set to ID. While sending a packet (i.e. from the start of frame event to the
end of frame event), it will turn on its error LED.

\author Tengfei Chang <tengfei.chang@inria.fr>, August 2020.
*/

#include "board.h"
#include "radio.h"
#include "leds.h"
#include "sctimer.h"
#include "radio_ble.h"

//=========================== defines =========================================

#define LENGTH_PACKET   125+LENGTH_CRC  ///< maximum length is 127 bytes
#define CHANNEL         37              ///< 2=2402MHz (adv channel 37)
#define TIMER_PERIOD    (0xffff>>0)     ///< 0xffff = 2s@32kHz

const static uint8_t ble_device_addr[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};

//=========================== variables =======================================

enum {
    APP_FLAG_START_FRAME = 0x01,
    APP_FLAG_END_FRAME   = 0x02,
    APP_FLAG_TIMER       = 0x04,
};

typedef enum {
    APP_STATE_TX         = 0x01,
    APP_STATE_RX         = 0x02,
} app_state_t;

typedef struct {
    uint8_t              num_startFrame;
    uint8_t              num_endFrame;
    uint8_t              num_timer;
} app_dbg_t;

app_dbg_t app_dbg;

typedef struct {
                uint8_t         flags;
                app_state_t     state;
                uint8_t         packet[LENGTH_PACKET];
                uint8_t         packet_len;
                int8_t          rxpk_rssi;
                uint8_t         rxpk_lqi;
                bool            rxpk_crc;
} app_vars_t;

app_vars_t app_vars;

//=========================== prototypes ======================================

void     cb_startFrame(PORT_TIMER_WIDTH timestamp);
void     cb_endFrame(PORT_TIMER_WIDTH timestamp);
void     cb_timer(void);

void     assemble_packet(void);

//=========================== main ============================================

/**
\brief The program starts executing here.
*/
int mote_main(void) {
    uint8_t i;

    uint8_t freq_offset;
    uint8_t sign;
    uint8_t read;

    // clear local variables
    memset(&app_vars,0,sizeof(app_vars_t));

    // initialize board
    board_init();
    radio_ble_init();

    // add callback functions radio
    radio_setStartFrameCb(cb_startFrame);
    radio_setEndFrameCb(cb_endFrame);

    // prepare packet
    app_vars.packet_len = sizeof(app_vars.packet);

    // start bsp timer
    sctimer_set_callback(cb_timer);
    sctimer_setCompare(sctimer_readCounter()+TIMER_PERIOD);
    sctimer_enable();

    // prepare radio
    radio_rfOn();
    // freq type only effects on scum port
    radio_ble_setFrequency(CHANNEL);

    // switch in RX by default
    radio_rxEnable();
    app_vars.state = APP_STATE_RX;

    // start by a transmit
    app_vars.flags |= APP_FLAG_TIMER;

    while (1) {

        // sleep while waiting for at least one of the flags to be set
        while (app_vars.flags==0x00) {
            board_sleep();
        }

        // handle and clear every flag
        while (app_vars.flags) {


            //==== APP_FLAG_START_FRAME (TX or RX)

            if (app_vars.flags & APP_FLAG_START_FRAME) {
                // start of frame

                switch (app_vars.state) {
                    case APP_STATE_RX:
                        // started receiving a packet

                        // led
                        leds_error_on();
                        break;
                    case APP_STATE_TX:
                        // started sending a packet

                        // led
                        leds_sync_on();
                    break;
                }

                // clear flag
                app_vars.flags &= ~APP_FLAG_START_FRAME;
            }

            //==== APP_FLAG_END_FRAME (TX or RX)

            if (app_vars.flags & APP_FLAG_END_FRAME) {
                // end of frame

                switch (app_vars.state) {

                    case APP_STATE_RX:

                        // done receiving a packet
                        app_vars.packet_len = sizeof(app_vars.packet);

                        // get packet from radio
                        radio_ble_getReceivedFrame(
                            app_vars.packet,
                            &app_vars.packet_len,
                            sizeof(app_vars.packet),
                            &app_vars.rxpk_rssi,
                            &app_vars.rxpk_lqi,
                            &app_vars.rxpk_crc
                        );

                        if (app_vars.packet_len==21) {
                            for (i=0; i<app_vars.packet_len; i++){
                            
                                printf("%x ",app_vars.packet[i]);
                            }
                            printf("\r\n");
                        }

                        // led
                        leds_error_off();

                        // continue to listen
                        radio_rxNow();
                        break;
                    case APP_STATE_TX:
                        // done sending a packet

                        memset( app_vars.packet, 0x00, sizeof(app_vars.packet) );

                        // switch to RX mode
                        radio_rxEnable();
                        radio_rxNow();
                        app_vars.state = APP_STATE_RX;

                        // led
                        leds_sync_off();
                        break;
                }
                // clear flag
                app_vars.flags &= ~APP_FLAG_END_FRAME;
            }

            //==== APP_FLAG_TIMER

            if (app_vars.flags & APP_FLAG_TIMER) {
                // timer fired

                if (app_vars.state==APP_STATE_RX) {
                    // stop listening
                    radio_rfOff();

                    // prepare packet
                    app_vars.packet_len = sizeof(app_vars.packet);
                    
                    assemble_packet();

                    // start transmitting packet
                    radio_ble_loadPacket(app_vars.packet,LENGTH_PACKET);
                    radio_txEnable();
                    radio_txNow();

                    app_vars.state = APP_STATE_TX;
                }

                // clear flag
                app_vars.flags &= ~APP_FLAG_TIMER;
            }
        }
    }
}
//=========================== private =========================================

void assemble_packet(void) {

    uint8_t i;
    memset( app_vars.packet, 0x00, sizeof(app_vars.packet) );
    i=0;
    app_vars.packet[i++]  = 0x42;               // BLE advertisement type (non-connectable)
    app_vars.packet[i++]  = 0x13;               // Payload length: 14
    app_vars.packet[i++]  = ble_device_addr[0]; // BLE adv address byte 0
    app_vars.packet[i++]  = ble_device_addr[1]; // BLE adv address byte 1
    app_vars.packet[i++]  = ble_device_addr[2]; // BLE adv address byte 2
    app_vars.packet[i++]  = ble_device_addr[3]; // BLE adv address byte 3
    app_vars.packet[i++]  = ble_device_addr[4]; // BLE adv address byte 4
    app_vars.packet[i++]  = ble_device_addr[5]; // BLE adv address byte 5
    app_vars.packet[i++]  = 0x02;               // BLE adv payload byte 0, AD group 1 length byte
    app_vars.packet[i++]  = 0x01;               // BLE adv payload byte 1, AD group 1 type byte (flags)
    app_vars.packet[i++]  = 0x06;               // BLE adv payload byte 2, AD group 1 payload
    app_vars.packet[i++]  = 0x04;               // BLE adv payload byte 3, AD group 2 length byte
    app_vars.packet[i++]  = 0xff;               // BLE adv payload byte 4, AD group 2 type byte (manufacturer-specific data)
    app_vars.packet[i++]  = 0x29;               // BLE adv payload byte 5, AD group 2 payload (SwaraLink Technologies ID)
    app_vars.packet[i++]  = 0x07;               // BLE adv payload byte 6, AD group 2 payload (SwaraLink Technologies ID)
    app_vars.packet[i++]  = 0xbe;               // BLE adv payload byte 7, AD group 2 payload (byte to send)
    app_vars.packet[i++]  = 0x04;               // BLE adv payload byte 8, AD group 3 length byte
    app_vars.packet[i++]  = 0x08;               // BLE adv payload byte 9, AD group 3 type byte (short local name)
    app_vars.packet[i++]  = 'x';                // BLE adv payload byte 10, AD group 3 payload (name "xyz")
    app_vars.packet[i++]  = 'y';                // BLE adv payload byte 11, AD group 3 payload (name "xyz")
    app_vars.packet[i++]  = 'z';                // BLE adv payload byte 12, AD group 3 payload (name "xyz")
}

//=========================== callbacks =======================================

void cb_startFrame(PORT_TIMER_WIDTH timestamp) {
    // set flag
    app_vars.flags |= APP_FLAG_START_FRAME;

    // update debug stats
    app_dbg.num_startFrame++;
}

void cb_endFrame(PORT_TIMER_WIDTH timestamp) {
    // set flag
    app_vars.flags |= APP_FLAG_END_FRAME;

    // update debug stats
    app_dbg.num_endFrame++;
}

void cb_timer(void) {
    // set flag
    app_vars.flags |= APP_FLAG_TIMER;

    // update debug stats
    app_dbg.num_timer++;

    sctimer_setCompare(sctimer_readCounter()+TIMER_PERIOD);
}