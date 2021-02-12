#include <stdio.h>
#include "idmanager.h"
#include "eui64.h"
#include "packetfunctions.h"
#include "neighbors.h"
#include "schedule.h"
#include "openserial.h"
#include "IEEE802154_security.h"

//=========================== typedefs =======================================

BEGIN_PACK
typedef struct {
    bool isDAGroot;
    uint8_t myPANID[2];
    uint8_t my16bID[2];
    uint8_t my64bID[8];
    uint8_t myPrefix[8];
} idManagerStatusEntry_t;
END_PACK

BEGIN_PACK
typedef struct {
    statusCtx_t statusMyID;
    statusCtx_t statusHasJoined;
} idManagerStatus_t;
END_PACK

//=========================== variables =======================================

idManagerVars_t id_manager_vars;
idManagerStatus_t id_manager_status_ctx;

//=========================== prototypes ======================================

static bool statusPrint_id(void);

static bool statusPrint_joined(void);

//=========================== public ==========================================

void idmanager_init(void) {

    // reset local variables
    memset(&id_manager_vars, 0, sizeof(idManagerVars_t));
    // this is used to not wakeup in non-activeslot
    id_manager_vars.slotSkip = FALSE;

    // isDAGroot
#if DAGROOT
    id_manager_vars.isDAGroot = TRUE;
#else
    id_manager_vars.isDAGroot = FALSE;
#endif

    // myPANID
    id_manager_vars.myPANID.type = ADDR_PANID;
    id_manager_vars.myPANID.addr_type.panid[0] = PANID_DEFINED & 0x00ff;
    id_manager_vars.myPANID.addr_type.panid[1] =(PANID_DEFINED & 0xff00) >> 8;

    // myPrefix
    id_manager_vars.myPrefix.type = ADDR_PREFIX;
#if DAGROOT
    id_manager_vars.myPrefix.addr_type.prefix[0]  = 0xbb;
    id_manager_vars.myPrefix.addr_type.prefix[1]  = 0xbb;
    id_manager_vars.myPrefix.addr_type.prefix[2]  = 0x00;
    id_manager_vars.myPrefix.addr_type.prefix[3]  = 0x00;
    id_manager_vars.myPrefix.addr_type.prefix[4]  = 0x00;
    id_manager_vars.myPrefix.addr_type.prefix[5]  = 0x00;
    id_manager_vars.myPrefix.addr_type.prefix[6]  = 0x00;
    id_manager_vars.myPrefix.addr_type.prefix[7]  = 0x00;
#else
    // set prefix to link-local
    id_manager_vars.myPrefix.addr_type.prefix[0] = 0xfe;
    id_manager_vars.myPrefix.addr_type.prefix[1] = 0x80;
    id_manager_vars.myPrefix.addr_type.prefix[2] = 0x00;
    id_manager_vars.myPrefix.addr_type.prefix[3] = 0x00;
    id_manager_vars.myPrefix.addr_type.prefix[4] = 0x00;
    id_manager_vars.myPrefix.addr_type.prefix[5] = 0x00;
    id_manager_vars.myPrefix.addr_type.prefix[6] = 0x00;
    id_manager_vars.myPrefix.addr_type.prefix[7] = 0x00;
#endif

    // my64bID
    id_manager_vars.my64bID.type = ADDR_64B;
    eui64_get(id_manager_vars.my64bID.addr_type.addr_64b);

    // my16bID
    packetfunctions_mac64bToMac16b(&id_manager_vars.my64bID, &id_manager_vars.my16bID);

    // set callbacks in drivers (prevents layer violations)
    openserial_setCb(idmanager_triggerAboutRoot, CB_ROOT);
    openserial_setCb(idmanager_getMyID, CB_ADDR);

    id_manager_status_ctx.statusHasJoined.id = STATUS_JOINED;
    id_manager_status_ctx.statusHasJoined.statusPrint_cb = statusPrint_joined;
    openserial_appendStatusCtx(&id_manager_status_ctx.statusHasJoined);

    id_manager_status_ctx.statusMyID.id = STATUS_ID;
    id_manager_status_ctx.statusMyID.statusPrint_cb = statusPrint_id;
    openserial_appendStatusCtx(&id_manager_status_ctx.statusMyID);

}

bool idmanager_getIsDAGroot(void) {
    bool res;
    INTERRUPT_DECLARATION();

    DISABLE_INTERRUPTS();
    res = id_manager_vars.isDAGroot;
    ENABLE_INTERRUPTS();
    return res;
}

void idmanager_setIsDAGroot(bool newRole) {
    INTERRUPT_DECLARATION();
    DISABLE_INTERRUPTS();
    id_manager_vars.isDAGroot = newRole;
    icmpv6rpl_updateMyDAGrankAndParentSelection();
    schedule_startDAGroot();
    ENABLE_INTERRUPTS();
}

bool idmanager_getIsSlotSkip(void) {
    bool res;
    INTERRUPT_DECLARATION();

    DISABLE_INTERRUPTS();
    res = id_manager_vars.slotSkip;
    ENABLE_INTERRUPTS();
    return res;
}

open_addr_t* idmanager_getMyID(uint8_t type) {
    open_addr_t *res;

    INTERRUPT_DECLARATION();
    DISABLE_INTERRUPTS();
    switch (type) {
        case ADDR_16B:
            res = &id_manager_vars.my16bID;
            break;
        case ADDR_64B:
            res = &id_manager_vars.my64bID;
            break;
        case ADDR_PANID:
            res = &id_manager_vars.myPANID;
            break;
        case ADDR_PREFIX:
            res = &id_manager_vars.myPrefix;
            break;
        case ADDR_128B:
            // build full IPv6 address from prefix and 64b ID.
        default:
            LOG_CRITICAL(COMPONENT_IDMANAGER, ERR_WRONG_ADDR_TYPE, (errorparameter_t) type, (errorparameter_t) 0);
            res = NULL;
            break;
    }
    ENABLE_INTERRUPTS();
    return res;
}

owerror_t idmanager_setMyID(open_addr_t *newID) {
    INTERRUPT_DECLARATION();
    DISABLE_INTERRUPTS();
    switch (newID->type) {
        case ADDR_16B:
            memcpy(&id_manager_vars.my16bID, newID, sizeof(open_addr_t));
            break;
        case ADDR_64B:
            memcpy(&id_manager_vars.my64bID, newID, sizeof(open_addr_t));
            break;
        case ADDR_PANID:
            memcpy(&id_manager_vars.myPANID, newID, sizeof(open_addr_t));
            break;
        case ADDR_PREFIX:
            memcpy(&id_manager_vars.myPrefix, newID, sizeof(open_addr_t));
            break;
        case ADDR_128B:
            //don't set 128b, but rather prefix and 64b
        default:
            LOG_CRITICAL(COMPONENT_IDMANAGER, ERR_WRONG_ADDR_TYPE,
                         (errorparameter_t) newID->type,
                         (errorparameter_t) 1);
            ENABLE_INTERRUPTS();
            return E_FAIL;
    }
    ENABLE_INTERRUPTS();
    return E_SUCCESS;
}

bool idmanager_isMyAddress(open_addr_t *addr) {
    open_addr_t temp_my128bID;
    bool res;
    INTERRUPT_DECLARATION();
    DISABLE_INTERRUPTS();

    switch (addr->type) {
        case ADDR_16B:
            res = packetfunctions_sameAddress(addr, &id_manager_vars.my16bID);
            ENABLE_INTERRUPTS();
            return res;
        case ADDR_64B:
            res = packetfunctions_sameAddress(addr, &id_manager_vars.my64bID);
            ENABLE_INTERRUPTS();
            return res;
        case ADDR_128B:
            // build temporary my128bID
            temp_my128bID.type = ADDR_128B;
            memcpy(&temp_my128bID.addr_type.addr_128b[0], &id_manager_vars.myPrefix.addr_type.prefix, 8);
            memcpy(&temp_my128bID.addr_type.addr_128b[8], &id_manager_vars.my64bID.addr_type.addr_64b, 8);

            res = packetfunctions_sameAddress(addr, &temp_my128bID);
            ENABLE_INTERRUPTS();
            return res;
        case ADDR_PANID:
            res = packetfunctions_sameAddress(addr, &id_manager_vars.myPANID);
            ENABLE_INTERRUPTS();
            return res;
        case ADDR_PREFIX:
            res = packetfunctions_sameAddress(addr, &id_manager_vars.myPrefix);
            ENABLE_INTERRUPTS();
            return res;
        default:
            LOG_CRITICAL(COMPONENT_IDMANAGER, ERR_WRONG_ADDR_TYPE,
                         (errorparameter_t) addr->type,
                         (errorparameter_t) 2);
            ENABLE_INTERRUPTS();
            return FALSE;
    }
}

void idmanager_triggerAboutRoot(void) {
    uint8_t number_bytes_from_input_buffer;
    uint8_t input_buffer[1 + 8 + 1 + 16];
    open_addr_t myPrefix;
    uint8_t dodagid[16];
    uint8_t keyIndex;
    uint8_t *keyValue;

    //=== get command from OpenSerial
    number_bytes_from_input_buffer = openserial_getInputBuffer(input_buffer, sizeof(input_buffer));
    if (number_bytes_from_input_buffer != sizeof(input_buffer)) {
        LOG_ERROR(COMPONENT_IDMANAGER, ERR_INPUTBUFFER_LENGTH,
                  (errorparameter_t) number_bytes_from_input_buffer,
                  (errorparameter_t) 0);
        return;
    };

    //=== handle command

    // take action (byte 0)
    switch (input_buffer[0]) {
        case ACTION_YES:
            idmanager_setIsDAGroot(TRUE);
            id_manager_vars.slotSkip = FALSE;
            break;
        case ACTION_NO:
            idmanager_setIsDAGroot(FALSE);
            id_manager_vars.slotSkip = TRUE;
            break;
        case ACTION_TOGGLE:
            if (idmanager_getIsDAGroot()) {
                idmanager_setIsDAGroot(FALSE);
                id_manager_vars.slotSkip = TRUE;
            } else {
                idmanager_setIsDAGroot(TRUE);
                id_manager_vars.slotSkip = FALSE;
            }
            break;
        default:
            LOG_ERROR(COMPONENT_IDMANAGER, ERR_INVALID_PARAM, (errorparameter_t)0, (errorparameter_t)0);
            return;
    }

    // store prefix (bytes 1-8)
    myPrefix.type = ADDR_PREFIX;
    memcpy(myPrefix.addr_type.prefix,&input_buffer[1], sizeof(myPrefix.addr_type.prefix));
    idmanager_setMyID(&myPrefix);

    // indicate DODAGid to RPL
    memcpy(&dodagid[0], id_manager_vars.myPrefix.addr_type.prefix, 8);  // prefix
    memcpy(&dodagid[8], id_manager_vars.my64bID.addr_type.addr_64b, 8); // eui64
    icmpv6rpl_writeDODAGid(dodagid);

    // store L2 security key index and key value
    // for the moment, keys K1 and K2 are the same
    keyIndex = input_buffer[9];
    keyValue = &input_buffer[10];
    IEEE802154_security_setBeaconKey(keyIndex, keyValue);
    IEEE802154_security_setDataKey(keyIndex, keyValue);
}

void idmanager_setJoinKey(uint8_t *key) {
    memcpy(id_manager_vars.joinKey, key, 16);
}

void idmanager_getJoinKey(uint8_t **pKey) {
    *pKey = id_manager_vars.joinKey;
}

void idmanager_setJoinAsn(asn_t *asn) {
    memcpy(&id_manager_vars.joinAsn, asn, sizeof(asn_t));
}

/**
\brief Trigger this module to print status information, over serial.

debugPrint_* functions are used by the openserial module to continuously print
status information about several modules in the OpenWSN stack.

\returns TRUE if this function printed something, FALSE otherwise.
*/
static bool statusPrint_id(void) {
    idManagerStatusEntry_t status_entry;

    status_entry.isDAGroot = id_manager_vars.isDAGroot;
    memcpy(status_entry.myPANID, id_manager_vars.myPANID.addr_type.panid, 2);
    memcpy(status_entry.my16bID, id_manager_vars.my16bID.addr_type.addr_16b, 2);
    memcpy(status_entry.my64bID, id_manager_vars.my64bID.addr_type.addr_64b, 8);
    memcpy(status_entry.myPrefix, id_manager_vars.myPrefix.addr_type.prefix, 8);

    openserial_printStatus(STATUS_ID, (uint8_t * ) & status_entry, sizeof(idManagerStatusEntry_t));

    return TRUE;
}

static bool statusPrint_joined(void) {
    asn_t asn_array;

    asn_array.byte4 = id_manager_vars.joinAsn.byte4;
    asn_array.bytes2and3 = id_manager_vars.joinAsn.bytes2and3;
    asn_array.bytes0and1 = id_manager_vars.joinAsn.bytes0and1;

    openserial_printStatus(STATUS_JOINED, (uint8_t * ) & asn_array, sizeof(asn_array));

    return TRUE;
}

//=========================== private =========================================
