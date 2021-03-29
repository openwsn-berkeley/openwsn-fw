add_library(openweb "")

target_sources(openweb
        PRIVATE
        opencoap/coap.c
        opencoap/oscore.c
        opencoap/cborencoder.c
        openweb.c)

target_compile_features(openweb PUBLIC c_std_99)
target_include_directories(openweb
        PUBLIC
        "."
        "opencoap")

target_link_libraries(openweb PRIVATE bsp)
target_link_libraries(openweb PRIVATE opendrivers)
target_link_libraries(openweb PRIVATE openstack)

if (DARWIN AND "${BOARD}" STREQUAL "python")
    set_target_properties(openweb PROPERTIES SUFFIX ".so")
elseif (WIN32 AND "${BOARD}" STREQUAL "python")
    set_target_properties(openweb PROPERTIES SUFFIX ".pyd")
endif()
