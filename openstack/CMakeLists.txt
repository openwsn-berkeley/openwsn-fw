add_library(openstack "")

target_sources(openstack
        PRIVATE
        02a-MAClow/adaptive_sync.c
        02a-MAClow/IEEE802154.c
        02a-MAClow/IEEE802154_security.c
        02a-MAClow/IEEE802154E.c
        02a-MAClow/topology.c
        02b-MAChigh/msf.c
        02b-MAChigh/neighbors.c
        02b-MAChigh/schedule.c
        02b-MAChigh/sixtop.c
        03a-IPHC/frag.c
        03a-IPHC/iphc.c
        03a-IPHC/openbridge.c
        03b-IPv6/forwarding.c
        03b-IPv6/icmpv6.c
        03b-IPv6/icmpv6echo.c
        03b-IPv6/icmpv6rpl.c
        04-TRAN/udp.c
        04-TRAN/sock/sock.c
        cross-layers/idmanager.c
        cross-layers/openqueue.c
        cross-layers/openrandom.c
        cross-layers/packetfunctions.c
        PUBLIC
        openstack.c)

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

target_link_libraries(openstack PUBLIC opendrivers)
target_link_libraries(openstack PUBLIC bsp)
target_link_libraries(openstack PUBLIC kernel)
