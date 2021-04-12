add_library(openapps "")

target_sources(openapps
        PRIVATE
        openapps.c)

if (OPT-RRT)
    target_sources(openapps PRIVATE rrt/rrt.c)
endif ()

if (OPT-UECHO)
    target_sources(openapps PRIVATE uecho/uecho.c)
endif ()

if (OPT-CSTORM)
    target_sources(openapps PRIVATE cstorm/cstorm.c)
endif ()

if (OPT-CLED)
    target_sources(openapps PRIVATE cled/cled.c)
endif ()

if (OPT-WELLKNOWN)
    target_sources(openapps PRIVATE cwellknown/cwellknown.c)
endif ()

if (OPT-C6T)
    target_sources(openapps PRIVATE c6t/c6t.c)
endif ()

if (OPT-CINFRARED)
    target_sources(openapps PRIVATE cinfrared/cinfrared.c)
endif ()

if (OPT-CINFO)
    target_sources(openapps PRIVATE cinfo/cinfo.c)
endif ()

if (OPT-CEXAMPLE)
    target_sources(openapps PRIVATE cexample/cexample.c)
endif ()

if (OPT-CJOIN)
    target_sources(openapps PRIVATE cjoin/cjoin.c cjoin/cojp_cbor.c)
endif ()


target_compile_features(openapps PUBLIC c_std_99)
target_include_directories(openapps
        PUBLIC
        .
        ${CMAKE_SOURCE_DIR}/inc
        PRIVATE
        c6t
        cled
        uecho
        cstorm
        cinfrared
        cexample
        cinfo
        rrt
        cwellknown
        cjoin)

target_link_libraries(openapps PRIVATE bsp)
target_link_libraries(openapps PRIVATE openweb)
target_link_libraries(openapps PRIVATE openstack)
target_link_libraries(openapps PRIVATE opendrivers)

