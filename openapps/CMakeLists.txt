add_library(openapps "")

target_sources(openapps
        PUBLIC
        openapps.c
        uecho/uecho.h
        uecho/uecho.c
        cjoin/cjoin.h
        cjoin/cojp_cbor.h
        cjoin/cojp_cbor.c
        cjoin/cjoin.c)

target_compile_features(openapps PUBLIC c_std_99)
target_include_directories(openapps
        PUBLIC
        "../openapps"
        "../openapps/uecho"
        "../openapps/cjoin")
target_link_libraries(openapps PRIVATE bsp)
target_link_libraries(openapps PRIVATE openweb)
target_link_libraries(openapps PRIVATE openstack)
target_link_libraries(openapps PRIVATE opendrivers)
