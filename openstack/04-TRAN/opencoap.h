#ifndef __OPENCOAP_H
#define __OPENCOAP_H

/**
\addtogroup Transport
\{
\addtogroup openCoap
\{
*/

#include "opentimers.h"

//=========================== define ==========================================

// IPv6 addresses of servers on the Internet
static const uint8_t ipAddr_ipsoRD[]    = {0x26, 0x07, 0xf7, 0x40, 0x00, 0x00, 0x00, 0x3f, \
                                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x29};
static const uint8_t ipAddr_motesEecs[] = {0x20, 0x01, 0x04, 0x70, 0x00, 0x66, 0x00, 0x19, \
                                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02};
static const uint8_t ipAddr_local[]     = {0x26, 0x07, 0xf1, 0x40, 0x04, 0x00, 0x10, 0x36, \
                                           0x4d, 0xcd, 0xab, 0x54, 0x81, 0x99, 0xc1, 0xf7}; 
                                           
static const uint8_t ipAddr_motedata[]  = {0x20, 0x01, 0x04, 0x70, 0x00, 0x66, 0x00, 0x17, \
                                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02};

//bbbb::1415:92cc:0:3 PORT:5683
static const uint8_t ipAddr_ringmaster[] = {0xbb, 0xbb, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};

/// the maximum number of options in a RX'ed CoAP message
#define MAX_COAP_OPTIONS               10 //3 before but we want gets with more options

// This value may be reduced as a memory optimization, but would invalidate spec compliance
#define COAP_MAX_TKL                   8

#define COAP_PAYLOAD_MARKER            0xFF

#define COAP_VERSION                   1

typedef enum {
   COAP_TYPE_CON                       = 0,
   COAP_TYPE_NON                       = 1,
   COAP_TYPE_ACK                       = 2,
   COAP_TYPE_RES                       = 3,
} coap_type_t;

typedef enum {
   COAP_CODE_EMPTY                     = 0,
   // request
   COAP_CODE_REQ_GET                   = 1,
   COAP_CODE_REQ_POST                  = 2,
   COAP_CODE_REQ_PUT                   = 3,
   COAP_CODE_REQ_DELETE                = 4,
   // response
   // - OK
   COAP_CODE_RESP_CREATED              = 65,
   COAP_CODE_RESP_DELETED              = 66,
   COAP_CODE_RESP_VALID                = 67,
   COAP_CODE_RESP_CHANGED              = 68,
   COAP_CODE_RESP_CONTENT              = 69,
   // - not OK
   COAP_CODE_RESP_BADREQ               = 128,
   COAP_CODE_RESP_UNAUTHORIZED         = 129,
   COAP_CODE_RESP_BADOPTION            = 130,
   COAP_CODE_RESP_FORBIDDEN            = 131,
   COAP_CODE_RESP_NOTFOUND             = 132,
   COAP_CODE_RESP_METHODNOTALLOWED     = 133,
   COAP_CODE_RESP_PRECONDFAILED        = 140,
   COAP_CODE_RESP_REQTOOLARGE          = 141,
   COAP_CODE_RESP_UNSUPPMEDIATYPE      = 143,
   // - error
   COAP_CODE_RESP_SERVERERROR          = 160,
   COAP_CODE_RESP_NOTIMPLEMENTED       = 161,
   COAP_CODE_RESP_BADGATEWAY           = 162,
   COAP_CODE_RESP_UNAVAILABLE          = 163,
   COAP_CODE_RESP_GWTIMEOUT            = 164,
   COAP_CODE_RESP_PROXYINGNOTSUPP      = 165,
} coap_code_t;

typedef enum {
   COAP_OPTION_NONE                    = 0,
   COAP_OPTION_NUM_IFMATCH             = 1,
   COAP_OPTION_NUM_URIHOST             = 3,
   COAP_OPTION_NUM_ETAG                = 4,
   COAP_OPTION_NUM_IFNONEMATCH         = 5,
   COAP_OPTION_NUM_URIPORT             = 7,
   COAP_OPTION_NUM_LOCATIONPATH        = 8,
   COAP_OPTION_NUM_URIPATH             = 11,
   COAP_OPTION_NUM_CONTENTFORMAT       = 12,
   COAP_OPTION_NUM_MAXAGE              = 14,
   COAP_OPTION_NUM_URIQUERY            = 15,
   COAP_OPTION_NUM_ACCEPT              = 16,
   COAP_OPTION_NUM_LOCATIONQUERY       = 20,
   COAP_OPTION_NUM_PROXYURI            = 35,
   COAP_OPTION_NUM_PROXYSCHEME         = 39,
} coap_option_t;

typedef enum {
   COAP_MEDTYPE_TEXTPLAIN              =  0,
   COAP_MEDTYPE_APPLINKFORMAT          = 40,
   COAP_MEDTYPE_APPXML                 = 41,
   COAP_MEDTYPE_APPOCTETSTREAM         = 42,
   COAP_MEDTYPE_APPEXI                 = 47,
   COAP_MEDTYPE_APPJSON                = 50,
} coap_media_type_t;

//=========================== typedef =========================================

typedef struct {
   uint8_t       Ver;
   coap_type_t   T;
   uint8_t       TKL;
   coap_code_t   Code;
   uint16_t      messageID;
   uint8_t       token[COAP_MAX_TKL];
} coap_header_iht;

typedef struct {
   coap_option_t type;
   uint8_t       length;
   uint8_t*      pValue;
} coap_option_iht;

typedef owerror_t (*callbackRx_cbt)(OpenQueueEntry_t* msg,
                                coap_header_iht*  coap_header,
                                coap_option_iht*  coap_options);
typedef void (*callbackSendDone_cbt)(OpenQueueEntry_t* msg,
                                      owerror_t error);

typedef struct coap_resource_desc_t coap_resource_desc_t;

struct coap_resource_desc_t {
   uint8_t               path0len;
   uint8_t*              path0val;
   uint8_t               path1len;
   uint8_t*              path1val;
   uint8_t               componentID;
   callbackRx_cbt        callbackRx;
   callbackSendDone_cbt  callbackSendDone;
   coap_header_iht       last_request;
   coap_resource_desc_t* next;
};

//=========================== module variables ================================

typedef struct {
   coap_resource_desc_t* resources;
   bool                  busySending;
   uint8_t               delayCounter;
   uint16_t              messageID;
} opencoap_vars_t;

//=========================== prototypes ======================================

// from stack
void          opencoap_init(void);
void          opencoap_receive(OpenQueueEntry_t* msg);
void          opencoap_sendDone(OpenQueueEntry_t* msg, owerror_t error);

// from CoAP resources
void          opencoap_writeLinks(OpenQueueEntry_t* msg);
void          opencoap_register(coap_resource_desc_t* desc);
owerror_t     opencoap_send(
    OpenQueueEntry_t*     msg,
    coap_type_t           type,
    coap_code_t           code,
    uint8_t               numOptions,
    coap_resource_desc_t* descSender
);

/**
\}
\}
*/

#endif
