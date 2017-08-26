#ifndef __OPENTCP_H
#define __OPENTCP_H

/**
\addtogroup Transport
\{
\addtogroup OpenTcp
\{
*/

#include "opentimers.h"

//=========================== define ==========================================

enum {
   TCP_INITIAL_SEQNUM             = 100,
   TCP_TIMEOUT                    = 1500, //in ms
};

enum TCP_STATE_enums {
   //listen state is not declared but emulated by a closed state with shouldIlisten==TRUE
   TCP_STATE_CLOSED               = 0,
   TCP_STATE_ALMOST_SYN_RECEIVED  = 1,
   TCP_STATE_SYN_RECEIVED         = 2,
   TCP_STATE_ALMOST_SYN_SENT      = 3,
   TCP_STATE_SYN_SENT             = 4,
   TCP_STATE_ALMOST_ESTABLISHED   = 5,
   TCP_STATE_ESTABLISHED          = 6,
   TCP_STATE_ALMOST_DATA_SENT     = 7,
   TCP_STATE_DATA_SENT            = 8,
   TCP_STATE_ALMOST_DATA_RECEIVED = 9,
   TCP_STATE_ALMOST_FIN_WAIT_1    = 10,
   TCP_STATE_FIN_WAIT_1           = 11,
   TCP_STATE_ALMOST_CLOSING       = 12,
   TCP_STATE_CLOSING              = 13,
   TCP_STATE_FIN_WAIT_2           = 14,
   TCP_STATE_ALMOST_TIME_WAIT     = 15,
   TCP_STATE_TIME_WAIT            = 16,
   TCP_STATE_ALMOST_CLOSE_WAIT    = 17,
   TCP_STATE_CLOSE_WAIT           = 18,
   TCP_STATE_ALMOST_LAST_ACK      = 19,
   TCP_STATE_LAST_ACK             = 20,
};

enum TCP_DEFAULTS_enum{
   TCP_DEFAULT_DATA_OFFSET        =   0x50,
   TCP_DEFAULT_WINDOW_SIZE        =     48,
   TCP_DEFAULT_URGENT_POINTER     = 0x0000,
};

enum TCP_ACK_FLAG_enum {
   TCP_ACK_WHATEVER               = 2,
   TCP_ACK_YES                    = 1,
   TCP_ACK_NO                     = 0,
};

enum TCP_PSH_FLAG_enum {
   TCP_PSH_WHATEVER               = 2,
   TCP_PSH_YES                    = 1,
   TCP_PSH_NO                     = 0,
};

enum TCP_RST_FLAG_enum {
   TCP_RST_WHATEVER               = 2,
   TCP_RST_YES                    = 1,
   TCP_RST_NO                     = 0,
};

enum TCP_SYN_FLAG_enum {
   TCP_SYN_WHATEVER               = 2,
   TCP_SYN_YES                    = 1,
   TCP_SYN_NO                     = 0,
};

enum TCP_FIN_FLAG_enum {
   TCP_FIN_WHATEVER               = 2,
   TCP_FIN_YES                    = 1,
   TCP_FIN_NO                     = 0,
};

enum TCP_FLAG_POSITIONS_enum {
   TCP_ACK                        = 4,
   TCP_PSH                        = 3,
   TCP_RST                        = 2,
   TCP_SYN                        = 1,
   TCP_FIN                        = 0,
};

//=========================== typedef =========================================

typedef struct {
   uint16_t source_port;
   uint16_t destination_port;
   uint32_t sequence_number;
   uint32_t ack_number;
   uint8_t  data_offset;
   uint8_t  control_bits;
   uint16_t window_size;
   uint16_t checksum;
   uint16_t urgent_pointer;
} tcp_ht;

//=========================== module variables ================================

typedef struct {
   uint8_t              state;
   uint32_t             mySeqNum;
   uint16_t             myPort;
   uint32_t             hisNextSeqNum;
   uint16_t             hisPort;
   open_addr_t          hisIPv6Address;
   OpenQueueEntry_t*    dataToSend;
   OpenQueueEntry_t*    dataReceived;
   bool                 timerStarted;
   opentimer_id_t       timerId;
} tcp_vars_t;

//=========================== prototypes ======================================

void     opentcp_init(void);
owerror_t  opentcp_connect(open_addr_t* dest, uint16_t param_hisPort, uint16_t param_myPort);
owerror_t  opentcp_send(OpenQueueEntry_t* msg);
void     opentcp_sendDone(OpenQueueEntry_t* msg, owerror_t error);
void     opentcp_receive(OpenQueueEntry_t* msg);
owerror_t  opentcp_close(void);
bool     opentcp_debugPrint(void);

/**
\}
\}
*/

#endif
