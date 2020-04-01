#ifndef OPENWSN_CHECK_CONFIG_H
#define OPENWSN_CHECK_CONFIG_H


#if !defined(OPENWSN_COAP_C) && (\
    defined(OPENWSN_C6T_C) || \
    defined(OPENWN_CEXAMPLE_C) || \
    defined(OPENWSN_CINFO_C) || \
    defined(OPENWSN_CINFRARED_C) || \
    defined(OPENWSN_CLED_C) || \
    defined(OPENWSN_CSENSORS_C) || \
    defined(OPENWSN_CSTORM_C) || \
    defined(OPENWSN_CWELLKNOWN) || \
    defined(OPENWSN_RRT_C))

#error "A CoAP dependent application is defined, but CoAP is not included in the build."
#endif

#if !defined(OPENWSN_OPENUDP_C) && (\
    defined(OPENWSN_USERIALBRIDGE_C) || \
    defined(OPENWN_UECHO_C) || \
    defined(OPENWSN_UINJECT_C) || \
    defined(OPENWSN_USERIALBRIDGE_C) || \
    defined(OPENWSN_UEXPIRATION_C) || \
    defined(OPENWSN_UEXP_MONITOR))

#error "A UDP dependent application is defined, but UDP is not included in the build."
#endif

#if !defined(OPENWSN_6LO_FRAGMENTATION_C) && (\
    defined(OPENWSN_MAX_PKTSIZE_SUPPORTED) || \
    defined(OPENWN_MAX_NUM_BIGPKTS))

#error "6LoWPAN fragmentation options specified, but 6LoWPAN fragmentation is not included in the build."
#endif

#if defined(OPENWSN_CJOIN_C) && !defined(OPENWSN_COAP_C)

#errori "CJOIN requires the CoAP protocol."
#endif

#if defined(OPENWSN_COAP_C) && (\
    !defined(OPENWSN_OPENUDP_C) && !defined(OPEWSN_TCP_C))

#error "CoAP requires a transport layer, i.e. UDP or TCP."
#endif

#endif /* OPENWSN_CHECK_CONFIG_H */