#ifndef OPENWSN_CONFIG_H
#define OPENWSN_CONFIG_H

// =========================== Debugging ============================

/**
 * \def OPENWSN_DEBUG_LEVEL
 *
 * Specifies the debugging level used in the OpenWSN stack.
 * - level 0: no logging
 * - level 1: only critical logs
 * - level 2: critical and error logs
 * - level 3: critical, error, and success
 * - level 4: critical, error, success, and warning
 * - level 5: critical, error, success, warning, and info
 * - level 6: critical, error, success, warning, info, and verbose
 *
 */
#ifndef OPENWSN_DEBUG_LEVEL
#define OPENWSN_DEBUG_LEVEL         6
#endif

// ========================== Applications ==========================

/**
 * \def OPENWSN_C6T_C
 *
 * Application that allows direct manipulation of the 6top scheduling.
 *
 * Requires: OPENWSN_COAP_C
 *
 */
#ifndef OPENWSN_C6T_C
#define OPENWSN_C6T_C (0)
#endif

/**
 * \def OPENWSN_CEXAMPLE_C
 *
 * A CoAP example application. It periodically sends a random string of numbers.
 *
 * Requires: OPENWSN_COAP_C
 *
 */
#ifndef OPENWSN_CEXAMPLE_C
#define OPENWSN_CEXAMPLE_C (0)
#endif

/**
 * \def OPENWSN_CINFO_C
 *
 * CoAP application which responds with information about the OpenWSN version running on the board.
 *
 * Requires: OPENWSN_COAP_C
 *
 */
#ifndef OPENWSN_CINFO_C
#define OPENWSN_CINFO_C (0)
#endif

/**
 * \def OPENWSN_CINFRARED_C
 *
 * A CoAP infrared application.
 *
 * Requires: OPENWSN_COAP_C
 *
 */
#ifndef OPENWSN_CINFRARED_C
#define OPENWSN_CINFRARED_C (0)
#endif

/**
 * \def OPENWSN_CLED_C
 *
 * CoAP application that exposes the leds of the board as a CoAP resource.
 *
 * Requires: OPENWSN_COAP_C
 *
 */
#ifndef OPENWSN_CLED_C
#define OPENWSN_CLED_C (0)
#endif

/**
 * \def OPENWSN_CSENSORS_C
 *
 * A CoAP resource which allows an application to GET/SET the state of sensors.
 *
 * Requires: OPENWSN_COAP_C
 *
 */
#ifndef OPENWSN_CSENSORS_C
#define OPENWSN_CSENSORS_C (0)
#endif

/**
 * \def OPENWSN_CSTORM_C
 *
 *
 *
 * Requires: OPENWSN_COAP_C
 *
 */
#ifndef OPENWSN_CSTORM_C
#define OPENWSN_CSTORM_C (0)
#endif

/**
 * \def OPENWSN_CWELLKNOWN_C
 *
 * Implements the CoAP .wellknown endpoint on a mote.
 *
 * Requires: OPENWSN_COAP_C
 *
 */
#ifndef OPENWSN_CWELLKNOWN_C
#define OPENWSN_CWELLKNOWN_C (0)
#endif

/**
 * \def OPENWSN_RRT_C
 *
 * A CoAP application.
 *
 * Requires: OPENWSN_COAP_C
 *
 */
#ifndef OPENWSN_RRT_C
#define OPENWSN_RRT_C (0)
#endif

/**
 * \def OPENWSN_UECHO_C
 *
 * An echo application (server side) that sits directly on top of UDP.
 *
 * Requires: OPENWSN_UDP_C
 *
 */
#ifndef OPENWSN_UECHO_C
#define OPENWSN_UECHO_C (0)
#endif

/**
 * \def OPENWSN_UEXPIRATION_C
 *
 *
 *
 * Requires: OPENWSN_UDP_C
 *
 */
#ifndef OPENWSN_UEXPIRATION_C
#define OPENWSN_UEXPIRATION_C (0)
#endif

/**
 * \def OPENWSN_UEXP_MONITOR_C
 *
 *
 *
 * Requires: OPENWSN_UDP_C
 *
 */
#ifndef OPENWSN_UEXP_MONITOR_C
#define OPENWSN_UEXP_MONITOR_C (0)
#endif

/**
 * \def OPENWSN_UINJECT_C
 *
 * Application that creates UDP traffic and injects it in the network.
 *
 * Requires: OPENWSN_UDP_C
 *
 */
#ifndef OPENWSN_UINJECT_C
#define OPENWSN_UINJECT_C (0)
#endif

/**
 * \def OPENWSN_USERIALBRIDGE_C
 *
 * Requires: OPENWSN_UDP_C
 *
 */
#ifndef OPENWSN_USERIALBRIDGE_C
#define OPENWSN_USERIALBRIDGE_C (0)
#endif

/**
 * \def OPENWSN_CJOIN_C
 *
 * The CJOIN protocol performs a secure joining and install link-layer keys
 *
 * Requires: OPENWSN_UDP_C, OPENWSN_COAP_C
 *
 */
#ifndef OPENWSN_CJOIN_C
#define OPENWSN_CJOIN_C (0)
#endif

// ======================= OpenWeb configuration =======================

/**
 * \def OPENWSN_COAP_C
 *
 * Implementation of the CoAP protocol.
 *
 * Requires: OPENWSN_UDP_C
 *
 */
#ifndef OPENWSN_COAP_C
#define OPENWSN_COAP_C (0)
#endif

/**
 * \def OPENWSN_COAP_PORT_DEFAULT
 *
 * Defines the default port to use for COAP
 *
 */
#ifndef OPENWSN_COAP_PORT_DEFAULT
#define OPENWSN_COAP_PORT_DEFAULT   (5683)
#endif

// ========================== Stack modules ===========================

/**
 * \def OPENWSN_UDP_C
 *
 * Implementation of the UDP protocol.
 *
 */
#ifndef OPENWSN_UDP_C
#define OPENWSN_UDP_C (0)
#endif

/**
 * \def OPENWSN_6LO_FRAGMENTATION_C
 *
 * Implements 6LoWPAN fragmentation.
 *
 * Configuration options:
 *  - MAX_PKTSIZE_SUPPORTED: defines the maximum IPV6 packet size (header + payload) the mote supports. Default
 *  value is 1320. This corresponds to a 40-byte IPv6 header + the minimal IPv6 MTU of 1280 bytes.
 *  - MAX_NUM_BIGPKTS: defines how many static buffer space will be allocated for processing large packets.
 *
 */
#ifndef OPENWSN_6LO_FRAGMENTATION_C
#define OPENWSN_6LO_FRAGMENTATION_C (0)
#endif

#if OPENWSN_6LO_FRAGMENTATION_C
#ifndef MAX_PKTSIZE_SUPPORTED
#define MAX_PKTSIZE_SUPPORTED   1320
#endif
#ifndef MAX_NUM_BIGPKTS
#define MAX_NUM_BIGPKTS         2
#endif
#endif

/**
 * \def OPENWSN_ICMPV6ECHO_C
 *
 * Enables the icmpv6 echo (ping) functionality
 */
#ifndef OPENWSN_ICMPV6ECHO_C
#define OPENWSN_ICMPV6ECHO_C (0)
#endif


/**
 * \def OPENWSN_IEEE802154E_SECURITY_C
 *
 * Enables link-layer security. When this is enabled you should also use BOARD_CRYPTOENGINE_ENABLED where possible.
 *
 * Requires: OPENWSN_CJOIN_C, OPENWSN_COAP_C, OPENWSN_UDP_C
 */
#ifndef OPENWSN_IEEE802154E_SECURITY_C
#define OPENWSN_IEEE802154E_SECURITY_C (0)
#endif


/**
 * \def OPENWSN_FORCETOPOLOGY_C
 *
 * Force the networks topology according the code in topology.c
 *
 */
#ifndef OPENWSN_FORCETOPOLOGY_C
#define OPENWSN_FORCETOPOLOGY_C (0)
#endif


/**
 * \def OPENWSN_ADAPTIVESYNC_C
 *
 * Force the networks topology according the code in topology.c
 *
 */
#ifndef OPENWSN_ADAPTIVE_SYNC_C
#define OPENWSN_ADAPTIVE_SYNC_C (0)
#endif


// ========================== Stack configuration ===========================

/**
 * \def DEADLINE_OPTION
 *
 * Activates IPv6 DEADLINE OPTION.
 *
 */
#ifndef DEADLINE_OPTION
#define DEADLINE_OPTION (0)
#endif

/**
 * \def ADAPTIVE_MSF
 *
 * Allow the MSF algorithm to dynamically remove and allocate slots, based on the traffic load in the network.
 *
 * Configuration options:
 *  - MSF_MAX_NUMCELLS: cell usage is calculated every MSF_MAX_NUMCELLS elapsed cells.
 *  - MSF_LIM_NUMCELLSUSED_HIGH: if cell usage is above this value, trigger 6P to add a single cell to the selected parent
 *  - MSF_LIM_NUMCELLSUSED_LOW: if cell usage is below this value, trigger 6P to remove a single cell to the selected parent
 */
#ifndef ADAPTIVE_MSF
#define ADAPTIVE_MSF (0)
#endif

#if ADAPTIVE_MSF
#ifndef MSF_MAX_NUMCELLS
#define MSF_MAX_NUMCELLS            32
#endif
#ifndef MSF_LIM_NUMCELLSUSED_HIGH
#define MSF_LIM_NUMCELLSUSED_HIGH   24
#endif
#ifndef MSF_LIM_NUMCELLSUSED_HIGH
#define MSF_LIM_NUMCELLSUSED_HIGH   8
#endif
#endif

/**
 * \def IEEE802154E_SINGLE_CHANNEL
 *
 * Sets channel to a fixed value (acceptable values are [11 - 26] and [0])
 * When the channel is set to 0, frequency hopping is enabled, otherwise a single channel is used.
 *
 */
#ifndef IEEE802154E_SINGLE_CHANNEL
#define IEEE802154E_SINGLE_CHANNEL      0
#endif

/**
 * \def PACKETQUEUE_LENGTH
 *
 * Specifies the size of the packet queue. Large queue sizes are required to support fragmentation but significantly
 * increase RAM usage.
 *
 */
#ifndef PACKETQUEUE_LENGTH
#define PACKETQUEUE_LENGTH              20
#endif

/**
 * \def DAGROOT
 *
 * Set this mote as the DODAG root.
 *
 */
#ifndef DAGROOT
#define DAGROOT (0)
#endif

/**
 * \def DAGROOT
 *
 * Set this mote as the DODAG root.
 *
 */
#ifndef PANID_DEFINED
#define PANID_DEFINED   (0xcafe)
#endif

// ======================== Board configuration ========================

/**
 * \def BOARD_CRYPTOENGINE_ENABLED
 *
 * Enable AES hardware acceleration. This options is only available on boards that support hardware acceleration. It
 * cannot be combined with the python board.
 *
 */
#ifndef BOARD_CRYPTOENGINE_ENABLED
#define BOARD_CRYPTOENGINE_ENABLED (0)
#endif

/**
 * \def BOARD_OPENSERIAL_PRINTF
 *
 * Enable usage of openserial_printf function to print strings over the serial port.
 * WARNING: this feature links in nano.specs and nosys.specs adding roughly 3Kb of Flash usage.
 *
 */
#ifndef BOARD_OPENSERIAL_PRINTF
#define BOARD_OPENSERIAL_PRINTF (0)
#endif


/**
 * \def BOARD_OPENSERIAL_SNIFFER
 *
 * Prints sniffed packet over serial.
 *
 */
#ifndef BOARD_OPENSERIAL_SNIFFER
#define BOARD_OPENSERIAL_SNIFFER (0)
#endif


/**
 * \def BOARD_SENSORS_ENABLED
 *
 * Includes the sensor driver
 *
 */
#ifndef BOARD_SENSORS_ENABLED
#define BOARD_SENSORS_ENABLED (0)
#endif

/**
 * \def BOARD_DEEP_SLEEP
 *
 * Enable deeper sleep for BOARDs that support it. This might hinder serial
 * operation.
 *
 */
#ifndef BOARD_DEEP_SLEEP
#define BOARD_DEEP_SLEEP (0)
#endif

/**
 * \def BOARD_FASTSIM_ENABLED
 *
 * Enables fast UART printing in simulation mode. Active by default.
 *
 */
#if !defined(BOARD_FASTSIM_ENABLED) && defined(PYTHON_BOARD)
#define BOARD_FASTSIM_ENABLED (1)
#else
#define BOARD_FASTSIM_ENABLED (0)
#endif

// ======================== Kernel configuration ========================

/**
 * \def SCHEDULER_DEBUG_ENABLE
 *
 * Enables storing of scheduler debug variables (scheduler_debug_vars)
 *
 */
#ifndef SCHEDULER_DEBUG_ENABLE
#define SCHEDULER_DEBUG_ENABLE (0)
#endif

#include "check_config.h"

#endif /* OPENWSN_CONFIG_H */
