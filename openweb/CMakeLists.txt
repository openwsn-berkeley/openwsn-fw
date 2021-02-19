add_library(openweb "")

target_sources(openweb
        PRIVATE
        opencoap/coap.c
        opencoap/oscore.c
        opencoap/cborencoder.c
        openweb.c)

target_compile_features(openweb PUBLIC c_std_99)
target_include_directories(openweb
        INTERFACE
        "../openweb"
        "../openweb/opencoap")

target_link_libraries(openweb PRIVATE bsp)
target_link_libraries(openweb PRIVATE opendrivers)
target_link_libraries(openweb PRIVATE openstack)
