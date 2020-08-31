#include "opendefs.h"
#include "iphc.h"
#include "sixtop.h"
#include "packetfunctions.h"
#include "idmanager.h"
#include "openserial.h"
#include "IEEE802154E.h"
#include "frag.h"
#include "forwarding.h"
#include "neighbors.h"
#include "openbridge.h"
#include "icmpv6rpl.h"

//=========================== variables =======================================

static const uint8_t dagroot_mac64b[] = {0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};

#if DEADLINE_OPTION
static monitor_expiration_vars_t  monitor_expiration_vars;
#endif

//=========================== prototypes ======================================

owerror_t iphc_retrieveIphcHeader(open_addr_t *temp_addr_16b,
                                  open_addr_t *temp_addr_64b,
                                  uint8_t *dispatch,
                                  uint8_t *tf,
                                  uint8_t *nh,
                                  uint8_t *hlim,
                                  uint8_t *sam,
                                  uint8_t *m,
                                  uint8_t *dam,
                                  OpenQueueEntry_t *msg,
                                  ipv6_header_iht *ipv6_header,
                                  uint8_t previousLen);

//===== IPv6 hop-by-hop header
owerror_t iphc_prependIPv6HopByHopHeader(OpenQueueEntry_t **msg, uint8_t nextheader, rpl_option_ht *rpl_option);

#if DEADLINE_OPTION
// IPv6 Deadline hop-by-hop header
owerror_t iphc_prependIPv6DeadlineHeader(OpenQueueEntry_t **msg);

uint8_t iphc_getAsnLen(uint8_t* asn);
#endif

//=========================== public ==========================================

void iphc_init(void) {
}

// send from upper layer: I need to add 6LoWPAN header
owerror_t iphc_sendFromForwarding(
        OpenQueueEntry_t *msg,
        ipv6_header_iht *ipv6_outer_header,
        ipv6_header_iht *ipv6_inner_header,
        rpl_option_ht *rpl_option,
#if DEADLINE_OPTION
        deadline_option_ht*	deadline_option,
#endif
        uint32_t *flow_label,
        uint8_t *rh3_copy,
        uint8_t rh3_length,
        uint8_t fw_SendOrfw_Rcv
) {
    open_addr_t temp_dest_prefix;
    open_addr_t temp_dest_mac64b;
    open_addr_t temp_src_prefix;
    open_addr_t temp_src_mac64b;
    open_addr_t temp_dagroot_ip128b;
    uint8_t sam;
    // take ownership over the packet
    msg->owner = COMPONENT_IPHC;

    // error checking
    if (idmanager_getIsDAGroot() == TRUE &&
        packetfunctions_isAllRoutersMulticast(&(msg->l3_destinationAdd)) == FALSE) {
        LOG_CRITICAL(COMPONENT_IPHC, ERR_BRIDGE_MISMATCH, (errorparameter_t) 0, (errorparameter_t) 0);
        return E_FAIL;
    }

    //discard the packet.. hop limit reached.
    if (ipv6_outer_header->src.type != ADDR_NONE) {
        // there is IPinIP check hop limit in ip in ip encapsulation
        if (ipv6_outer_header->hop_limit == 0) {
            LOG_ERROR(COMPONENT_IPHC, ERR_HOP_LIMIT_REACHED, (errorparameter_t) 0, (errorparameter_t) 0);
            return E_FAIL;
        } else {
            // decrement the packet's hop limit
            ipv6_outer_header->hop_limit--;
        }
    } else {
        if (ipv6_inner_header->hop_limit == 0) {
            LOG_ERROR(COMPONENT_IPHC, ERR_HOP_LIMIT_REACHED, (errorparameter_t) 0, (errorparameter_t) 0);
            return E_FAIL;
        } else {
            // decrement the packet's hop limit
            ipv6_inner_header->hop_limit--;
        }
    }

    packetfunctions_ip128bToMac64b(&(msg->l3_destinationAdd), &temp_dest_prefix, &temp_dest_mac64b);
    //xv poipoi -- get the src prefix as well
    packetfunctions_ip128bToMac64b(&(msg->l3_sourceAdd), &temp_src_prefix, &temp_src_mac64b);
    //XV -poipoi we want to check if the source address prefix is the same as destination prefix
    if (packetfunctions_sameAddress(&temp_dest_prefix, &temp_src_prefix)) {
        sam = IPHC_SAM_64B;    // no ipinip 6loRH if in the same prefix
    } else {
        //not the same prefix. so the packet travels to another network
        //check if this is a source routing pkt. in case it is then the DAM is elided as it is in the SrcRouting header.
        if (packetfunctions_isBroadcastMulticast(&(msg->l3_destinationAdd)) == FALSE) {
            // ip in ip will be presented
            sam = IPHC_SAM_128B;
        } else {
            // this is DIO, source address elided, multicast bit is set
            sam = IPHC_SAM_ELIDED;
        }
    }

    //IPinIP 6LoRH will be added at here if necessary.
    if (packetfunctions_sameAddress(&temp_dest_prefix, &temp_src_prefix)) {
        // same network, IPinIP is elided
    } else {
        if (packetfunctions_isBroadcastMulticast(&(msg->l3_destinationAdd)) == FALSE) {
            memset(&(temp_dagroot_ip128b), 0, sizeof(open_addr_t));
            packetfunctions_mac64bToIp128b(idmanager_getMyID(ADDR_PREFIX), (open_addr_t *) dagroot_mac64b,
                                           &(temp_dagroot_ip128b));
            if (
                    (
                            ipv6_outer_header->src.type == ADDR_NONE &&
                            packetfunctions_sameAddress(&(msg->l3_sourceAdd), &(temp_dagroot_ip128b))
                    ) ||
                    (
                            ipv6_outer_header->src.type != ADDR_NONE &&
                            packetfunctions_sameAddress(&(ipv6_outer_header->src), &(temp_dagroot_ip128b))
                    )
                    ) {
                // hop limit
                if (packetfunctions_reserveHeader(&msg, sizeof(uint8_t)) == E_FAIL) {
                    return E_FAIL;
                }
                *((uint8_t * )(msg->payload)) = ipv6_outer_header->hop_limit;
                // type
                if (packetfunctions_reserveHeader(&msg, sizeof(uint8_t)) == E_FAIL) {
                    return E_FAIL;
                }
                *((uint8_t * )(msg->payload)) = IPECAP_6LOTH_TYPE;
                // length 
                if (packetfunctions_reserveHeader(&msg, sizeof(uint8_t)) == E_FAIL) {
                    return E_FAIL;
                }
                *((uint8_t * )(msg->payload)) = ELECTIVE_6LoRH | 1;
            } else {
                if (sam == IPHC_SAM_128B) {
                    // encapsulate address
                    if (packetfunctions_writeAddress(&msg, &(msg->l3_sourceAdd), OW_BIG_ENDIAN) == E_FAIL){
                        return E_FAIL;
                    }
                    // hoplim
                    if (packetfunctions_reserveHeader(&msg, sizeof(uint8_t)) == E_FAIL) {
                        return E_FAIL;
                    }
                    *((uint8_t * )(msg->payload)) = ipv6_outer_header->hop_limit;
                    // type
                    if (packetfunctions_reserveHeader(&msg, sizeof(uint8_t)) == E_FAIL) {
                        return E_FAIL;
                    }
                    *((uint8_t * )(msg->payload)) = IPECAP_6LOTH_TYPE;
                    // length
                    if (packetfunctions_reserveHeader(&msg, sizeof(uint8_t)) == E_FAIL) {
                        return E_FAIL;
                    }
                    *((uint8_t * )(msg->payload)) = ELECTIVE_6LoRH | 17;
                }
            }
        } else {
            // this is DIO, no IPinIP either
        }
    }

#if DEADLINE_OPTION
    if ((msg->creator == COMPONENT_UEXPIRATION) && (deadline_option != NULL)) {
        if (
                deadline_option->optionType == DEADLINE_HOPBYHOP_HEADER_OPTION_TYPE &&
                packetfunctions_isBroadcastMulticast(&(msg->l3_destinationAdd)) == FALSE
                ) {
            if (iphc_prependIPv6DeadlineHeader(&msg) == E_FAIL){
                return E_FAIL;
            }
        }
    }
#endif

    //prepend Option hop by hop header except when src routing and dst is not 0xffff
    //-- this is a little trick as src routing is using an option header set to 0x00
    if (
            rpl_option->optionType == RPL_HOPBYHOP_HEADER_OPTION_TYPE &&
            packetfunctions_isBroadcastMulticast(&(msg->l3_destinationAdd)) == FALSE
            ) {
        if (iphc_prependIPv6HopByHopHeader(&msg, msg->l4_protocol, rpl_option) == E_FAIL){
            return E_FAIL;
        }
    }

    // copy RH3s back if length > 0
    if (rh3_length > 0) {
        if (packetfunctions_reserveHeader(&msg, rh3_length) == E_FAIL) {
            return E_FAIL;
        }
        memcpy(&msg->payload[0], &rh3_copy[0], rh3_length);
    }

    // if there are 6LoRH in the packet, add page dispatch no.1
    if (
            (*((uint8_t * )(msg->payload)) & FORMAT_6LORH_MASK) == CRITICAL_6LORH ||
            (*((uint8_t * )(msg->payload)) & FORMAT_6LORH_MASK) == ELECTIVE_6LoRH
            ) {
        if (packetfunctions_reserveHeader(&msg, sizeof(uint8_t)) == E_FAIL) {
            return E_FAIL;
        }
        *((uint8_t * )(msg->payload)) = PAGE_DISPATCH_NO_1;
    }

#if OPENWSN_6LO_FRAGMENTATION_C
    return frag_fragment6LoPacket(msg);
#else
    return sixtop_send(msg);
#endif
}

//send from bridge: 6LoWPAN header already added by OpenLBR, send as is
owerror_t iphc_sendFromBridge(OpenQueueEntry_t *msg) {
    msg->owner = COMPONENT_IPHC;
    // error checking
    if (idmanager_getIsDAGroot() == FALSE) {
        LOG_CRITICAL(COMPONENT_IPHC, ERR_BRIDGE_MISMATCH, (errorparameter_t) 1, (errorparameter_t) 0);
        return E_FAIL;
    }

    // send directly to sixtop layer (6lowpan headers still attached)
    return sixtop_send(msg);
}

void iphc_sendDone(OpenQueueEntry_t *msg, owerror_t error) {
    msg->owner = COMPONENT_IPHC;
    if (msg->creator == COMPONENT_OPENBRIDGE) {
        openbridge_sendDone(msg, error);
    } else {
        forwarding_sendDone(msg, error);
    }
}

void iphc_receive(OpenQueueEntry_t *msg) {
    ipv6_header_iht ipv6_outer_header;
    ipv6_header_iht ipv6_inner_header;
    uint8_t page_length;
    rpl_option_ht rpl_option;
    uint8_t rpi_length;
#if DEADLINE_OPTION
    deadline_option_ht *deadline_ptr = NULL;
    deadline_option_ht curr_deadline_option;
#endif

    msg->owner = COMPONENT_IPHC;

    memset(&ipv6_outer_header, 0, sizeof(ipv6_header_iht));
    memset(&ipv6_inner_header, 0, sizeof(ipv6_header_iht));
    memset(&rpl_option, 0, sizeof(rpl_option_ht));

#if DEADLINE_OPTION
    ipv6_outer_header.deadline_option = NULL;
#endif

    // then regular header
    if (iphc_retrieveIPv6Header(msg, &ipv6_outer_header, &ipv6_inner_header, &page_length) == E_FAIL) {
        openqueue_freePacketBuffer(msg);
        return;
    }

    // if the address is broadcast address, the ipv6 header is the inner header
    if (idmanager_getIsDAGroot() == FALSE || packetfunctions_isBroadcastMulticast(&(ipv6_inner_header.dest))) {
        packetfunctions_tossHeader(&msg, page_length);
        if (ipv6_outer_header.next_header == IANA_IPv6HOPOPT && ipv6_outer_header.hopByhop_option != NULL) {
            // retrieve hop-by-hop header (includes RPL option)
            rpi_length = iphc_retrieveIPv6HopByHopHeader(msg, &rpl_option);
#if DEADLINE_OPTION
            if (ipv6_outer_header.deadline_option) {
                memset(&curr_deadline_option, 0, sizeof(curr_deadline_option));
                iphc_retrieveIPv6DeadlineHeader(msg, ipv6_outer_header.deadline_option, &curr_deadline_option);
                deadline_ptr = &curr_deadline_option;
            }
#endif
            // toss the headers
            packetfunctions_tossHeader(&msg, rpi_length);
        }

        // send up the stack
        forwarding_receive(
                msg,
                &ipv6_outer_header,
                &ipv6_inner_header,
#if DEADLINE_OPTION
                deadline_ptr,
#endif
                &rpl_option
        );
    } else {
        openbridge_receive(msg);                   //out to the OpenVisualizer
    }
}

//=========================== private =========================================

//===== IPv6 header

owerror_t iphc_prependIPv6Header(
        OpenQueueEntry_t **msg,
        uint8_t tf,
        uint32_t value_flowLabel,
        uint8_t nh,
        uint8_t value_nextHeader,
        uint8_t hlim,
        uint8_t value_hopLimit,
        bool cid,
        bool sac,
        uint8_t sam,
        bool m,
        bool dac,
        uint8_t dam,
        open_addr_t *value_dest,
        open_addr_t *value_src,
        uint8_t fw_SendOrfw_Rcv
) {
    uint8_t temp_8b;

    // destination address
    switch (dam) {
        case IPHC_DAM_ELIDED:
            if (m == IPHC_M_YES) {
                if (packetfunctions_reserveHeader(msg, sizeof(uint8_t)) == E_FAIL) {
                    return E_FAIL;
                }
                *((uint8_t * )((*msg)->payload)) = value_dest->addr_128b[15];
            } else {
                //nothing
            }
            break;
        case IPHC_DAM_16B:
            if (m == IPHC_M_YES) {
                // tengfei: to do
            } else {
                if (value_dest->type != ADDR_16B) {
                    LOG_CRITICAL(COMPONENT_IPHC, ERR_WRONG_ADDR_TYPE,
                                 (errorparameter_t) value_dest->type,
                                 (errorparameter_t) 0);
                    return E_FAIL;
                };
                if (packetfunctions_writeAddress(msg, value_dest, OW_BIG_ENDIAN) == E_FAIL){
                    return E_FAIL;
                }
            }
            break;
        case IPHC_DAM_64B:
            if (m == IPHC_M_YES) {
                // tengfei: to do
            } else {
                if (value_dest->type != ADDR_64B) {
                    LOG_CRITICAL(COMPONENT_IPHC, ERR_WRONG_ADDR_TYPE,
                                 (errorparameter_t) value_dest->type,
                                 (errorparameter_t) 1);
                    return E_FAIL;
                };
                if (packetfunctions_writeAddress(msg, value_dest, OW_BIG_ENDIAN) == E_FAIL){
                    return E_FAIL;
                }
            }
            break;
        case IPHC_DAM_128B:
            if (m == IPHC_M_YES) {
                // tengfei: to do
            } else {
                if (value_dest->type != ADDR_128B) {
                    LOG_CRITICAL(COMPONENT_IPHC, ERR_WRONG_ADDR_TYPE,
                                 (errorparameter_t) value_dest->type,
                                 (errorparameter_t) 2);
                    return E_FAIL;
                };
                if (packetfunctions_writeAddress(msg, value_dest, OW_BIG_ENDIAN) == E_FAIL){
                    return E_FAIL;
                }
            }
            break;
        default:
            LOG_CRITICAL(COMPONENT_IPHC, ERR_6LOWPAN_UNSUPPORTED, (errorparameter_t) 0, (errorparameter_t) dam);
            return E_FAIL;
    }

    // source address
    switch (sam) {
        case IPHC_SAM_ELIDED:
            break;
        case IPHC_SAM_16B:
            if (fw_SendOrfw_Rcv == PCKTSEND) {
                if (packetfunctions_writeAddress(msg, (idmanager_getMyID(ADDR_16B)), OW_BIG_ENDIAN) == E_FAIL){
                    return E_FAIL;
                }
            }
            if (fw_SendOrfw_Rcv == PCKTFORWARD) {
                if (value_src->type != ADDR_16B) {
                    LOG_CRITICAL(COMPONENT_IPHC, ERR_WRONG_ADDR_TYPE,
                                 (errorparameter_t) value_src->type,
                                 (errorparameter_t) 0);
                    return E_FAIL;
                }
                if (packetfunctions_writeAddress(msg, value_src, OW_BIG_ENDIAN) == E_FAIL){
                    return E_FAIL;
                }
            }
            break;
        case IPHC_SAM_64B:
            if (fw_SendOrfw_Rcv == PCKTSEND) {
                if (packetfunctions_writeAddress(msg, (idmanager_getMyID(ADDR_64B)), OW_BIG_ENDIAN) == E_FAIL){
                    return E_FAIL;
                }
            }
            if (fw_SendOrfw_Rcv == PCKTFORWARD) {
                if (value_src->type != ADDR_64B) {
                    LOG_CRITICAL(COMPONENT_IPHC, ERR_WRONG_ADDR_TYPE,
                                 (errorparameter_t) value_src->type,
                                 (errorparameter_t) 1);
                    return E_FAIL;
                }
                if (packetfunctions_writeAddress(msg, value_src, OW_BIG_ENDIAN) == E_FAIL){
                    return E_FAIL;
                }
            }
            break;
        case IPHC_SAM_128B:
            if (fw_SendOrfw_Rcv == PCKTSEND) {
                if (packetfunctions_writeAddress(msg, (idmanager_getMyID(ADDR_64B)), OW_BIG_ENDIAN) == E_FAIL ||
                    packetfunctions_writeAddress(msg, (idmanager_getMyID(ADDR_PREFIX)), OW_BIG_ENDIAN) == E_FAIL) {
                    return E_FAIL;
                }
            }
            if (fw_SendOrfw_Rcv == PCKTFORWARD) {
                if (value_src->type != ADDR_128B) {
                    LOG_CRITICAL(COMPONENT_IPHC, ERR_WRONG_ADDR_TYPE,
                                 (errorparameter_t) value_src->type,
                                 (errorparameter_t) 2);
                    return E_FAIL;
                }
                if (packetfunctions_writeAddress(msg, value_src, OW_BIG_ENDIAN) == E_FAIL){
                    return E_FAIL;
                }
            }
            break;
        default:
            LOG_CRITICAL(COMPONENT_IPHC, ERR_6LOWPAN_UNSUPPORTED, (errorparameter_t) 1, (errorparameter_t) sam);
            return E_FAIL;
    }

    // hop limit
    switch (hlim) {
        case IPHC_HLIM_INLINE:
            if (packetfunctions_reserveHeader(msg, sizeof(uint8_t)) == E_FAIL) {
                return E_FAIL;
            }
            *((uint8_t * )((*msg)->payload)) = value_hopLimit;
            break;
        case IPHC_HLIM_1:
        case IPHC_HLIM_64:
        case IPHC_HLIM_255:
            break;
        default:
            LOG_CRITICAL(COMPONENT_IPHC, ERR_6LOWPAN_UNSUPPORTED, (errorparameter_t) 2, (errorparameter_t) hlim);
            return E_FAIL;
    }

    // next header
    switch (nh) {
        case IPHC_NH_INLINE:
            if (packetfunctions_reserveHeader(msg, sizeof(uint8_t)) == E_FAIL) {
                return E_FAIL;
            }
            *((uint8_t * )((*msg)->payload)) = value_nextHeader;
            break;
        case IPHC_NH_COMPRESSED:
            //do nothing, the next header will be there
            break;
        default:
            LOG_CRITICAL(COMPONENT_IPHC, ERR_6LOWPAN_UNSUPPORTED, (errorparameter_t) 3, (errorparameter_t) nh);
            return E_FAIL;
    }

    // flowlabel
    switch (tf) {
        case IPHC_TF_3B:
            if (packetfunctions_reserveHeader(msg, sizeof(uint8_t)) == E_FAIL) {
                return E_FAIL;
            }
            *((uint8_t * )((*msg)->payload)) = ((uint32_t)(value_flowLabel & 0x000000ff) >> 0);
            if (packetfunctions_reserveHeader(msg, sizeof(uint8_t)) == E_FAIL) {
                return E_FAIL;
            }
            *((uint8_t * )((*msg)->payload)) = ((uint32_t)(value_flowLabel & 0x0000ff00) >> 8);
            if (packetfunctions_reserveHeader(msg, sizeof(uint8_t)) == E_FAIL) {
                return E_FAIL;
            }
            *((uint8_t * )((*msg)->payload)) = ((uint32_t)(value_flowLabel & 0x00ff0000) >> 16);
            break;
        case IPHC_TF_ELIDED:
            break;
        case IPHC_TF_4B:
            //unsupported
        case IPHC_TF_1B:
            //unsupported
        default:
            LOG_CRITICAL(COMPONENT_IPHC, ERR_6LOWPAN_UNSUPPORTED, (errorparameter_t) 4, (errorparameter_t) tf);
            return E_FAIL;
    }

    // header
    temp_8b = 0;
    temp_8b |= cid << IPHC_CID;
    temp_8b |= sac << IPHC_SAC;
    temp_8b |= sam << IPHC_SAM;
    temp_8b |= m << IPHC_M;
    temp_8b |= dac << IPHC_DAC;
    temp_8b |= dam << IPHC_DAM;
    if (packetfunctions_reserveHeader(msg, sizeof(uint8_t)) == E_FAIL) {
        return E_FAIL;
    }
    *((uint8_t * )((*msg)->payload)) = temp_8b;
    temp_8b = 0;
    temp_8b |= IPHC_DISPATCH_IPHC << IPHC_DISPATCH;
    temp_8b |= tf << IPHC_TF;
    temp_8b |= nh << IPHC_NH;
    temp_8b |= hlim << IPHC_HLIM;
    if (packetfunctions_reserveHeader(msg, sizeof(uint8_t)) == E_FAIL) {
        return E_FAIL;
    }
    *((uint8_t * )((*msg)->payload)) = temp_8b;

    return E_SUCCESS;
}

owerror_t iphc_retrieveIPv6Header(OpenQueueEntry_t *msg, ipv6_header_iht *ipv6_outer_header,
                                  ipv6_header_iht *ipv6_inner_header, uint8_t *page_length) {
    uint8_t temp_8b;
    open_addr_t temp_addr_16b;
    open_addr_t temp_addr_64b;
    uint8_t dispatch;
    uint8_t tf;
    uint8_t nh;
    uint8_t hlim;
    uint8_t sam;
    uint8_t m;
    uint8_t dam;
    uint8_t size;
    uint8_t lorh_type;
    uint8_t rh3_index;
    uint8_t page;
    uint8_t lorh_length;

    uint8_t extention_header_length;

    *page_length = 0;
    extention_header_length = 0;
    rh3_index = 0;
    ipv6_outer_header->header_length = 0;
    ipv6_inner_header->header_length = 0;
    ipv6_outer_header->rhe_length = 0;


    // four steps to retrieve:
    // step 1. Paging number
    // step 2. IP in IP 6LoRH
    // step 3. IPv6 extention header
    // step 4. IPv6 inner header
    // ====================== 1. paging number =================================
    temp_8b = *((uint8_t * )(msg->payload) + ipv6_outer_header->header_length);
    if ((temp_8b & PAGE_DISPATCH_TAG) == PAGE_DISPATCH_TAG) {
        page = temp_8b & PAGE_DISPATCH_NUM;
        *page_length = 1;
    } else {
        page = 0;
    }
    //======================= 2. and 3. IPv6 extention header and IPv6 inner header =========================
    if (page == 1) {
        extention_header_length = 0;
        temp_8b = *((uint8_t * )(msg->payload) + *page_length + extention_header_length);
        while ((temp_8b & FORMAT_6LORH_MASK) == CRITICAL_6LORH || (temp_8b & FORMAT_6LORH_MASK) == ELECTIVE_6LoRH) {
            switch (temp_8b & FORMAT_6LORH_MASK) {
                case CRITICAL_6LORH :
                    lorh_type = *((uint8_t * )(msg->payload) + *page_length + extention_header_length + 1);
                    if (lorh_type <= RH3_6LOTH_TYPE_4) {
                        if (rh3_index == MAXNUM_RH3) {
                            LOG_ERROR(COMPONENT_IPHC, ERR_6LOWPAN_UNSUPPORTED,
                                      (errorparameter_t) 13,
                                      (errorparameter_t)(rh3_index));
                            return E_FAIL;
                        }
                        if (rh3_index == 0) {
                            if (ipv6_outer_header->hopByhop_option == NULL) {
                                ipv6_outer_header->next_header = IANA_IPv6ROUTE;
                            }
                            ipv6_outer_header->routing_header[rh3_index] = (uint8_t * )(msg->payload) + \
                                *page_length + \
                                extention_header_length;
                        }
                        size = temp_8b & RH3_6LOTH_SIZE_MASK;
                        size += 1;
                        switch (lorh_type) {
                            case 0:
                                extention_header_length += 2 + 1 * size;
                                break;
                            case 1:
                                extention_header_length += 2 + 2 * size;
                                break;
                            case 2:
                                extention_header_length += 2 + 4 * size;
                                break;
                            case 3:
                                extention_header_length += 2 + 8 * size;
                                break;
                            case 4:
                                extention_header_length += 2 + 16 * size;
                                break;
                        }
                    } else {
                        if (lorh_type == 5) {
                            if (ipv6_outer_header->routing_header[0] == NULL) {
                                ipv6_outer_header->next_header = IANA_IPv6HOPOPT;
                            }
                            ipv6_outer_header->hopByhop_option = (uint8_t * )(msg->payload) + *page_length + \
                                extention_header_length;
                            switch (temp_8b & (I_FLAG | K_FLAG)) {
                                case 0:
                                    extention_header_length += 2 + 3;
                                    break;
                                case 1:
                                case 2:
                                    extention_header_length += 2 + 2;
                                    break;
                                case 3:
                                    extention_header_length += 2 + 1;
                                    break;
                            }
                        } else {
                            //log wrong inf
                            LOG_ERROR(COMPONENT_IPHC, ERR_6LOWPAN_UNSUPPORTED,
                                      (errorparameter_t) 14,
                                      (errorparameter_t)(lorh_type));
                        }
                    }
                    break;
                case ELECTIVE_6LoRH :
                    // this is an elective 6LoRH
                    lorh_length = temp_8b & IPINIP_LEN_6LORH_MASK;
                    lorh_type = *(uint8_t * )(
                            msg->payload + ipv6_outer_header->header_length + *page_length + extention_header_length +
                            1);
                    if (lorh_type == IPINIP_TYPE_6LORH) {
                        ipv6_outer_header->header_length += 1;
                        // this is IpinIP 6LoRH
                        ipv6_outer_header->header_length += 1;
                        ipv6_outer_header->hop_limit = *(uint8_t * )(
                                msg->payload + ipv6_outer_header->header_length + *page_length +
                                extention_header_length);
                        ipv6_outer_header->header_length += 1;
                        // destination address maybe is the first address in RH3 6LoRH OR dest adress in IPHC, reset
                        // first update destination address if necessary after the processing
                        ipv6_outer_header->dest.type = ADDR_NONE;
                        memset(&(ipv6_outer_header->dest.addr_128b[0]), 0, 16);
                        if (lorh_length == 1) {
                            // source address is root
                            memset(&(ipv6_outer_header->src), 0, sizeof(open_addr_t));
                            packetfunctions_mac64bToIp128b(idmanager_getMyID(ADDR_PREFIX),
                                                           (open_addr_t *) dagroot_mac64b, &(ipv6_outer_header->src));
                            // destination address is the first address in RH3 6LoRH OR dest adress in IPHC
                        } else {
                            switch (lorh_length - 1) {
                                case 16:
                                    // this is source address
                                    packetfunctions_readAddress(((uint8_t * )(
                                            msg->payload + ipv6_outer_header->header_length + *page_length +
                                            extention_header_length)), ADDR_128B, &ipv6_outer_header->src,
                                                                OW_BIG_ENDIAN);
                                    ipv6_outer_header->header_length += 16 * sizeof(uint8_t);
                                    break;
                                case 8:
                                    // this is source address
                                    packetfunctions_readAddress(((uint8_t * )(
                                            msg->payload + ipv6_outer_header->header_length + *page_length +
                                            extention_header_length)), ADDR_64B, &temp_addr_64b, OW_BIG_ENDIAN);
                                    ipv6_outer_header->header_length += 8 * sizeof(uint8_t);
                                    packetfunctions_mac64bToIp128b(idmanager_getMyID(ADDR_PREFIX), &temp_addr_64b,
                                                                   &ipv6_outer_header->src);
                                    break;
                                default:
                                    // do not support other length yet and destination address will be in RH3 or IPHC
                                    LOG_ERROR(COMPONENT_IPHC, ERR_6LOWPAN_UNSUPPORTED,
                                              (errorparameter_t) 12,
                                              (errorparameter_t)(lorh_length - 1));
                            }
                        }
                    }
#if DEADLINE_OPTION
                    else if (lorh_type == DEADLINE_6LOTH_TYPE) {
                        ipv6_outer_header->deadline_option = (uint8_t * )(msg->payload) + *page_length + \
                                extention_header_length;
                        extention_header_length += (lorh_length + 1);
                    }
#endif
                    else {
                        // unknown elective packet, print error and skip it
                        LOG_ERROR(COMPONENT_IPHC, ERR_6LOWPAN_UNSUPPORTED,
                                  (errorparameter_t) 13,
                                  (errorparameter_t)(rh3_index));
                        extention_header_length += 2 + lorh_length;
                        ipv6_outer_header->rhe_length += 2 + lorh_length;
                    }
                    break;
            }
            temp_8b = *(uint8_t * )((msg->payload) + \
                        *page_length + \
                        extention_header_length + \
                        ipv6_outer_header->header_length);
            rh3_index++;
        }
    }

    //======================= 4. IPHC inner header =============================
    if (iphc_retrieveIphcHeader(
            &temp_addr_16b,
            &temp_addr_64b,
            &dispatch,
            &tf,
            &nh,
            &hlim,
            &sam,
            &m,
            &dam,
            msg,
            ipv6_inner_header,
            extention_header_length + ipv6_outer_header->header_length + *page_length
    ) == E_FAIL) {
        return E_FAIL;
    }

    return E_SUCCESS;
}

owerror_t iphc_retrieveIphcHeader(open_addr_t *temp_addr_16b,
                                open_addr_t *temp_addr_64b,
                                uint8_t *dispatch,
                                uint8_t *tf,
                                uint8_t *nh,
                                uint8_t *hlim,
                                uint8_t *sam,
                                uint8_t *m,
                                uint8_t *dam,
                                OpenQueueEntry_t *msg,
                                ipv6_header_iht *ipv6_header,
                                uint8_t previousLen) {

    uint8_t page;
    uint8_t temp_8b;
    uint8_t ipinip_length;
    uint8_t lowpan_nhc;

    temp_8b = *((uint8_t * )(msg->payload) + ipv6_header->header_length + previousLen);

    if ((temp_8b & PAGE_DISPATCH_TAG) == PAGE_DISPATCH_TAG) {
        page = temp_8b & PAGE_DISPATCH_NUM;
        ipv6_header->header_length += sizeof(uint8_t);
    } else {
        page = 0;
    }

    if (
            page == 0 ||
            (page == 1 && (*((uint8_t * )(msg->payload) + ipv6_header->header_length + previousLen) & 0xC0) != 0x80)
            ) {
        // header
        temp_8b = *((uint8_t * )(msg->payload) + ipv6_header->header_length + previousLen);
        *dispatch = (temp_8b >> IPHC_DISPATCH) & 0x07;   // 3b
        *tf = (temp_8b >> IPHC_TF) & 0x03;   // 2b
        *nh = (temp_8b >> IPHC_NH) & 0x01;   // 1b
        *hlim = (temp_8b >> IPHC_HLIM) & 0x03;   // 2b
        ipv6_header->header_length += sizeof(uint8_t);
        temp_8b = *((uint8_t * )(msg->payload) + ipv6_header->header_length + previousLen);
        // cid unused
        // sac unused
        *sam = (temp_8b >> IPHC_SAM) & 0x03;   // 2b
        // m unused
        *m = (temp_8b >> IPHC_M) & 0x01;   // 1b
        // dac unused
        *dam = (temp_8b >> IPHC_DAM) & 0x03;   // 2b
        ipv6_header->header_length += sizeof(uint8_t);

        // dispatch
        switch (*dispatch) {
            case IPHC_DISPATCH_IPHC:
                break;
            default:
                LOG_ERROR(COMPONENT_IPHC, ERR_6LOWPAN_UNSUPPORTED,
                          (errorparameter_t) 5,
                          (errorparameter_t)(*dispatch));
                return E_FAIL;
        }

        // flowlabel
        switch (*tf) {
            case IPHC_TF_3B:

                ipv6_header->flow_label = 0;
                ipv6_header->flow_label |=
                        ((uint32_t) * ((uint8_t * )(msg->payload) + ipv6_header->header_length + previousLen)) << 0;
                ipv6_header->header_length += sizeof(uint8_t);
                ipv6_header->flow_label |=
                        ((uint32_t) * ((uint8_t * )(msg->payload) + ipv6_header->header_length + previousLen)) << 8;
                ipv6_header->header_length += sizeof(uint8_t);
                ipv6_header->flow_label |=
                        ((uint32_t) * ((uint8_t * )(msg->payload) + ipv6_header->header_length + previousLen)) << 16;
                ipv6_header->header_length += sizeof(uint8_t);
                break;
            case IPHC_TF_ELIDED:
                ipv6_header->flow_label = 0;
                break;
            case IPHC_TF_4B:
                //unsupported
            case IPHC_TF_1B:
                //unsupported
            default:
                LOG_ERROR(COMPONENT_IPHC, ERR_6LOWPAN_UNSUPPORTED, (errorparameter_t) 6, (errorparameter_t)(*tf));
                return E_FAIL;
        }

        // next header
        switch (*nh) {
            case IPHC_NH_INLINE:
                // Full 8 bits for Next Header are carried in-line
                ipv6_header->next_header_compressed = FALSE;
                ipv6_header->next_header = *((uint8_t * )(msg->payload) + ipv6_header->header_length + previousLen);
                ipv6_header->header_length += sizeof(uint8_t);

                break;
            case IPHC_NH_COMPRESSED:
                // the Next header field is compressed and the next header is encoded
                // using LOWPAN_NHC, which is discussed in Section 4.1 of RFC6282
                // we don't parse anything here, but will look at the (compressed)
                // next header after having parsed all address fields.
                ipv6_header->next_header_compressed = TRUE;
                break;
            default:
                LOG_ERROR(COMPONENT_IPHC, ERR_6LOWPAN_UNSUPPORTED, (errorparameter_t) 7, (errorparameter_t)(*nh));
                return E_FAIL;
        }

        // hop limit
        switch (*hlim) {
            case IPHC_HLIM_INLINE:
                ipv6_header->hop_limit = *((uint8_t * )(msg->payload + ipv6_header->header_length + previousLen));
                ipv6_header->header_length += sizeof(uint8_t);
                break;
            case IPHC_HLIM_1:
                ipv6_header->hop_limit = 1;
                break;
            case IPHC_HLIM_64:
                ipv6_header->hop_limit = 64;
                break;
            case IPHC_HLIM_255:
                ipv6_header->hop_limit = 255;
                break;
            default:
                LOG_ERROR(COMPONENT_IPHC, ERR_6LOWPAN_UNSUPPORTED, (errorparameter_t) 8, (errorparameter_t)(*hlim));
                return E_FAIL;
        }

        // source address
        switch (*sam) {
            case IPHC_SAM_ELIDED:
                packetfunctions_mac64bToIp128b(idmanager_getMyID(ADDR_PREFIX), &(msg->l2_nextORpreviousHop),
                                               &ipv6_header->src);
                break;
            case IPHC_SAM_16B:
                packetfunctions_readAddress(((uint8_t * )(msg->payload + ipv6_header->header_length + previousLen)),
                                            ADDR_16B, temp_addr_16b, OW_BIG_ENDIAN);
                ipv6_header->header_length += 2 * sizeof(uint8_t);
                packetfunctions_mac16bToMac64b(temp_addr_16b, temp_addr_64b);
                packetfunctions_mac64bToIp128b(idmanager_getMyID(ADDR_PREFIX), temp_addr_64b, &ipv6_header->src);
                break;
            case IPHC_SAM_64B:
                packetfunctions_readAddress(((uint8_t * )(msg->payload + ipv6_header->header_length + previousLen)),
                                            ADDR_64B, temp_addr_64b, OW_BIG_ENDIAN);
                ipv6_header->header_length += 8 * sizeof(uint8_t);
                packetfunctions_mac64bToIp128b(idmanager_getMyID(ADDR_PREFIX), temp_addr_64b, &ipv6_header->src);
                break;
            case IPHC_SAM_128B:
                packetfunctions_readAddress(((uint8_t * )(msg->payload + ipv6_header->header_length + previousLen)),
                                            ADDR_128B, &ipv6_header->src, OW_BIG_ENDIAN);
                ipv6_header->header_length += 16 * sizeof(uint8_t);
                break;
            default:
                LOG_ERROR(COMPONENT_IPHC, ERR_6LOWPAN_UNSUPPORTED, (errorparameter_t) 9, (errorparameter_t)(*sam));
                return E_FAIL;
        }

        // destination address
        if (*m == IPHC_M_YES) {
            switch (*dam) {
                case IPHC_DAM_ELIDED:
                    ipv6_header->dest.type = ADDR_128B;
                    memcpy(&(ipv6_header->dest.addr_128b[0]), all_routers_multicast, sizeof(all_routers_multicast));
                    ipv6_header->dest.addr_128b[15] = *(msg->payload + ipv6_header->header_length + previousLen);
                    ipv6_header->header_length += sizeof(uint8_t);
                    break;
                case IPHC_DAM_16B:
                    // tengfei: todo
                    break;
                case IPHC_DAM_64B:
                    // tengfei: todo
                    break;
                case IPHC_DAM_128B:
                    // tengfei: todo
                    break;
                default:
                    LOG_ERROR(COMPONENT_IPHC, ERR_6LOWPAN_UNSUPPORTED, (errorparameter_t) 10, (errorparameter_t)(*dam));
                    return E_FAIL;
            }
        } else {
            switch (*dam) {
                case IPHC_DAM_ELIDED:
                    packetfunctions_mac64bToIp128b(idmanager_getMyID(ADDR_PREFIX), idmanager_getMyID(ADDR_64B),
                                                   &(ipv6_header->dest));
                    break;
                case IPHC_DAM_16B:
                    packetfunctions_readAddress(((uint8_t * )(msg->payload + ipv6_header->header_length + previousLen)),
                                                ADDR_16B, temp_addr_16b, OW_BIG_ENDIAN);
                    ipv6_header->header_length += 2 * sizeof(uint8_t);
                    packetfunctions_mac16bToMac64b(temp_addr_16b, temp_addr_64b);
                    packetfunctions_mac64bToIp128b(idmanager_getMyID(ADDR_PREFIX), temp_addr_64b, &ipv6_header->dest);
                    break;
                case IPHC_DAM_64B:
                    packetfunctions_readAddress(((uint8_t * )(msg->payload + ipv6_header->header_length + previousLen)),
                                                ADDR_64B, temp_addr_64b, OW_BIG_ENDIAN);
                    ipv6_header->header_length += 8 * sizeof(uint8_t);
                    packetfunctions_mac64bToIp128b(idmanager_getMyID(ADDR_PREFIX), temp_addr_64b, &ipv6_header->dest);
                    break;
                case IPHC_DAM_128B:
                    packetfunctions_readAddress(((uint8_t * )(msg->payload + ipv6_header->header_length + previousLen)),
                                                ADDR_128B, &ipv6_header->dest, OW_BIG_ENDIAN);
                    ipv6_header->header_length += 16 * sizeof(uint8_t);
                    break;
                default:
                    LOG_ERROR(COMPONENT_IPHC, ERR_6LOWPAN_UNSUPPORTED, (errorparameter_t) 10, (errorparameter_t)(*dam));
                    return E_FAIL;
            }
        }
        //TODO, check NH if compressed no?
        if (ipv6_header->next_header_compressed) {
            lowpan_nhc = *(msg->payload + ipv6_header->header_length +
                           previousLen);//get the next element after addresses
            if ((lowpan_nhc & NHC_UDP_MASK) == NHC_UDP_ID) { //check if it is UDP LOWPAN_NHC
                ipv6_header->next_header = IANA_UDP;
            } else {
                //error?
            }
        }
    } else {
        // we are in 6LoRH now
        if (page == 1) {
            temp_8b = *(uint8_t * )(msg->payload + ipv6_header->header_length + previousLen);
            if ((temp_8b & FORMAT_6LORH_MASK) == ELECTIVE_6LoRH) {
                // this is an elective 6LoRH
                ipinip_length = temp_8b & IPINIP_LEN_6LORH_MASK;
                ipv6_header->header_length += 1;
                temp_8b = *(uint8_t * )(msg->payload + ipv6_header->header_length + previousLen);
                if (temp_8b == IPINIP_TYPE_6LORH) {
                    // this is IpinIP 6LoRH
                    ipv6_header->header_length += 1;
                    ipv6_header->hop_limit = *(uint8_t * )(msg->payload + ipv6_header->header_length + previousLen);
                    ipv6_header->header_length += 1;
                    // destination address maybe is the first address in RH3 6LoRH OR dest adress in IPHC, reset first
                    // update destination address if necessary after the processing
                    ipv6_header->dest.type = ADDR_NONE;
                    memset(&(ipv6_header->dest.addr_128b[0]), 0, 16);
                    if (ipinip_length == 1) {
                        // source address is root
                        memset(&(ipv6_header->src), 0, sizeof(open_addr_t));
                        packetfunctions_mac64bToIp128b(idmanager_getMyID(ADDR_PREFIX), (open_addr_t *) dagroot_mac64b,
                                                       &(ipv6_header->src));
                        // destination address is the first address in RH3 6LoRH OR dest adress in IPHC
                    } else {
                        switch (ipinip_length - 1) {
                            case 16:
                                // this is source address
                                packetfunctions_readAddress(
                                        ((uint8_t * )(msg->payload + ipv6_header->header_length + previousLen)),
                                        ADDR_128B, &ipv6_header->src, OW_BIG_ENDIAN);
                                ipv6_header->header_length += 16 * sizeof(uint8_t);
                                break;
                            case 8:
                                // this is source address
                                packetfunctions_readAddress(
                                        ((uint8_t * )(msg->payload + ipv6_header->header_length + previousLen)),
                                        ADDR_64B, temp_addr_64b, OW_BIG_ENDIAN);
                                ipv6_header->header_length += 8 * sizeof(uint8_t);
                                packetfunctions_mac64bToIp128b(idmanager_getMyID(ADDR_PREFIX), temp_addr_64b,
                                                               &ipv6_header->src);
                                break;
                            default:
                                // do not support other length yet and destination address will be in RH3 or IPHC
                                LOG_ERROR(COMPONENT_IPHC, ERR_6LOWPAN_UNSUPPORTED,
                                          (errorparameter_t) 12,
                                          (errorparameter_t)(ipinip_length - 1));
                                return E_FAIL;
                        }
                    }
                }

            } else {
                // ip in ip is elided: the followig is a critical 6lorh
                // finish the ip header compression
            }
        }
    }
    return E_SUCCESS;
}

//===== IPv6 hop-by-hop header

/**
\brief Prepend an IPv6 hop-by-hop header to a message.

\note The field are written in reverse order.

\param[in,out] msg             The message to prepend the header to.
\param[in]     nextheader      The next header value to use.
\param[in]     rpl_option      The RPL option to include.
*/
owerror_t iphc_prependIPv6HopByHopHeader(OpenQueueEntry_t **msg, uint8_t nextheader, rpl_option_ht *rpl_option) {
    uint8_t temp_8b;

    if ((rpl_option->flags & K_FLAG) == 0) {
        if (packetfunctions_reserveHeader(msg, sizeof(uint16_t)) == E_FAIL) {
            return E_FAIL;
        }
        (*msg)->payload[0] = (uint8_t)((rpl_option->senderRank & 0xFF00) >> 8);
        (*msg)->payload[1] = (uint8_t)(rpl_option->senderRank & 0x00FF);
    } else {
        if (packetfunctions_reserveHeader(msg, sizeof(uint8_t)) == E_FAIL) {
            return E_FAIL;
        }
        *((uint8_t * )((*msg)->payload)) = (uint8_t)((rpl_option->senderRank & 0xFF00) >> 8);
    }

    if ((rpl_option->flags & I_FLAG) == 0) {
        if (packetfunctions_reserveHeader(msg, sizeof(uint8_t)) == E_FAIL){
            return E_FAIL;
        }
        *((uint8_t * )((*msg)->payload)) = rpl_option->rplInstanceID;
    }

    if (packetfunctions_reserveHeader(msg, sizeof(uint8_t)) == E_FAIL) {
        return E_FAIL;
    }
    *((uint8_t * )((*msg)->payload)) = RPI_6LOTH_TYPE;

    temp_8b = CRITICAL_6LORH | rpl_option->flags;
    if (packetfunctions_reserveHeader(msg, sizeof(uint8_t)) == E_FAIL) {
        return E_FAIL;
    }
    *((uint8_t * )((*msg)->payload)) = temp_8b;

    return E_SUCCESS;
}

#if DEADLINE_OPTION
//===== IPv6 Deadline hop-by-hop header
/**
\brief Prepend an IPv6 Deadline hop-by-hop header to a message.

\note The field are written in reverse order.

\param[in,out] msg   The message to prepend the header to.
*/
owerror_t iphc_prependIPv6DeadlineHeader(OpenQueueEntry_t **msg) {
    uint8_t temp_8b = 0, temp_len = 0, asn_len = 0;
    deadline_option_ht curr_deadline_option;
    uint8_t asn_array[5];

    // Origination Time (OT)
    if ((*msg)->orgination_time_flag == 1) {
        ieee154e_getAsn(asn_array);

        asn_len = iphc_getAsnLen(asn_array);

        if (packetfunctions_reserveHeader(msg, asn_len) == E_FAIL) {
            return E_FAIL
        }
        memcpy(&((*msg)->payload[0]), &asn_array, asn_len * sizeof(uint8_t));
        temp_len += asn_len;

        //set OTL flag value
        curr_deadline_option.org_otl = asn_len - 1;
    } else {
        //set OTL flag value
        curr_deadline_option.org_otl = 0;
    }

    // Expiration Time (ET)
    ieee154e_calculateExpTime((*msg)->max_delay, asn_array);
    asn_len = iphc_getAsnLen(asn_array);
    if (packetfunctions_reserveHeader(msg, asn_len) == E_FAIL) {
        return E_FAIL;
    }
    memcpy(&((*msg)->payload[0]), &asn_array, asn_len * sizeof(uint8_t));
    temp_len += asn_len;

    // 4th byte
    // TU(2bytes) | EXP(3bytes) | RSV(3bytes)
    curr_deadline_option.time_unit = 2; // in ASN
    curr_deadline_option.exponent = 0; // Time in ASN
    curr_deadline_option.rsv = 0; // Reserved
    temp_8b = (curr_deadline_option.time_unit << 6) | (curr_deadline_option.exponent << 3) | curr_deadline_option.rsv;
    if (packetfunctions_reserveHeader(msg, sizeof(uint8_t)) == E_FAIL){
        return E_FAIL;
    }
    *((uint8_t * )((*msg)->payload)) = temp_8b;
    temp_len++;

    // 3rd byte
    // O_FLAG(1byte) | D_FLAG(1byte) | ETL(3bytes) | OTL(3bytes)
    curr_deadline_option.exp_etl = asn_len - 1;

    curr_deadline_option.o_flag = (*msg)->orgination_time_flag;
    curr_deadline_option.d_flag = (*msg)->drop_flag;

    temp_8b = ((*msg)->orgination_time_flag << 7) | ((*msg)->drop_flag << 6) | (curr_deadline_option.exp_etl << 3) |
              (curr_deadline_option.org_otl);
    if (packetfunctions_reserveHeader(msg, sizeof(uint8_t)) == E_FAIL){
        return E_FAIL;
    }
    *((uint8_t * )((*msg)->payload)) = temp_8b;
    temp_len++;

    // 2nd byte : Elective Header Type : DEADLINE
    if (packetfunctions_reserveHeader(msg, sizeof(uint8_t)) == E_FAIL){
        return E_FAIL;
    }
    *((uint8_t * )((*msg)->payload)) = DEADLINE_6LOTH_TYPE;
    temp_len++;

    // 1st byte : 6LoRH Type: ELECTIVE
    temp_8b = ELECTIVE_6LoRH | temp_len;
    if (packetfunctions_reserveHeader(msg, sizeof(uint8_t)) == E_FAIL){
        return E_FAIL;
    }
    *((uint8_t * )((*msg)->payload)) = temp_8b;

    return E_SUCCESS;
}

#endif

/**
\brief Retrieve an IPv6 hop-by-hop header from a message.

\param[in,out] msg        The message to retrieve the header from.
\param[out]    rpl_option Pointer to the structure to hold the retrieved RPL option.
\returns       the header length in bytes.
*/
uint8_t iphc_retrieveIPv6HopByHopHeader(OpenQueueEntry_t *msg, rpl_option_ht *rpl_option) {
    uint8_t temp_8b;
    uint8_t type;
    uint8_t length;

    // initialize the header length (will increment at each field)
    length = 0;

    temp_8b = *((uint8_t * )(msg->payload) + length);
    type = *((uint8_t * )(msg->payload) + length + 1);

    if (
            (temp_8b & FORMAT_6LORH_MASK) == CRITICAL_6LORH &&
            type == RPI_6LOTH_TYPE
            ) {
        // this is 6LoRH RPI, move pointer to compressed field
        length += sizeof(uint16_t);
        // get the O, R, F I and K value
        rpl_option->flags = (uint8_t)(temp_8b & FLAG_MASK);
        // check FLAG I to get rplinstance ID
        if ((temp_8b & I_FLAG) == 0) {
            rpl_option->rplInstanceID = *((uint8_t * )(msg->payload) + length);
            length += sizeof(uint8_t);
        } else {
            // Global RPLInstanceID
            rpl_option->rplInstanceID = 0;
        }
        // check FLAG K to get senderRannk
        if ((temp_8b & K_FLAG) == 0) {
            rpl_option->senderRank = *((uint8_t * )(msg->payload) + length);
            rpl_option->senderRank = ((rpl_option->senderRank) << 8) + *((uint8_t * )(msg->payload) + length + 1);
            length += sizeof(uint16_t);
        } else {
            rpl_option->senderRank = *((uint8_t * )(msg->payload) + length);
            rpl_option->senderRank = rpl_option->senderRank * 256;
            length += sizeof(uint8_t);
        }
    } else {
        LOG_ERROR(COMPONENT_IPHC, ERR_6LOWPAN_UNSUPPORTED, (errorparameter_t) 14, (errorparameter_t) type);
    }
    return length;
}

#if DEADLINE_OPTION
/**
\brief Retrieve a Deadline hop-by-hop header from a message.

\param[in,out] msg            The message to retrieve the header from.
\param[in] deadline_msg_ptr   Pointer to the Deadline header.
\param[out] deadline_option   Pointer to the structure to hold the retrieved Deadline option 
*/
void iphc_retrieveIPv6DeadlineHeader(
        OpenQueueEntry_t *msg,
        uint8_t *deadline_msg_ptr,
        deadline_option_ht *deadline_option
) {
    uint8_t temp_8b, type, i, length = 0;
    asn_t exp_asn, my_asn;
    uint8_t my_asn_array[5];

    temp_8b = *((uint8_t *) deadline_msg_ptr);
    length++;
    type = *((uint8_t * )(deadline_msg_ptr) + length);

    if (
            (temp_8b & FORMAT_6LORH_MASK) == ELECTIVE_6LoRH &&
            type == DEADLINE_6LOTH_TYPE
            ) {

        // 3rd byte
        length++;
        temp_8b = *((uint8_t * )(deadline_msg_ptr) + length);

        deadline_option->o_flag = (temp_8b >> 7);
        deadline_option->d_flag = (temp_8b >> 6);
        deadline_option->exp_etl = (temp_8b >> 3);
        deadline_option->org_otl = (temp_8b);

        // 4th byte
        length++;
        temp_8b = *((uint8_t * )(deadline_msg_ptr) + length);
        deadline_option->time_unit = (temp_8b >> 6);
        deadline_option->exponent = (temp_8b >> 3);

        // Expiration Time
        length++;
        for (i = 0; i <= deadline_option->exp_etl; i++) {
            deadline_option->et_val[i] = *((uint8_t * )(deadline_msg_ptr) + length);
            length++;
        }
        ieee154e_orderToASNStructure(deadline_option->et_val, &exp_asn);

        // Calculate delay experienced by packet
        ieee154e_getAsn(my_asn_array);
        ieee154e_orderToASNStructure(my_asn_array, &my_asn);
        deadline_option->time_left = ieee154e_computeAsnDiff(&exp_asn, &my_asn);
        if (deadline_option->time_left < 0) {
            deadline_option->time_left = 0;
            LOG_ERROR(COMPONENT_IPHC, ERR_6LORH_DEADLINE_EXPIRED, (errorparameter_t) 0, (errorparameter_t) 0);
        }
        monitor_expiration_vars.time_left = deadline_option->time_left;

        // Origination Time
        if (deadline_option->o_flag) {
            for (i = 0; i <= deadline_option->org_otl; i++) {
                deadline_option->ot_val[i] = *((uint8_t * )(deadline_msg_ptr) + length);
                length++;
            }
            ieee154e_orderToASNStructure(deadline_option->ot_val, &my_asn);
            monitor_expiration_vars.time_elapsed = ieee154e_asnDiff(&my_asn);
        }

    } else {
        LOG_ERROR(COMPONENT_IPHC, ERR_6LOWPAN_UNSUPPORTED, (errorparameter_t) 14, (errorparameter_t) type);
    }
}

// To send deadline hop-by-hop info to upper layers
void iphc_getDeadlineInfo(monitor_expiration_vars_t *stats) {
    stats->time_left = monitor_expiration_vars.time_left;
    stats->time_elapsed = monitor_expiration_vars.time_elapsed;
    memset(&(monitor_expiration_vars), 0, sizeof(monitor_expiration_vars_t));
}

uint8_t iphc_getAsnLen(uint8_t *asn) {
    uint8_t i;
    for (i = 5; i >= 1; i--) {
        if (asn[i - 1] != 0) {
            return (i);
        }
    }
    return 5;
}

#endif
