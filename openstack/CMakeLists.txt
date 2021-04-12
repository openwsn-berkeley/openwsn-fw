add_library(openstack "")

target_sources(openstack
        PRIVATE
        02a-MAClow/IEEE802154.c
        02a-MAClow/IEEE802154_security.c
        02a-MAClow/IEEE802154E.c
        02a-MAClow/topology.c
        02b-MAChigh/msf.c
        02b-MAChigh/neighbors.c
        02b-MAChigh/schedule.c
        02b-MAChigh/sixtop.c
        03a-IPHC/iphc.c
        03a-IPHC/openbridge.c
        03b-IPv6/forwarding.c
        03b-IPv6/icmpv6.c
        03b-IPv6/icmpv6rpl.c
        04-TRAN/udp.c
        04-TRAN/sock/sock.c
        cross-layers/idmanager.c
        cross-layers/openqueue.c
        cross-layers/openrandom.c
        cross-layers/packetfunctions.c
        openstack.c)

if (OPT-FRAG)
    target_sources(openstack PRIVATE 03a-IPHC/frag.c)
endif()

if (OPT-PING)
    target_sources(openstack PRIVATE 03b-IPv6/icmpv6echo.c)
endif()


target_compile_features(openstack PUBLIC c_std_99)
target_include_directories(openstack
        PUBLIC
        .
        ${CMAKE_SOURCE_DIR}/inc
        02a-MAClow
        02b-MAChigh
        03a-IPHC
        03b-IPv6
        04-TRAN
        04-TRAN/sock
        cross-layers)

target_link_libraries(openstack PRIVATE opendrivers)
target_link_libraries(openstack PRIVATE bsp)
target_link_libraries(openstack PRIVATE kernel)

