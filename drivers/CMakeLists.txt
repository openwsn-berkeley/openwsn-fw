add_library(opendrivers "")

target_sources(opendrivers
        PRIVATE
        opendrivers.c
        common/opensensors.c
        common/openserial.c
        common/opentimers.c
        common/crypto/aes128.c
        common/crypto/ccms.c
        common/crypto/hkdf.c
        common/crypto/hmac.c
        common/crypto/sha224-256.c
        common/crypto/usha.c
        common/openhdlc.c)

target_compile_features(opendrivers PUBLIC c_std_99)
target_include_directories(opendrivers
        PUBLIC "."
        PUBLIC "../inc"
        PUBLIC "common"
        PUBLIC "common/crypto")

target_link_libraries(opendrivers PUBLIC bsp)
target_link_libraries(opendrivers PUBLIC kernel)

