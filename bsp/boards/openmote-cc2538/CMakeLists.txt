add_library(bsp STATIC "")

file(GLOB BSP_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/headers/*.h")
file(GLOB BSP_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/source/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/source/*.c")
file(GLOB HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/../*.h")
file(GLOB CHIPS_H
        "${CMAKE_CURRENT_SOURCE_DIR}/../../chips/sht21/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../chips/adxl346/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../chips/max44009/*.h")

file(GLOB CHIPS_C
        "${CMAKE_CURRENT_SOURCE_DIR}/../../chips/adxl346/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../chips/sht21/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../chips/max44009/*.c")

target_sources(bsp
        PRIVATE
        ${BSP_SOURCES}
        ${CHIPS_C}
        adc_sensor.c
        adc_sensor.h
        board.c
        board_info.h
        cc2538rf.h
        debugpins.c
        eui64.c
        i2c.c
        leds.c
        pwm.c
        pwm.h
        radio.c
        sctimer.c
        sensors.c
        startup_gcc.c
        uart.c)

if (OPT-CRYPTO-HW)
    target_sources(bsp PRIVATE cryptoengine.c)
endif ()

target_include_directories(bsp
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/.
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../../chips)
