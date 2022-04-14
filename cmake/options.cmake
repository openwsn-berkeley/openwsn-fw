set(LOG_LEVEL "6" CACHE STRING "Select a logging level: 0 (no logs) - 6 (all logs)")
add_definitions(-DOPENWSN_LOG_LEVEL=${LOG_LEVEL})
set_property(CACHE LOG_LEVEL PROPERTY STRINGS "0" "1" "2" "3" "4" "5" "6")

set(IEEE154E_CHANNEL "0" CACHE STRING "Pick a fiexed channel between 11 and 26, or select 0 for channel hopping")
add_definitions(-DIEEE802154E_SINGLE_CHANNEL=${IEEE154E_CHANNEL})

set(DEFAULT_COAP_PORT "5683" CACHE STRING "Set a default CoAP server port")
add_definitions(-DDEFAULT_COAP_PORT=${DEFAULT_COAP_PORT})

set(PACKETQUEUE_LENGTH "20" CACHE STRING "Set the size of the packet buffer")
add_definitions(-DPACKETQUEUE_LENGTH=${PACKETQUEUE_LENGTH})

set(PANID "0xcafe" CACHE STRING "Set a 2-byte PAN ID")
add_definitions(-DPANID_DEFINED=${PANID})

option(OPT-COAP "Enable the COAP protocol" OFF)
if (OPT-COAP)
    add_definitions(-DOPENWSN_COAP_C)
endif ()

option(OPT-FORCE-TOPO "Enable a fixed topology. You need to set the topology in openstack/02a-MAClow/topology.c" OFF)
if (OPT-FORCE-TOPO)
    add_definitions(-DOPENWSN_FORCETOPOLOGY_C)
endif ()

option(OPT-UDP "Enable the UDP protocol" OFF)
if (OPT-UDP)
    add_definitions(-DOPENWSN_UDP_C)
endif ()

option(OPT-L2-SEC "Enable L2 security" OFF)
if (OPT-L2-SEC)
    add_definitions(-DOPENWSN_IEEE802154E_SECURITY_C)
endif ()

option(OPT-FRAG "Enable 6LoWPAN fragmentation" OFF)
if (OPT-FRAG)
    add_definitions(-DOPENWSN_6LO_FRAGMENTATION_C)
endif ()

option(OPT-PING "Enable the ping functionality (icmpv6_echo)" OFF)
if (OPT-PING)
    add_definitions(-DOPENWSN_ICMPV6ECHO_C)
endif ()

option(OPT-CJOIN "Enable the ping functionality" OFF)
if (OPT-CJOIN)
    add_definitions(-DOPENWSN_CJOIN_C)
endif ()

option(OPT-UECHO "UDP application that echoes all receives data" OFF)
if (OPT-UECHO)
    add_definitions(-DOPENWSN_UECHO_C)
endif ()

option(OPT-RRT "Simple CoAP application which returns board information" OFF)
if (OPT-RRT)
    add_definitions(-DOPENWSN_RRT_C)
endif ()

option(OPT-WELLKNOWN "Simple CoAP endpoint which returns the all the registered URIs." OFF)
if (OPT-WELLKNOWN)
    add_definitions(-DOPENWSN_CWELLKNOWN_C)
endif ()

option(OPT-C6T "Simple CoAP application that allows manipulation of the schedule through 6top." OFF)
if (OPT-C6T)
    add_definitions(-DOPENWSN_C6T_C)
endif ()

option(OPT-CEXAMPLE "" OFF)
if (OPT-CEXAMPLE)
    add_definitions(-DOPENWSN_CEXAMPLE_C)
endif ()

option(OPT-CLED "Simple CoAP to toggle board leds." OFF)
if (OPT-CLED)
    add_definitions(-DOPENWSN_CLED_C)
endif ()

option(OPT-CINFRARED "Simple CoAP to toggle board leds." OFF)
if (OPT-CINFRARED)
    add_definitions(-DOPENWSN_CINFRARED_C)
endif ()

option(OPT-CSTORM "Simple CoAP application which returns board information" OFF)
if (OPT-CSTORM)
    add_definitions(-DOPENWSN_CSTORM_C)
endif ()

option(OPT-CINFO "" OFF)
if (OPT-CINFO)
    add_definitions(-DOPENWSN_CINFO_C)
endif ()

option(OPT-MSF "Enable the adaptive MSF functionality" OFF)
if (OPT-MSF)
    add_definitions(-DADAPTIVE_MSF)
endif ()

option(OPT-DAGROOT "Configure the build as DAGroot" OFF)
if (OPT-DAGROOT)
    add_definitions(-DDAGROOT)
endif ()

option(OPT-PRINTF "Enable printf functionality" OFF)
if (OPT-PRINTF)
    add_definitions(-DBOARD_OPENSERIAL_PRINTF)
endif ()

option(OPT-CRYPTO-HW "Enable hardware acceleration for crypto operations" OFF)
if (OPT-CRYPTO-HW)
    add_definitions(-DBOARD_CRYPTOENGINE_ENABLED)
endif ()
