add_library(drivers "")

target_sources(drivers
        PRIVATE
        common/crypto/aes128.c
        common/crypto/aes128.h
        common/crypto/ccms.c
        common/crypto/ccms.h
        common/crypto/hkdf.c
        common/crypto/hmac.c
        common/crypto/sha-private.h
        common/crypto/sha.h
        common/crypto/sha224-256.c
        common/crypto/usha.c
        common/openhdlc.c
        common/openhdlc.h
        common/opensensors.c
        common/opensensors.h
        common/openserial.c
        common/openserial.h
        common/opentimers.c
        common/opentimers.h)

target_compile_features(drivers PUBLIC c_std_99)
target_include_directories(drivers
        PUBLIC "."
        PUBLIC "common"
        PUBLIC "common/crypto"
        PRIVATE "${CMAKE_SOURCE_DIR}/openstack/02a-MAClow"
        PRIVATE "${CMAKE_SOURCE_DIR}/openstack/02b-MAChigh"
        PRIVATE "${CMAKE_SOURCE_DIR}/openstack/03a-IPHC"
        PRIVATE "${CMAKE_SOURCE_DIR}/openstack/03b-IPv6"
        PRIVATE "${CMAKE_SOURCE_DIR}/openstack/cross-layers")

target_link_libraries(drivers PUBLIC bsp)
target_link_libraries(drivers PUBLIC kernel)
