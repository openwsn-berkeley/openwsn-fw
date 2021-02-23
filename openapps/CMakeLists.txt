add_library(openapps "")

target_sources(openapps
        PRIVATE
        openapps.c
        rrt/rrt.c
        uecho/uecho.c
        cstorm/cstorm.c
        cled/cled.c
        cwellknown/cwellknown.c
        c6t/c6t.c
        cinfrared/cinfrared.c
        cinfo/cinfo.c
        cexample/cexample.c
        cjoin/cojp_cbor.c
        cjoin/cjoin.c)

target_compile_features(openapps PUBLIC c_std_99)
target_include_directories(openapps
        PUBLIC
        "."
        PRIVATE
        "c6t"
        "cled"
        "uecho"
        "cstorm"
        "cinfrared"
        "cexample"
        "cinfo"
        "rrt"
        "cwellknown"
        "cjoin")

target_link_libraries(openapps PRIVATE bsp)
target_link_libraries(openapps PRIVATE openweb)
target_link_libraries(openapps PRIVATE openstack)
target_link_libraries(openapps PRIVATE opendrivers)
