#include "opendefs.h"
#include "idmanager.h"
#include "eui64.h"
#include "packetfunctions.h"
#include "openserial.h"
#include "neighbors.h"
#include "schedule.h"
#include "IEEE802154_security.h"

//=========================== variables =======================================

idmanager_vars_t idmanager_vars;

//=========================== prototypes ======================================

//=========================== public ==========================================

void idmanager_init(void) {

    // reset local variables
    memset(&idmanager_vars, 0, sizeof(idmanager_vars_t));
    // this is used to not wakeup in non-activeslot
    idmanager_vars.slotSkip = FALSE;

    // isDAGroot
#if DAGROOT
    idmanager_vars.isDAGroot = TRUE;
#else
    idmanager_vars.isDAGroot = FALSE;
#endif

    // myPANID
    idmanager_vars.myPANID.type = ADDR_PANID;
    idmanager_vars.myPANID.addr_type.panid[0] = PANID_DEFINED & 0x00ff;
    idmanager_vars.myPANID.addr_type.panid[1] =(PANID_DEFINED & 0xff00)>>8;

    // myPrefix
    idmanager_vars.myPrefix.type = ADDR_PREFIX;
#if DAGROOT
    idmanager_vars.myPrefix.prefix[0]  = 0xbb;
    idmanager_vars.myPrefix.prefix[1]  = 0xbb;
    idmanager_vars.myPrefix.prefix[2]  = 0x00;
    idmanager_vars.myPrefix.prefix[3]  = 0x00;
    idmanager_vars.myPrefix.prefix[4]  = 0x00;
    idmanager_vars.myPrefix.prefix[5]  = 0x00;
    idmanager_vars.myPrefix.prefix[6]  = 0x00;
    idmanager_vars.myPrefix.prefix[7]  = 0x00;
#else
    // set prefix to link-local
    idmanager_vars.myPrefix.addr_type.prefix[0] = 0xfe;
    idmanager_vars.myPrefix.addr_type.prefix[1] = 0x80;
    idmanager_vars.myPrefix.addr_type.prefix[2] = 0x00;
    idmanager_vars.myPrefix.addr_type.prefix[3] = 0x00;
    idmanager_vars.myPrefix.addr_type.prefix[4] = 0x00;
    idmanager_vars.myPrefix.addr_type.prefix[5] = 0x00;
    idmanager_vars.myPrefix.addr_type.prefix[6] = 0x00;
    idmanager_vars.myPrefix.addr_type.prefix[7] = 0x00;
#endif

    // my64bID
    idmanager_vars.my64bID.type = ADDR_64B;
    eui64_get(idmanager_vars.my64bID.addr_type.addr_64b);

    // my16bID
    packetfunctions_mac64bToMac16b(&idmanager_vars.my64bID, &idmanager_vars.my16bID);
}

bool idmanager_getIsDAGroot(void) {
    bool res;
    INTERRUPT_DECLARATION();

    DISABLE_INTERRUPTS();
    res = idmanager_vars.isDAGroot;
    ENABLE_INTERRUPTS();
    return res;
}

void idmanager_setIsDAGroot(bool newRole) {
    INTERRUPT_DECLARATION();
    DISABLE_INTERRUPTS();
    idmanager_vars.isDAGroot = newRole;
    icmpv6rpl_updateMyDAGrankAndParentSelection();
    schedule_startDAGroot();
    ENABLE_INTERRUPTS();
}

bool idmanager_getIsSlotSkip(void) {
    bool res;
    INTERRUPT_DECLARATION();

    DISABLE_INTERRUPTS();
    res = idmanager_vars.slotSkip;
    ENABLE_INTERRUPTS();
    return res;
}

open_addr_t* idmanager_getMyID(uint8_t type) {
    open_addr_t *res;

    INTERRUPT_DECLARATION();
    DISABLE_INTERRUPTS();
    switch (type) {
        case ADDR_16B:
            res = &idmanager_vars.my16bID;
            break;
        case ADDR_64B:
            res = &idmanager_vars.my64bID;
            break;
        case ADDR_PANID:
            res = &idmanager_vars.myPANID;
            break;
        case ADDR_PREFIX:
            res = &idmanager_vars.myPrefix;
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
            memcpy(&idmanager_vars.my16bID, newID, sizeof(open_addr_t));
            break;
        case ADDR_64B:
            memcpy(&idmanager_vars.my64bID, newID, sizeof(open_addr_t));
            break;
        case ADDR_PANID:
            memcpy(&idmanager_vars.myPANID, newID, sizeof(open_addr_t));
            break;
        case ADDR_PREFIX:
            memcpy(&idmanager_vars.myPrefix, newID, sizeof(open_addr_t));
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
            res = packetfunctions_sameAddress(addr, &idmanager_vars.my16bID);
            ENABLE_INTERRUPTS();
            return res;
        case ADDR_64B:
            res = packetfunctions_sameAddress(addr, &idmanager_vars.my64bID);
            ENABLE_INTERRUPTS();
            return res;
        case ADDR_128B:
            // build temporary my128bID
            temp_my128bID.type = ADDR_128B;
            memcpy(&temp_my128bID.addr_type.addr_128b[0], &idmanager_vars.myPrefix.addr_type.prefix, 8);
            memcpy(&temp_my128bID.addr_type.addr_128b[8], &idmanager_vars.my64bID.addr_type.addr_64b, 8);

            res = packetfunctions_sameAddress(addr, &temp_my128bID);
            ENABLE_INTERRUPTS();
            return res;
        case ADDR_PANID:
            res = packetfunctions_sameAddress(addr, &idmanager_vars.myPANID);
            ENABLE_INTERRUPTS();
            return res;
        case ADDR_PREFIX:
            res = packetfunctions_sameAddress(addr, &idmanager_vars.myPrefix);
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
            idmanager_vars.slotSkip = FALSE;
            break;
        case ACTION_NO:
            idmanager_setIsDAGroot(FALSE);
            idmanager_vars.slotSkip = TRUE;
            break;
        case ACTION_TOGGLE:
            if (idmanager_getIsDAGroot()) {
                idmanager_setIsDAGroot(FALSE);
                idmanager_vars.slotSkip = TRUE;
            } else {
                idmanager_setIsDAGroot(TRUE);
                idmanager_vars.slotSkip = FALSE;
            }
            break;
        default:
            LOG_ERROR(COMPONENT_IDMANAGER, ERR_INVALID_PARAM, (errorparameter_t)0, (errorparameter_t)0);
            return;
    }

    // store prefix (bytes 1-8)
    myPrefix.type = ADDR_PREFIX;
    memcpy(
            myPrefix.addr_type.prefix,
            &input_buffer[1],
            sizeof(myPrefix.addr_type.prefix)
    );
    idmanager_setMyID(&myPrefix);

    // indicate DODAGid to RPL
    memcpy(&dodagid[0], idmanager_vars.myPrefix.addr_type.prefix, 8);  // prefix
    memcpy(&dodagid[8], idmanager_vars.my64bID.addr_type.addr_64b, 8); // eui64
    icmpv6rpl_writeDODAGid(dodagid);

    // store L2 security key index and key value
    // for the moment, keys K1 and K2 are the same
    keyIndex = input_buffer[9];
    keyValue = &input_buffer[10];
    IEEE802154_security_setBeaconKey(keyIndex, keyValue);
    IEEE802154_security_setDataKey(keyIndex, keyValue);
}

void idmanager_setJoinKey(uint8_t *key) {
    memcpy(idmanager_vars.joinKey, key, 16);
}

void idmanager_getJoinKey(uint8_t **pKey) {
    *pKey = idmanager_vars.joinKey;
    return;
}

void idmanager_setJoinAsn(asn_t *asn) {
    memcpy(&idmanager_vars.joinAsn, asn, sizeof(asn_t));
}

/**
\brief Trigger this module to print status information, over serial.

debugPrint_* functions are used by the openserial module to continuously print
status information about several modules in the OpenWSN stack.

\returns TRUE if this function printed something, FALSE otherwise.
*/
bool debugPrint_id(void) {
    debugIDManagerEntry_t output;

    output.isDAGroot = idmanager_vars.isDAGroot;
    memcpy(output.myPANID, idmanager_vars.myPANID.addr_type.panid, 2);
    memcpy(output.my16bID, idmanager_vars.my16bID.addr_type.addr_16b, 2);
    memcpy(output.my64bID, idmanager_vars.my64bID.addr_type.addr_64b, 8);
    memcpy(output.myPrefix, idmanager_vars.myPrefix.addr_type.prefix, 8);

    openserial_printStatus(STATUS_ID, (uint8_t * ) & output, sizeof(debugIDManagerEntry_t));
    return TRUE;
}

bool debugPrint_joined(void) {
    asn_t output;
    output.byte4 = idmanager_vars.joinAsn.byte4;
    output.bytes2and3 = idmanager_vars.joinAsn.bytes2and3;
    output.bytes0and1 = idmanager_vars.joinAsn.bytes0and1;
    openserial_printStatus(STATUS_JOINED, (uint8_t * ) & output, sizeof(output));
    return TRUE;
}

//=========================== private =========================================
