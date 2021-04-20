#ifndef OPENWSN_COAP_H
#define OPENWSN_COAP_H

/**
\addtogroup Transport
\{
\addtogroup Coap
\{
*/

#include "config.h"
#include "sock.h"
#include "async.h"

//=========================== define ==========================================

/// the maximum number of options in a RX'ed CoAP message
#define MAX_COAP_OPTIONS               (10) // 3 before but we want GET requests with more options

// This value may be reduced as a memory optimization, but would invalidate spec compliance
#define COAP_MAX_TKL                   (8)

#define COAP_MAX_MSG_LEN               (IPV6_PACKET_SIZE - 20)

#define COAP_PAYLOAD_MARKER            (0xFF)

#define COAP_VERSION                   (1)

// OSCOAP related defines

#define OSCOAP_MAX_ID_LEN              (10)

#define OSCOAP_MASTER_SECRET_LEN       (16)

#define OSCORE_OPT_MAX_LEN             (1 + 2 + 1 + OSCOAP_MAX_ID_LEN + OSCOAP_MAX_ID_LEN)

#define AES_CCM_16_64_128              (10)   // algorithm value as defined in COSE spec

#define AES_CCM_16_64_128_KEY_LEN      (16)

#define AES_CCM_16_64_128_IV_LEN       (13)

#define AES_CCM_16_64_128_TAG_LEN      (8)

#define STATELESS_PROXY_STATE_LEN      (1 + 16 + 2) // seq no, ipv6 address, port number
#define STATELESS_PROXY_TAG_LEN        (4)

typedef enum {
    COAP_TYPE_CON = 0,
    COAP_TYPE_NON = 1,
    COAP_TYPE_ACK = 2,
    COAP_TYPE_RES = 3,
} coap_type_t;

typedef enum {
    COAP_CODE_EMPTY = 0,
    // request
    COAP_CODE_REQ_GET = 1,
    COAP_CODE_REQ_POST = 2,
    COAP_CODE_REQ_PUT = 3,
    COAP_CODE_REQ_DELETE = 4,
    // response
    // - OK
    COAP_CODE_RESP_CREATED = 65,
    COAP_CODE_RESP_DELETED = 66,
    COAP_CODE_RESP_VALID = 67,
    COAP_CODE_RESP_CHANGED = 68,
    COAP_CODE_RESP_CONTENT = 69,
    // - not OK
    COAP_CODE_RESP_BADREQ = 128,
    COAP_CODE_RESP_UNAUTHORIZED = 129,
    COAP_CODE_RESP_BADOPTION = 130,
    COAP_CODE_RESP_FORBIDDEN = 131,
    COAP_CODE_RESP_NOTFOUND = 132,
    COAP_CODE_RESP_METHODNOTALLOWED = 133,
    COAP_CODE_RESP_PRECONDFAILED = 140,
    COAP_CODE_RESP_REQTOOLARGE = 141,
    COAP_CODE_RESP_UNSUPPMEDIATYPE = 143,
    // - error
    COAP_CODE_RESP_SERVERERROR = 160,
    COAP_CODE_RESP_NOTIMPLEMENTED = 161,
    COAP_CODE_RESP_BADGATEWAY = 162,
    COAP_CODE_RESP_UNAVAILABLE = 163,
    COAP_CODE_RESP_GWTIMEOUT = 164,
    COAP_CODE_RESP_PROXYINGNOTSUPP = 165,
} coap_code_t;

typedef enum {
    COAP_OPTION_NONE = 0,
    COAP_OPTION_NUM_IFMATCH = 1,
    COAP_OPTION_NUM_URIHOST = 3,
    COAP_OPTION_NUM_ETAG = 4,
    COAP_OPTION_NUM_IFNONEMATCH = 5,
    COAP_OPTION_NUM_URIPORT = 7,
    COAP_OPTION_NUM_LOCATIONPATH = 8,
    COAP_OPTION_NUM_OSCORE = 9,
    COAP_OPTION_NUM_URIPATH = 11,
    COAP_OPTION_NUM_CONTENTFORMAT = 12,
    COAP_OPTION_NUM_MAXAGE = 14,
    COAP_OPTION_NUM_URIQUERY = 15,
    COAP_OPTION_NUM_ACCEPT = 16,
    COAP_OPTION_NUM_LOCATIONQUERY = 20,
    COAP_OPTION_NUM_PROXYURI = 35,
    COAP_OPTION_NUM_PROXYSCHEME = 39,
    COAP_OPTION_NUM_STATELESSPROXY = 40,
} coap_option_t;

typedef enum {
    COAP_OPTION_CLASS_E = 0,
    COAP_OPTION_CLASS_I = 1,
    COAP_OPTION_CLASS_U = 2,
    COAP_OPTION_CLASS_ALL = 3,
} coap_option_class_t;

typedef enum {
    COAP_MEDTYPE_TEXTPLAIN = 0,
    COAP_MEDTYPE_APPLINKFORMAT = 40,
    COAP_MEDTYPE_APPXML = 41,
    COAP_MEDTYPE_APPOCTETSTREAM = 42,
    COAP_MEDTYPE_APPEXI = 47,
    COAP_MEDTYPE_APPJSON = 50,
    COAP_MEDTYPE_APPCBOR = 60,
} coap_media_type_t;

//=========================== typedef =========================================

typedef struct {
    uint8_t Ver;
    coap_type_t T;
    uint8_t TKL;
    coap_code_t Code;
    uint16_t messageID;
    uint8_t token[COAP_MAX_TKL];
    uint16_t oscoreSeqNum;
} coap_header_iht;

typedef struct {
    coap_option_t type;
    uint8_t length;
    uint8_t *pValue;
} coap_option_iht;

typedef struct {
    uint32_t bitArray;
    uint16_t rightEdge;
} replay_window_t;

typedef struct {
    // common context
    uint8_t aeadAlgorithm;
    uint8_t commonIV[AES_CCM_16_64_128_IV_LEN];
    uint8_t idContext[OSCOAP_MAX_ID_LEN];
    uint8_t idContextLen;
    // sender context 
    uint8_t senderID[OSCOAP_MAX_ID_LEN];
    uint8_t senderIDLen;
    uint8_t senderKey[AES_CCM_16_64_128_KEY_LEN];
    uint16_t sequenceNumber;
    // recipient context
    uint8_t recipientID[OSCOAP_MAX_ID_LEN];
    uint8_t recipientIDLen;
    uint8_t recipientKey[AES_CCM_16_64_128_KEY_LEN];
    replay_window_t window;
} oscore_security_context_t;

typedef owerror_t (*callbackRx_cbt)(OpenQueueEntry_t *msg,
                                    coap_header_iht *coap_header,
                                    coap_option_iht *coap_incomingOptions,
                                    coap_option_iht *coap_outgoingOptions,
                                    uint8_t *coap_outgoingOptionsLen);

typedef void (*callbackSendDone_cbt)(OpenQueueEntry_t *msg,
                                     owerror_t error);

typedef struct coap_resource_desc_t coap_resource_desc_t;

struct coap_resource_desc_t {
    uint8_t path0len;
    uint8_t *path0val;
    uint8_t path1len;
    uint8_t *path1val;
    uint8_t componentID;
    oscore_security_context_t *securityContext;
    bool discoverable;
    callbackRx_cbt callbackRx;
    callbackSendDone_cbt callbackSendDone;
    coap_header_iht last_request;
    coap_resource_desc_t *next;
};

typedef struct {
    uint8_t key[16];
    uint8_t buffer[STATELESS_PROXY_STATE_LEN + STATELESS_PROXY_TAG_LEN];
    uint8_t sequenceNumber;
} coap_statelessproxy_vars_t;

//=========================== module variables ================================

typedef struct {
    coap_resource_desc_t *resources;
    bool busySending;
    uint8_t delayCounter;
    uint16_t messageID;
    coap_statelessproxy_vars_t statelessProxy;
    sock_udp_t sock;
} coap_vars_t;

//=========================== prototypes ======================================

// from stack
void coap_init(void);

// from CoAP resources
void coap_writeLinks(OpenQueueEntry_t *msg, uint8_t componentID);

void coap_register(coap_resource_desc_t *desc);

owerror_t coap_send(
        OpenQueueEntry_t *msg,
        coap_type_t type,
        coap_code_t code,
        uint8_t TKL,
        coap_option_iht *options,
        uint8_t optionsLen,
        coap_resource_desc_t *descSender
);

// option handling for OSCORE
coap_option_class_t coap_get_option_class(coap_option_t type);

uint8_t coap_options_encode(OpenQueueEntry_t *msg,
                            coap_option_iht *options,
                            uint8_t optionsLen,
                            coap_option_class_t class);

uint8_t coap_options_parse(uint8_t *buffer,
                           uint8_t bufferLen,
                           coap_option_iht *options,
                           uint8_t *optionsLen);

uint8_t coap_find_option(coap_option_iht *array, uint8_t arrayLen, coap_option_t option, uint8_t *startIndex);

/**
\}
\}
*/

#endif /* OPENWSN_COAP_H */
