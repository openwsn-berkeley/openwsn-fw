#ifndef OPENWSN_CHECK_CONFIG_H
#define OPENWSN_CHECK_CONFIG_H

#if !defined(PYTHON_BOARD) && \
    !defined(TELOSB) && \
    !defined(WSN430V13B) && \
    !defined(WSN430V14) && \
    !defined(GINA) && \
    !defined(Z1) && \
    !defined(OPENMOTESTM) && \
    !defined(OPENMOTE_CC2538) && \
    !defined(OPENMOTE_B) && \
    !defined(OPENMOTE_B_24GHZ) && \
    !defined(OPENMOTE_B_SUBGHZ) && \
    !defined(AGILEFOX) && \
    !defined(IOTLAB_M3) && \
    !defined(IOTLAB_A8_M3) && \
    !defined(SAMR21_XPRO) && \
    !defined(NRF52840) && \
    !defined(NRF52833_XXAA) && \
    !defined(NRF5340_XXAA_NET)
#error 'Board name must be specified to check for configuration errors'
#endif

#if (defined(OPENMOTE_CC2538) || \
    defined(OPENMOTE_B) || \
    defined(OPENMOTE_B_24GHZ) || \
    defined(OPENMOTE_B_SUBGHZ) || \
    defined(TELOSB) || \
    defined(WSN430V13B) || \
    defined(WSN430V14) || \
    defined(OPENMOTESTM) || \
    defined(GINA) || \
    defined(NRF52840) || \
    defined(AGILEFOX) || \
    defined(IOTLAB_M3) || \
    defined(IOTLAB_A8_M3) || \
    defined(SAMR21_XPRO) || \
    defined(Z1)) && \
    defined(OPENWSN_IEEE802154E_SECURITY_C) && \
    !defined(BOARD_CRYPTOENGINE_ENABLED)
#warning 'Software encryption might be too slow on certain hardware. It is recommend to use BOARD_CRYTPOENGINE_ENABLED where possible.'
#endif

#if defined(BOARD_CRYPTOENGINE_ENABLED) && (\
    defined(WSN430V13B) || \
    defined(WSN430V14) || \
    defined(AGILEFOX) || \
    defined(GINA) || \
    defined(NRF52840) || \
    defined(SAMR21_XPRO) || \
    defined(Z1) || \
    defined(OPENMOTESTM))
#error 'Hardware encryption not supported on this platform.'
#endif

#if defined(OPENWSN_IEEE802154E_SECURITY_C) && !defined(OPENWSN_CJOIN_C)
#error 'Link-layer security requires CJOIN application.'
#endif

#if defined(PYTHON_BOARD) && defined(BOARD_CRYPTOENGINE_ENABLED)
#error 'Python board does not support hardware acceleration.'
#endif

#if defined(IEEE802154E_SINGLE_CHANNEL) && \
    ((IEEE802154E_SINGLE_CHANNEL != 0) && \
    ((IEEE802154E_SINGLE_CHANNEL < 11) || \
    (IEEE802154E_SINGLE_CHANNEL > 26)))
#error 'Illegal value for OPENWSN_IEEE802154E_SINGLE_CHANNEL'
#endif

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

#if defined(OPENWSN_CSENSORS_C) && !defined(BOARD_SENSORS_ENABLED)
#error "The CSENSORS app requires the sensor drivers."
#endif

#if !defined(OPENWSN_UDP_C) && (\
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

#error "CJOIN requires the CoAP protocol."
#endif

#if defined(OPENWSN_COAP_C) && (\
    !defined(OPENWSN_UDP_C) && !defined(OPEWSN_TCP_C))

#error "CoAP requires a transport layer, i.e. UDP or TCP."
#endif

#endif /* OPENWSN_CHECK_CONFIG_H */
