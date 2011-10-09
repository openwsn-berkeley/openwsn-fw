#ifndef __OPENCOAP_H
#define __OPENCOAP_H

/**
\addtogroup Transport
\{
\addtogroup openCoap
\{
*/

//=========================== define ==========================================

#define MAX_COAP_OPTIONS               3

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
   COAP_OPTION_NONE                    =  0,
   COAP_OPTION_CONTENTTYPE             =  1,
   COAP_OPTION_MAXAGE                  =  2,
   COAP_OPTION_PROXYURI                =  3,
   COAP_OPTION_ETAG                    =  4,
   COAP_OPTION_URIHOST                 =  5,
   COAP_OPTION_LOCATIONPATH            =  6,
   COAP_OPTION_URIPORT                 =  7,
   COAP_OPTION_LOCATIONQUERY           =  8,
   COAP_OPTION_URIPATH                 =  9,
   COAP_OPTION_TOKEN                   = 11,
   COAP_OPTION_ACCEPT                  = 12,
   COAP_OPTION_IFMATCH                 = 13,
   COAP_OPTION_URIQUERY                = 15,
   COAP_OPTION_IFNONEMATCH             = 21,
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
   uint8_t       OC;
   coap_code_t   Code;
   uint8_t       MessageId[2];
} coap_header_iht;

typedef struct {
   coap_option_t type;
   uint8_t       length;
   uint8_t*      pValue;
} coap_option_iht;

typedef void (*callbackRx_t)(OpenQueueEntry_t* msg,
                             coap_header_iht*  coap_header,
                             coap_option_iht*  coap_options);
typedef void (*callbackTimer_t)(void);

typedef struct coap_resource_desc_t coap_resource_desc_t;
struct coap_resource_desc_t {
   uint8_t               path0len;
   uint8_t*              path0val;
   uint8_t               path1len;
   uint8_t*              path1val;
   bool                  messageIDused;
   uint16_t              messageID;
   callbackRx_t          callbackRx;
   callbackTimer_t       callbackTimer;
   coap_resource_desc_t* next;
};

//=========================== variables =======================================

//=========================== prototypes ======================================

void    opencoap_init();
void    opencoap_register(coap_resource_desc_t* desc);
error_t opencoap_send(OpenQueueEntry_t* msg);
void    opencoap_receive(OpenQueueEntry_t* msg);
void    opencoap_sendDone(OpenQueueEntry_t* msg, error_t error);

/**
\}
\}
*/

#endif
