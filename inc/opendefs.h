/**
\brief General OpenWSN definitions

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, August 2010
\author Ankur Mehta <mehtank@eecs.berkeley.edu>, September 2010
\author Savio Sciancalepore <savio.sciancalepore@poliba.it>, TelematicsLab April 2015
\author Giuseppe Piro <giuseppe.piro@poliba.it>,
\author Gennaro Boggia <gennaro.boggia@poliba.it>,
\author Luigi Alfredo Grieco <alfredo.grieco@poliba.it>
\author Timothy Claeys <timothy.claeys@gmail.com>
*/

#ifndef OPENWSN_OPENDEFS_H
#define OPENWSN_OPENDEFS_H

// general
#include <stdint.h>               // needed for uin8_t, uint16_t
#include "config.h"
#include "toolchain_defs.h"
#include "board_info.h"
#include "af.h"


//=========================== define ==========================================

static const uint8_t infoStackName[] = "OpenWSN ";
#define OPENWSN_VERSION_MAJOR     1
#define OPENWSN_VERSION_MINOR     25
#define OPENWSN_VERSION_PATCH     0

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define LENGTH_ADDR16b   2
#define LENGTH_ADDR64b   8
#define LENGTH_ADDR128b  16

#define MAXNUMNEIGHBORS  30

// maximum celllist length
#define CELLLIST_MAX_LEN 5

// frame sizes
#define IEEE802154_FRAME_SIZE   127

#if OPENWSN_6LO_FRAGMENTATION_C
#define IPV6_PACKET_SIZE        MAX_PKTSIZE_SUPPORTED
#else
#define IPV6_PACKET_SIZE        IEEE802154_FRAME_SIZE
#endif

enum {
    E_SUCCESS = 0,
    E_FAIL = 1,
};

// types of addresses
enum {
    ADDR_NONE = 0,
    ADDR_16B = 1,
    ADDR_64B = 2,
    ADDR_128B = 3,
    ADDR_PANID = 4,
    ADDR_PREFIX = 5,
    ADDR_ANYCAST = 6,
};

enum {
    OW_LITTLE_ENDIAN = TRUE,
    OW_BIG_ENDIAN = FALSE,
};

// protocol numbers, as defined by the IANA
enum {
    IANA_IPv6HOPOPT = 0x00,
    IANA_UDP = 0x11,
    IANA_IPv6ROUTING = 0x03,
    IANA_IPv6ROUTE = 0x2b,//used for source routing
    IANA_ICMPv6 = 0x3a,
    IANA_ICMPv6_ECHO_REQUEST = 128,
    IANA_ICMPv6_ECHO_REPLY = 129,
    IANA_ICMPv6_RS = 133,
    IANA_ICMPv6_RA = 134,
    IANA_ICMPv6_RA_PREFIX_INFORMATION = 3,
    IANA_ICMPv6_RPL = 155,
    IANA_ICMPv6_RPL_DIS = 0x00,
    IANA_ICMPv6_RPL_DIO = 0x01,
    IANA_ICMPv6_RPL_DAO = 0x02,
    IANA_RSVP = 46,
    IANA_UNDEFINED = 250, //use an unassigned
};

// well known ports (which we define)
// warning: first 4 MSB of 2° octet may coincide with previous protocol number
enum {
    //UDP
    WKP_UDP_COAP = OPENWSN_COAP_PORT_DEFAULT,
    WKP_UDP_ECHO = 7,
    WKP_UDP_EXPIRATION = 5,
    WKP_UDP_MONITOR = 3,
    WKP_UDP_INJECT = 61617,
    WKP_UDP_RINGMASTER = 15000,
    WKP_UDP_SERIALBRIDGE = 2001,
};

//status elements
enum {
    STATUS_ISSYNC = 0,
    STATUS_ID,
    STATUS_DAGRANK,
    STATUS_ASN,
    STATUS_MACSTATS,
    STATUS_SCHEDULE,
    STATUS_BACKOFF,
    STATUS_QUEUE,
    STATUS_NEIGHBORS,
    STATUS_KAPERIOD,
    STATUS_JOINED,
    STATUS_MSF,
    STATUS_MAX,
};

// component identifiers, order is important
enum {
    COMPONENT_NULL = 0x00,
    COMPONENT_OPENWSN = 0x01,
    //cross-layers
    COMPONENT_IDMANAGER = 0x02,
    COMPONENT_OPENQUEUE = 0x03,
    COMPONENT_OPENSERIAL = 0x04,
    COMPONENT_PACKETFUNCTIONS = 0x05,
    COMPONENT_RANDOM = 0x06,
    //PHY
    COMPONENT_RADIO = 0x07,
    //MAClow
    COMPONENT_IEEE802154 = 0x08,
    COMPONENT_IEEE802154E = 0x09,

    // all components with higher component id than COMPONENT_IEEE802154E
    // won't be able to get free packets from the queue
    // when the mote is not synch

    //MAClow<->MAChigh ("virtual components")
    COMPONENT_SIXTOP_TO_IEEE802154E = 0x0a,
    COMPONENT_IEEE802154E_TO_SIXTOP = 0x0b,
    //MAChigh
    COMPONENT_SIXTOP = 0x0c,
    COMPONENT_NEIGHBORS = 0x0d,
    COMPONENT_SCHEDULE = 0x0e,
    COMPONENT_SIXTOP_RES = 0x0f,
    COMPONENT_MSF = 0x10,
    //IPHC
    COMPONENT_OPENBRIDGE = 0x11,
    COMPONENT_IPHC = 0x12,
    COMPONENT_FRAG = 0x13,
    //IPv6
    COMPONENT_FORWARDING = 0x14,
    COMPONENT_ICMPv6 = 0x15,
    COMPONENT_ICMPv6ECHO = 0x16,
    COMPONENT_ICMPv6ROUTER = 0x17,
    COMPONENT_ICMPv6RPL = 0x18,
    //TRAN
    COMPONENT_UDP = 0x19,
    COMPONENT_SOCK_TO_UDP = 0x1a,
    COMPONENT_UDP_TO_SOCK = 0x1b,
    COMPONENT_OPENCOAP = 0x1c,
    // secure join
    COMPONENT_CJOIN = 0x1d,
    COMPONENT_OSCORE = 0x1e,
    // applications
    COMPONENT_C6T = 0x1f,
    COMPONENT_CEXAMPLE = 0x20,
    COMPONENT_CINFO = 0x21,
    COMPONENT_CLEDS = 0x22,
    COMPONENT_CSENSORS = 0x23,
    COMPONENT_CSTORM = 0x24,
    COMPONENT_CWELLKNOWN = 0x25,
    COMPONENT_UECHO = 0x26,
    COMPONENT_UINJECT = 0x27,
    COMPONENT_RRT = 0x28,
    COMPONENT_SECURITY = 0x29,
    COMPONENT_USERIALBRIDGE = 0x2a,
    COMPONENT_UEXPIRATION = 0x2b,
    COMPONENT_UMONITOR = 0x2c,
    COMPONENT_CINFRARED = 0x2d,
};

/**
\brief error codes used throughout the OpenWSN stack

\note The comments are used in the Python parsing tool:
   - {0} refers to the value of the first argument,
   - {1} refers to the value of the second argument,
*/
enum {
    // l7
    ERR_JOINED = 0x01,                          // node joined
    ERR_JOIN_REQUEST = 0x02,                    // sending CJOIN request
    ERR_SEQUENCE_NUMBER_OVERFLOW = 0x03,        // OSCORE sequence number reached maximum value
    ERR_REPLAY_FAILED = 0x04,                   // OSCORE replay protection failed
    ERR_DECRYPTION_FAILED = 0x05,               // OSCORE decryption and tag verification failed
    ERR_ABORT_JOIN_PROCESS = 0x06,              // Aborted join process (code location {0})
    // l4
    ERR_WRONG_TRAN_PROTOCOL = 0x07,             // unknown transport protocol {0} (code location {1})
    ERR_UNSUPPORTED_PORT_NUMBER = 0x08,         // unsupported port number {0} (code location {1})
    ERR_INVALID_CHECKSUM = 0x09,                // invalid checksum, expected 0x{:04x}, found 0x{:04x}
    // l3a/b
    ERR_RCVD_ECHO_REQUEST = 0x0a,               // received an echo request (length: {0})
    ERR_RCVD_ECHO_REPLY = 0x0b,                 // received an echo reply
    ERR_6LORH_DEADLINE_EXPIRED = 0x0c,          // the received packet has expired
    ERR_6LORH_DEADLINE_DROPPED = 0x0d,          // packet expiry time reached, dropped
    ERR_UNEXPECTED_DAO = 0x0e,                  // unexpected DAO (code location {0}). A change maybe happened on dagroot node.
    ERR_UNSUPPORTED_ICMPV6_TYPE = 0x0f,         // unsupported ICMPv6 type {0} (code location {1})
    ERR_6LOWPAN_UNSUPPORTED = 0x10,             // unsupported 6LoWPAN parameter {1} at location {0}, dropping packet
    ERR_NO_NEXTHOP = 0x11,                      // no next hop for layer 3 destination {0:x}{1:x}
    ERR_INVALID_FWDMODE = 0x12,                 // invalid forward mode
    ERR_LARGE_DAGRANK = 0x13,                   // large DAGrank {0}, set to {1}
    ERR_HOP_LIMIT_REACHED = 0x14,               // packet discarded hop limit reached
    ERR_LOOP_DETECTED = 0x15,                   // loop detected due to previous rank {0} lower than current node rank {1}
    ERR_WRONG_DIRECTION = 0x16,                 // upstream packet set to be downstream, possible loop.
    ERR_FORWARDING_PACKET_DROPPED = 0x17,       // packet to forward is dropped (code location {0})
    ERR_FRAG_INVALID_SIZE = 0x19,               // invalid original packet size ({0} > {1})
    ERR_FRAG_REASSEMBLED = 0x1a,                // reassembled fragments into big packet (size: {0}, tag: {1})
    ERR_FRAG_FAST_FORWARD = 0x1b,               // fast-forwarded all fragments with tag {0} (total size: {1})
    ERR_FRAG_STORED = 0x1c,                     // stored a fragment with offset {0} (currently in buffer: {1})
    ERR_FRAG_REASSEMBLY_OR_VRB_TIMEOUT = 0x1d,  // reassembly or vrb timer expired for fragments with tag {0}
    ERR_FRAG_FRAGMENTING = 0x1e,                // fragmenting a big packet, original size {0}, number of fragments {1}
    ERR_BRIDGE_MISMATCH = 0x1f,                 // bridge mismatch (code location {0})
    // l2b
    ERR_SCHEDULE_ADD_DUPLICATE_SLOT = 0x20,     // the slot {0} to be added is already in schedule
    ERR_NEIGHBORS_FULL = 0x21,                  // neighbors table is full (max number of neighbor is {0})
    ERR_NO_SENT_PACKET = 0x22,                  // there is no sent packet in queue
    ERR_NO_RECEIVED_PACKET = 0x23,              // there is no received packet in queue
    ERR_SCHEDULE_OVERFLOWN = 0x24,              // schedule overflown
    ERR_SIXTOP_RETURNCODE = 0x25,               // sixtop return code {0} at sixtop state {1}
    ERR_SIXTOP_REQUEST = 0x26,                  // sending a 6top request
    ERR_SIXTOP_COUNT = 0x27,                    // there are {0} cells to request mote
    ERR_SIXTOP_LIST = 0x28,                     // the cells reserved to request mote contains slot {0} and slot {1}
    ERR_UNSUPPORTED_FORMAT = 0x29,              // the received packet format is not supported (code location {0})
    ERR_UNSUPPORTED_METADATA = 0x2a,            // the metadata type is not suppored
    ERR_TX_CELL_USAGE = 0x2b,                   // TX cell usage during last period: {}
    ERR_RX_CELL_USAGE = 0x2c,                   // RX cell usage during last period: {}
    // l2a
    ERR_WRONG_CELLTYPE = 0x2d,                  // wrong celltype {0} at slotOffset {1}
    ERR_IEEE154_UNSUPPORTED = 0x2e,             // unsupported IEEE802.15.4 parameter {1} at location {0}
    ERR_DESYNCHRONIZED = 0x2f,                  // got desynchronized at slotOffset {0}
    ERR_SYNCHRONIZED = 0x30,                    // synchronized at slotOffset {0}
    ERR_LARGE_TIMECORRECTION = 0x31,            // large timeCorr.: {0} ticks (code loc. {1})
    ERR_WRONG_STATE_IN_ENDFRAME_SYNC = 0x32,    // wrong state {0} in end of frame+sync
    ERR_WRONG_STATE_IN_STARTSLOT = 0x33,        // wrong state {0} in startSlot, at slotOffset {1}
    ERR_WRONG_STATE_IN_TIMERFIRES = 0x34,       // wrong state {0} in timer fires, at slotOffset {1}
    ERR_WRONG_STATE_IN_NEWSLOT = 0x35,          // wrong state {0} in start of frame, at slotOffset {1}
    ERR_WRONG_STATE_IN_ENDOFFRAME = 0x36,       // wrong state {0} in end of frame, at slotOffset {1}
    ERR_MAXTXDATAPREPARE_OVERFLOW = 0x37,       // maxTxDataPrepare overflows while at state {0} in slotOffset {1}
    ERR_MAXRXACKPREPARE_OVERFLOWS = 0x38,       // maxRxAckPrepapare overflows while at state {0} in slotOffset {1}
    ERR_MAXRXDATAPREPARE_OVERFLOWS = 0x39,      // maxRxDataPrepapre overflows while at state {0} in slotOffset {1}
    ERR_MAXTXACKPREPARE_OVERFLOWS = 0x3a,       // maxTxAckPrepapre overflows while at state {0} in slotOffset {1}
    ERR_WDDATADURATION_OVERFLOWS = 0x3b,        // wdDataDuration overflows while at state {0} in slotOffset {1}
    ERR_WDRADIO_OVERFLOWS = 0x3c,               // wdRadio overflows while at state {0} in slotOffset {1}
    ERR_WDRADIOTX_OVERFLOWS = 0x3d,             // wdRadioTx overflows while at state {0} in slotOffset {1}
    ERR_WDACKDURATION_OVERFLOWS = 0x3e,         // wdAckDuration overflows while at state {0} in slotOffset {1}
    ERR_SECURITY = 0x3f,                        // security error on frameType {0}, code location {1}
    ERR_INVALID_PACKET_FROM_RADIO = 0x40,       // invalid packet from radio
    // drivers
    ERR_GETDATA_ASKS_TOO_FEW_BYTES = 0x41,      // getData asks for too few bytes, maxNumBytes={0}, fill level={1}
    ERR_WRONG_CRC_INPUT = 0x42,                 // wrong CRC in input Buffer
    // cross layer
    ERR_BUFFER_OVERFLOW = 0x43,                 // buffer overflow detected (code location {0})
    ERR_BUSY_SENDING = 0x44,                    // busy sending
    ERR_UNEXPECTED_SENDDONE = 0x45,             // sendDone for packet I didn't send
    ERR_NO_FREE_PACKET_BUFFER = 0x46,           // no free packet buffer (code location {0})
    ERR_NO_FREE_TIMER_OR_QUEUE_ENTRY = 0x47,    // no free timer or queue entry (code location {0})
    ERR_FREEING_UNUSED = 0x48,                  // freeing unused memory
    ERR_FREEING_ERROR = 0x49,                   // freeing memory unsupported memory
    ERR_MSG_UNKNOWN_TYPE = 0x4a,                // unknown message type {0}
    ERR_WRONG_ADDR_TYPE = 0x4b,                 // wrong address type {0} (code location {1})
    ERR_PACKET_TOO_LONG = 0x4c,                 // total packet size is too long, length {0} (adding {1} bytes)
    ERR_PACKET_TOO_SHORT = 0x4d,                // total packet size is too short, length {0} (removing {1} bytes)
    ERR_INPUTBUFFER_LENGTH = 0x4e,              // input length problem, length={0}
    ERR_BOOTED = 0x4f,                          // booted
    ERR_MAXRETRIES_REACHED = 0x50,              // maxretries reached (counter: {0})
    ERR_EMPTY_QUEUE_OR_UNKNOWN_TIMER = 0x51,    // empty queue or trying to remove unknown timer id (code location {0})
    ERR_PUSH_LOWER_LAYER = 0x52,                // failed to push to lower layer
    ERR_INVALID_PARAM = 0x53,                   // received an invalid parameter
    ERR_COPY_TO_SPKT = 0x54,                    // copy packet content to small packet (pkt len {} < max len {})
    ERR_COPY_TO_BPKT = 0x55,                    // copy packet content to big packet (pkt len {} > max len {})
    ERR_INIT_FAILURE = 0x56,                    // module initialization failure (failed to set callback {0})
};

//=========================== typedef =========================================


typedef uint16_t errorparameter_t;
typedef uint16_t dagrank_t;
typedef uint8_t owerror_t;

BEGIN_PACK
typedef struct {
    uint8_t byte4;
    uint16_t bytes2and3;
    uint16_t bytes0and1;
} asn_t;
END_PACK

typedef asn_t
        macFrameCounter_t;

BEGIN_PACK

typedef struct {
    bool isUsed;
    uint16_t slotoffset;
    uint16_t channeloffset;
} cellInfo_ht;

typedef struct {  // always written big endian, i.e. MSB in addr[0]
    uint8_t type;
    union {
        uint8_t addr_16b[2];
        uint8_t addr_64b[8];
        uint8_t addr_128b[16];
        uint8_t panid[2];
        uint8_t prefix[8];
    } addr_type;
} open_addr_t;
END_PACK

typedef struct {
    // admin
    uint8_t creator;                                           // the component which called getFreePacketBuffer()
    uint8_t owner;                                             // the component which currently owns the entry
    uint8_t *payload;                                          // pointer to the start of the payload within 'packet'
    int16_t length;                                            // length in bytes of the payload
    // l7
#if DEADLINE_OPTION
    uint16_t      max_delay;                                   // Max delay in milliseconds before which the packet should be delivered to the receiver
    bool          orgination_time_flag;
    bool          drop_flag;
#endif
    bool is_cjoin_response;
#if OPENWSN_6LO_FRAGMENTATION_C
    bool          is_big_packet;
#endif

    // l4
    uint8_t l4_protocol;                                       // l4 protocol to be used
    bool l4_protocol_compressed;                               // is the l4 protocol header compressed?
    uint16_t l4_sourcePortORicmpv6Type;                        // l4 source port
    uint16_t l4_destination_port;                              // l4 destination port
    uint8_t *l4_payload;                                       // pointer to the start of the payload of l4 (used for retransmits)
    uint8_t l4_length;                                         // length of the payload of l4 (used for retransmits)

    // l3
    open_addr_t l3_destinationAdd;                             // 128b IPv6 destination (down stack)
    open_addr_t l3_sourceAdd;                                  // 128b IPv6 source address
    bool l3_useSourceRouting;                                  // TRUE when the packet goes downstream

#if OPENWSN_6LO_FRAGMENTATION_C
    bool         l3_isFragment;
#endif
    // l2
    owerror_t l2_sendDoneError;                                // outcome of trying to send this packet
    open_addr_t l2_nextORpreviousHop;                          // 64b IEEE802.15.4 next (down stack) or previous (up) hop address
    uint8_t l2_frameType;                                      // beacon, data, ack, cmd
    uint8_t l2_dsn;                                            // sequence number of the received frame
    uint8_t l2_retriesLeft;                                    // number Tx retries left before packet dropped (dropped when hits 0)
    uint8_t l2_numTxAttempts;                                  // number Tx attempts
    asn_t l2_asn;                                              // at what ASN the packet was Tx'ed or Rx'ed
    uint8_t *l2_payload;                                       // pointer to the start of the payload of l2 (used for MAC to fill in ASN in ADV)
    cellInfo_ht l2_sixtop_celllist_add[CELLLIST_MAX_LEN];      // record celllist to be added and will be added when 6P response sendDone
    cellInfo_ht l2_sixtop_celllist_delete[CELLLIST_MAX_LEN];   // record celllist to be removed and will be removed when 6P response sendDone
    uint16_t l2_sixtop_frameID;                                // frameID in 6P message
    uint8_t l2_sixtop_messageType;                             // indicating the sixtop message type
    uint8_t l2_sixtop_command;                                 // command of the received 6p request, recorded in 6p response
    uint8_t l2_sixtop_cellOptions;                             // celloptions, used when 6p response senddone. (it's the same with cellOptions in 6p request but with TX and RX bits have been flipped)
    uint8_t l2_sixtop_returnCode;                              // return code in 6P response
    uint8_t *l2_ASNpayload;                                    // pointer to the ASN in EB
    uint8_t *l2_nextHop_payload;                               // pointer to the nexthop address in frame
    uint8_t l2_joinPriority;                                   // the join priority received in EB
    bool l2_IEListPresent;                                     // did have IE field?
    bool l2_payloadIEpresent;                                  // did I have payload IE field
    bool l2_joinPriorityPresent;
    bool l2_isNegativeACK;                                     // is the negative ACK?
    int16_t l2_timeCorrection;                                 // record the timeCorrection and print out at endOfslot
    bool l2_sendOnTxCell;                                      // mark the frame is sent on txCell
    // layer-2 security
    uint8_t l2_securityLevel;                                  // the security level specified for the current frame
    uint8_t l2_keyIdMode;                                      // the key Identifier mode specified for the current frame
    uint8_t l2_keyIndex;                                       // the key Index specified for the current frame
    open_addr_t l2_keySource;                                  // the key Source specified for the current frame
    uint8_t l2_authenticationLength;                           // the length of the authentication field
    uint8_t commandFrameIdentifier;                            // used in case of Command Frames
    uint8_t *l2_FrameCounter;                                  // pointer to the FrameCounter in the MAC header
    // l1 (drivers)
    uint8_t l1_txPower;                                        // power for packet to Tx at
    int8_t l1_rssi;                                            // RSSI of received packet
    uint8_t l1_lqi;                                            // LQI of received packet
    bool l1_crc;                                               // did received packet pass CRC check?
    // the packet
    uint8_t packet[1 + 1 + 125 + 2 + 1];                       // 1B spi address, 1B length, 125B data, 2B CRC, 1B LQI
} OpenQueueEntry_t;


#if OPENWSN_6LO_FRAGMENTATION_C
typedef struct {
    OpenQueueEntry_t standard_entry;
    // 130 bytes alread allocated in the normal OpenQueueEntry
    uint8_t packet_remainder[IPV6_PACKET_SIZE - 130];
} OpenQueueBigEntry_t;
#endif


BEGIN_PACK
typedef struct {
    bool used;
    bool insecure;
    uint8_t parentPreference;
    bool stableNeighbor;
    uint8_t switchStabilityCounter;
    open_addr_t addr_64b;
    dagrank_t DAGrank;
    int8_t rssi;
    uint8_t numRx;
    uint8_t numTx;
    uint8_t numTxACK;

    // number of times the tx counter wraps,can be removed if memory is a restriction, also check openvisualizer then.
    uint8_t numWraps;
    asn_t asn;
    uint8_t joinPrio;
    bool f6PNORES;
    uint8_t sequenceNumber;
    uint8_t backoffExponenton;
    uint8_t backoff;
} neighborRow_t;
END_PACK


//=========================== variables =======================================

//=========================== prototypes ======================================

#endif /* OPENWSN_OPENDEFS_H */
