add_library(openstack STATIC "")

target_sources(openstack
        PRIVATE
        02a-MAClow/adaptive_sync.c
        02a-MAClow/adaptive_sync.h
        02a-MAClow/IEEE802154.c
        02a-MAClow/IEEE802154.h
        02a-MAClow/IEEE802154_security.c
        02a-MAClow/IEEE802154_security.h
        02a-MAClow/IEEE802154E.c
        02a-MAClow/IEEE802154E.h
        02a-MAClow/topology.c
        02a-MAClow/topology.h
        02b-MAChigh/msf.c
        02b-MAChigh/msf.h
        02b-MAChigh/neighbors.c
        02b-MAChigh/neighbors.h
        02b-MAChigh/schedule.c
        02b-MAChigh/schedule.h
        02b-MAChigh/sixtop.c
        02b-MAChigh/sixtop.h
        03a-IPHC/frag.c
        03a-IPHC/frag.h
        03a-IPHC/iphc.c
        03a-IPHC/iphc.h
        03a-IPHC/openbridge.c
        03a-IPHC/openbridge.h
        03b-IPv6/forwarding.c
        03b-IPv6/forwarding.h
        03b-IPv6/icmpv6.c
        03b-IPv6/icmpv6.h
        03b-IPv6/icmpv6echo.c
        03b-IPv6/icmpv6echo.h
        03b-IPv6/icmpv6rpl.c
        03b-IPv6/icmpv6rpl.h
        04-TRAN/udp.c
        04-TRAN/udp.h
        04-TRAN/async.h
        04-TRAN/async_types.h
        04-TRAN/sock.h
        04-TRAN/sock.c
        04-TRAN/sock_internal.h
        04-TRAN/sock_types.h
        cross-layers/idmanager.c
        cross-layers/idmanager.h
        cross-layers/openqueue.c
        cross-layers/openqueue.h
        cross-layers/openrandom.c
        cross-layers/openrandom.h
        cross-layers/packetfunctions.c
        cross-layers/packetfunctions.h
        PUBLIC
        openstack.c
        openstack.h)

target_compile_features(openstack PUBLIC c_std_99)
target_include_directories(openstack
        PUBLIC "../openstack"
        PRIVATE "../openstack/02a-MAClow"
        PRIVATE "../openstack/02b-MAChigh"
        PRIVATE "../openstack/03a-IPHC"
        PRIVATE "../openstack/03b-IPv6"
        PRIVATE "../openstack/04-TRAN"
        PRIVATE "../openstack/cross-layers"
        INTERFACE "../openstack/02a-MAClow"
        INTERFACE "../openstack/02b-MAChigh"
        INTERFACE "../openstack/03a-IPHC"
        INTERFACE "../openstack/03b-IPv6"
        INTERFACE "../openstack/04-TRAN"
        INTERFACE "../openstack/cross-layers")

target_link_libraries(openstack PUBLIC drivers)
target_link_libraries(openstack PUBLIC kernel)
target_link_libraries(openstack PUBLIC openweb)
target_link_libraries(openstack PUBLIC openapps)
