/**
\brief General OpenWSN definitions

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, August 2010
\author Ankur Mehta <mehtank@eecs.berkeley.edu>, September 2010
*/

#ifndef __OPENWSN_H
#define __OPENWSN_H

//general
#include <stdint.h>                              // needed for uin8_t, uint16_t
//#include "string.h"                              // needed for memcpy and memcmp <-- now in board.info
#include "board_info.h"

//=========================== define ==========================================

static const uint8_t infoStackName[] = "OpenWSN ";
#define OPENWSN_VERSION_MAJOR 1
#define OPENWSN_VERSION_MINOR 2
#define OPENWSN_VERSION_PATCH 1

// enter the last byte of your mote's address if you want it to be an LBR
#define DEBUG_MOTEID_MASTER 0x6A


#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

enum {
   E_SUCCESS                           = 0,          
   E_FAIL                              = 1,
};

// types of addresses
enum {
   ADDR_NONE                           = 0,
   ADDR_16B                            = 1,
   ADDR_64B                            = 2,
   ADDR_128B                           = 3,
   ADDR_PANID                          = 4,
   ADDR_PREFIX                         = 5,
   ADDR_ANYCAST                        = 6,
};

enum {
   LITTLE_ENDIAN                       = TRUE,
   BIG_ENDIAN                          = FALSE,
};

// protocol numbers, as defined by the IANA
enum {
   IANA_UNDEFINED                      = 0x00,
   IANA_TCP                            = 0x06,
   IANA_UDP                            = 0x11,
   IANA_ICMPv6                         = 0x3a,
   IANA_ICMPv6_ECHO_REQUEST            =  128,
   IANA_ICMPv6_ECHO_REPLY              =  129,
   IANA_ICMPv6_RS                      =  133,
   IANA_ICMPv6_RA                      =  134,
   IANA_ICMPv6_RA_PREFIX_INFORMATION   =    3,
   IANA_ICMPv6_RPL                     =  155,
   IANA_ICMPv6_RPL_DIO                 = 0x01,
   IANA_RSVP                           = 46,
};

// well known ports (which we define)
enum {
   //TCP
   WKP_TCP_HTTP                        =   80,
   WKP_TCP_ECHO                        =    7,
   WKP_TCP_INJECT                      = 2188,
   WKP_TCP_DISCARD                     =    9,
   //UDP
   WKP_UDP_COAP                        = 5683,
   WKP_UDP_HELI                        = 2192,
   WKP_UDP_IMU                         = 2190,
   WKP_UDP_ECHO                        =    7,
   WKP_UDP_INJECT                      = 2188,
   WKP_UDP_DISCARD                     =    9,
   WKP_UDP_RAND                        = 61000,
};

//status elements
enum {
   STATUS_ISSYNC                       = 0,
   STATUS_ID                           = 1,
   STATUS_DAGRANK                      = 2,
   STATUS_OUTBUFFERINDEXES             = 3,
   STATUS_ASN                          = 4,
   STATUS_MACSTATS                     = 5,
   STATUS_SCHEDULE                     = 6,
   STATUS_QUEUE                        = 7,
   STATUS_NEIGHBORS                    = 8,
   STATUS_MAX                          = 9,
};

//component identifiers
enum {
   COMPONENT_NULL                      = 0x00,
   //cross-layers
   COMPONENT_IDMANAGER                 = 0x01,
   COMPONENT_OPENQUEUE                 = 0x02,
   COMPONENT_OPENSERIAL                = 0x03,
   COMPONENT_PACKETFUNCTIONS           = 0x04,
   COMPONENT_RANDOM                    = 0x05,
   //PHY
   COMPONENT_RADIO                     = 0x06,
   //MAClow
   COMPONENT_IEEE802154                = 0x07,
   COMPONENT_IEEE802154E               = 0x08,
   //MAClow<->MAChigh ("virtual components")
   COMPONENT_RES_TO_IEEE802154E        = 0x09,
   COMPONENT_IEEE802154E_TO_RES        = 0x0a,
   //MAChigh
   COMPONENT_RES                       = 0x0b,
   COMPONENT_NEIGHBORS                 = 0x0c,
   COMPONENT_SCHEDULE                  = 0x0d,
   //IPHC
   COMPONENT_OPENBRIDGE                = 0x0e,
   COMPONENT_IPHC                      = 0x0f,
   //IPv6
   COMPONENT_FORWARDING                = 0x10,
   COMPONENT_ICMPv6                    = 0x11,
   COMPONENT_ICMPv6ECHO                = 0x12,
   COMPONENT_ICMPv6ROUTER              = 0x13,
   COMPONENT_ICMPv6RPL                 = 0x14,
   //TRAN
   COMPONENT_OPENTCP                   = 0x15,             
   COMPONENT_OPENUDP                   = 0x16,
   COMPONENT_OPENCOAP                  = 0x17,
   //App test
   COMPONENT_TCPECHO                   = 0x18,
   COMPONENT_TCPINJECT                 = 0x19,
   COMPONENT_TCPPRINT                  = 0x1a,
   COMPONENT_UDPECHO                   = 0x1b,
   COMPONENT_UDPINJECT                 = 0x1c,
   COMPONENT_UDPPRINT                  = 0x1d,
   COMPONENT_RSVP                      = 0x1e,
   //App
   COMPONENT_OHLONE                    = 0x1f,
   COMPONENT_HELI                      = 0x20,
   COMPONENT_IMU                       = 0x21,
   COMPONENT_RLEDS                     = 0x22,
   COMPONENT_RREG                      = 0x23,
   COMPONENT_RWELLKNOWN                = 0x24,
   COMPONENT_RT                        = 0x25,
   COMPONENT_REX                       = 0x26,
   COMPONENT_RXL1                      = 0x27,
   COMPONENT_RINFO                     = 0x28,
   COMPONENT_RHELI                     = 0x29,
   COMPONENT_RRUBE                     = 0x2a,
   COMPONENT_LAYERDEBUG                = 0x2b,
   COMPONENT_UDPRAND                   = 0x2c,
};

//error codes
enum {
   // l7
   ERR_SENDDONE                        = 0x01, // sendDone                          [AppP]    arg1=error arg2=ack   
   ERR_RCVD_ECHO_REQUEST               = 0x02, // received echo request             [RPLC]
   ERR_RCVD_ECHO_REPLY                 = 0x03, // received echo reply               [RPLC]
   ERR_GETDATA_ASKS_TOO_FEW_BYTES      = 0x04, // getData asks too few bytes        [SerialIO] arg1=maxNumBytes arg2=input_buffer_fill_level
   ERR_INPUT_BUFFER_OVERFLOW           = 0x05, // input buffer overflow             [SerialIO]
   // l4
   ERR_WRONG_TRAN_PROTOCOL             = 0x06, // wrong transport protocol          [App] arg=tran_protocol
   ERR_WRONG_TCP_STATE                 = 0x07, // wrong TCP state                   [TCP] arg=state arg2=location
   ERR_RESET                           = 0x08, // TCP reset                         [TCP] arg=state arg2=location
   ERR_UNSUPPORTED_PORT_NUMBER         = 0x09, // unsupported port number           [all Apps and transport protocols] arg1=portNumber
   // l3
   ERR_UNSUPPORTED_ICMPV6_TYPE         = 0x0a, // unsupported ICMPv6 type           [RPLC] arg1=icmpv6_type arg2=location
   ERR_6LOWPAN_UNSUPPORTED             = 0x0b, // unsupported 6LoWPAN parameter     [IPHC] arg1=location arg2=param
   ERR_NO_NEXTHOP                      = 0x0c, // no nextHop                        [RPL]   
   // l2b
   ERR_NEIGHBORS_FULL                  = 0x0d, // neighbors table is full           [NeighborsP] arg1=MAXNUMNEIGHBORS
   ERR_NO_SENT_PACKET                  = 0x0e, // there is no sent packet in queue
   ERR_NO_RECEIVED_PACKET              = 0x0f, // there is no received packet in queue
   // l2a
   ERR_ASN_MISALIGNEMENT               = 0x10, // impossible ASN in ADV
   ERR_WRONG_CELLTYPE                  = 0x11, // wrong celltype                    [Schedule,IEEE802154EP,OpenQueueP] arg1=type      
   ERR_IEEE154_UNSUPPORTED             = 0x12, // unsupported 802154 parameter      [IEEE802154EP] arg1=location arg2=param   
   ERR_DESYNCHRONIZED                  = 0x13, // desynchronized                    [IEEE802154EP] arg1=slotOffset
   ERR_SYNCHRONIZED                    = 0x14, // synchronized                      [IEEE802154EP] arg1=slotOffset
   ERR_WRONG_STATE_IN_ENDFRAME_SYNC    = 0x15, // wrong state in end of frame+sync
   ERR_WRONG_STATE_IN_STARTSLOT        = 0x16, // wrong state in startSlot          [IEEE802154EP]  arg1=state arg2=slotOffset
   ERR_WRONG_STATE_IN_TIMERFIRES       = 0x17, // wrong state in timer fires        [IEEE154E] arg1=state, arg2=slotOffset  
   ERR_WRONG_STATE_IN_NEWSLOT          = 0x18, // wrong state in start of frame     [IEEE154E] arg1=state, arg2=slotOffset
   ERR_WRONG_STATE_IN_ENDOFFRAME       = 0x19, // wrong state in end of frame       [IEEE154E] arg1=state, arg2=slotOffset
   ERR_MAXTXDATAPREPARE_OVERFLOW       = 0x1a, // maxTxDataPrepare overflows        [IEEE154E] arg1=state, arg2=slotOffset
   ERR_MAXRXACKPREPARE_OVERFLOWS       = 0x1b, // maxRxAckPrepapre overflows        [IEEE154E] arg1=state, arg2=slotOffset
   ERR_MAXRXDATAPREPARE_OVERFLOWS      = 0x1c, // maxRxDataPrepapre overflows       [IEEE154E] arg1=state, arg2=slotOffset
   ERR_MAXTXACKPREPARE_OVERFLOWS       = 0x1d, // maxTxAckPrepapre overflows        [IEEE154E] arg1=state, arg2=slotOffset   
   ERR_WDDATADURATION_OVERFLOWS        = 0x1e, // wdDataDuration overflows          [IEEE154E] arg1=state, arg2=slotOffset
   ERR_WDRADIO_OVERFLOW                = 0x1f, // wdRadio overflows                 [IEEE154E] arg1=state, arg2=slotOffset
   ERR_WDRADIOTX_OVERFLOWS             = 0x20, // wdRadioTx overflows               [IEEE154E] arg1=state, arg2=slotOffset
   ERR_WDACKDURATION_OVERFLOWS         = 0x21, // wdAckDuration overflows           [IEEE154E] arg1=state, arg2=slotOffset   
   // drivers
   ERR_WRONG_IRQ_STATUS                = 0x22, // wrong IRQ_STATUS                  [radio] arg1=irq_status
   // general
   ERR_BUSY_SENDING                    = 0x23, // busy sending a packet             [RPLP,TCPP] arg1=location
   ERR_UNEXPECTED_SENDDONE             = 0x24, // sendDone for packet I didn't send [App,Advertise,KeepAlive,Reservation]
   ERR_NO_FREE_PACKET_BUFFER           = 0x25, // no free Queuepkt Cell             [NeighborsP, NRESP, AppSensorP, IEEE802154EP] arg1=codeLocation
   ERR_FREEING_UNUSED                  = 0x26, // freeing unused memory             []
   ERR_FREEING_ERROR                   = 0x27, // freeing memory unsupported memory []
   ERR_UNSUPPORTED_COMMAND             = 0x28, // unsupported command=arg1          [SerialIOP] arg1=command
   ERR_MSG_UNKNOWN_TYPE                = 0x29, // received message of unknown type  [NRESC,OpenQueueP] arg1=type
   ERR_WRONG_ADDR_TYPE                 = 0x2a, // wrong address type                [IEEE802154EP,IDManagerP,PacketFunctions] arg1=addressType arg2=codeLocation
   ERR_BRIDGE_MISMATCH                 = 0x2b, // isBridge mismatch                 [NRES] arg1=code_location
   ERR_HEADER_TOO_LONG                 = 0x2c, // header too long                   [PacketFunctions] arg1=code_location
   ERR_INPUTBUFFER_LENGTH              = 0x2d, // input length problem              [openSerial, all components which get Triggered] arg1=input_buffer_length arg2=location   
   ERR_UNSPECIFIED                     = 0x2e, // unspecified error                 []
};

//=========================== typedef =========================================

typedef uint16_t  shortnodeid_t;
typedef uint64_t  longnodeid_t;
typedef uint16_t  errorparameter_t;
typedef uint8_t   dagrank_t;
typedef uint8_t   error_t;
#define bool uint8_t

typedef struct {
   uint8_t  byte4;
   uint16_t bytes2and3;
   uint16_t bytes0and1;
} asn_t;

typedef struct {                                 // always written big endian, i.e. MSB in addr[0]
   uint8_t type;
   union {
      uint8_t addr_16b[2];
      uint8_t addr_64b[8];
      uint8_t addr_128b[16];
      uint8_t panid[2];
      uint8_t prefix[8];
   };
} open_addr_t;

typedef struct {
   //admin
   uint8_t       creator;                        // the component which called getFreePacketBuffer()
   uint8_t       owner;                          // the component which currently owns the entry
   uint8_t*      payload;                        // pointer to the start of the payload within 'packet'
   uint8_t       length;                         // length in bytes of the payload
   //l4
   uint8_t       l4_protocol;                    // l4 protocol to be used
   bool          l4_protocol_compressed;         // is the l4 protocol header compressed?
   uint16_t      l4_sourcePortORicmpv6Type;      // l4 source port
   uint16_t      l4_destination_port;            // l4 destination port
   uint8_t*      l4_payload;                     // pointer to the start of the payload of l4 (used for retransmits)
   uint8_t       l4_length;                      // length of the payload of l4 (used for retransmits)
   //l3
   open_addr_t   l3_destinationORsource;         // 128b IPv6 destination (down stack) or source address (up)
   //l2
   error_t       l2_sendDoneError;               // outcome of trying to send this packet
   open_addr_t   l2_nextORpreviousHop;           // 64b IEEE802.15.4 next (down stack) or previous (up) hop address
   uint8_t       l2_frameType;                   // beacon, data, ack, cmd
   uint8_t       l2_dsn;                         // sequence number of the received frame
   uint8_t       l2_retriesLeft;                 // number Tx retries left before packet dropped (dropped when hits 0)
   uint8_t       l2_numTxAttempts;               // number Tx attempts
   asn_t         l2_asn;                         // at what ASN the packet was Tx'ed or Rx'ed
   uint8_t*      l2_payload;                     // pointer to the start of the payload of l2 (used for MAC to fill in ASN in ADV)
   //l1 (drivers)
   uint8_t       l1_txPower;                     // power for packet to Tx at
   int8_t        l1_rssi;                        // RSSI of received packet
   uint8_t       l1_lqi;                         // LQI of received packet
   bool          l1_crc;                         // did received packet pass CRC check?
   //the packet
   uint8_t       packet[1+1+125+2+1];            // 1B spi address, 1B length, 125B data, 2B CRC, 1B LQI
} OpenQueueEntry_t;

//=========================== variables =======================================

//=========================== prototypes ======================================

void openwsn_init();

#endif
