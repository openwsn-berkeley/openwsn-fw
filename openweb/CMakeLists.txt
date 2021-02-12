add_library(openweb "")

target_sources(openweb
        PUBLIC
        opencoap/coap.c
        opencoap/coap.h
        opencoap/oscore.c
        opencoap/oscore.h
        opencoap/cborencoder.c
        opencoap/cborencoder.h
        openweb.c
        openweb.h)

target_compile_features(openweb PUBLIC c_std_99)
target_include_directories(openweb PUBLIC "../openweb" "../openweb/opencoap")

target_link_libraries(openweb PRIVATE bsp)
target_link_libraries(openweb PRIVATE opendrivers)
target_link_libraries(openweb PRIVATE openstack)
