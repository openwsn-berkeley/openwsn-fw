add_library(bsp "")

file(GLOB SRCS "${CMAKE_CURRENT_SOURCE_DIR}/*.c" "${CMAKE_CURRENT_SOURCE_DIR}/*.s")

file(GLOB
        DRIVER_SRCS
        "${CMAKE_CURRENT_SOURCE_DIR}/library/STM32F10x_StdPeriph_Lib_V3.5.0/Libraries/STM32F10x_StdPeriph_Driver/src/*.c")

file(GLOB
        CMSIS_CORE
        "${CMAKE_CURRENT_SOURCE_DIR}/library/STM32F10x_StdPeriph_Lib_V3.5.0/Libraries/CMSIS/CM3/CoreSupport/*.c")

file(GLOB
        CMSIS_DEVICE
        "${CMAKE_CURRENT_SOURCE_DIR}/library/STM32F10x_StdPeriph_Lib_V3.5.0/Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x/*.c")

target_sources(bsp
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/configure/stm32f10x_it.c
        PRIVATE
        ${SRCS}
        ${DRIVER_SRCS}
        ${CMSIS_CORE}
        ${CMSIS_DEVICE}
        ${CMAKE_SOURCE_DIR}/bsp/chips/at86rf231/radio.c)

target_include_directories(bsp
        PUBLIC
        ${CMAKE_SOURCE_DIR}/inc
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}/.
        ${CMAKE_CURRENT_SOURCE_DIR}/configure
        ${CMAKE_CURRENT_SOURCE_DIR}/library/STM32F10x_StdPeriph_Lib_V3.5.0/Libraries/STM32F10x_StdPeriph_Driver/inc
        ${CMAKE_CURRENT_SOURCE_DIR}/library/STM32F10x_StdPeriph_Lib_V3.5.0/Libraries/CMSIS/CM3/CoreSupport
        ${CMAKE_CURRENT_SOURCE_DIR}/library/STM32F10x_StdPeriph_Lib_V3.5.0/Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x/
        PRIVATE
        ${CMAKE_SOURCE_DIR}/bsp/chips/at86rf231)

