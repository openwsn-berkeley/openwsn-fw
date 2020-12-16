set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(SIZE_TOOL arm-none-eabi-size)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

UNSET(CMAKE_C_FLAGS CACHE)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wextra")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wpedantic")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wstrict-prototypes")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mthumb")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=cortex-m3")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffunction-sections")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fdata-sections")
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -flto")

UNSET(CMAKE_EXE_LINKER_FLAGS CACHE)

# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto")
set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -T${CMAKE_SOURCE_DIR}/bsp/boards/iot-lab_M3/stm32_flash.ld")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -nostartfiles -specs=nosys.specs -specs=nano.specs")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Map,iotlab-m3.map")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections")

# put everything in cache
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}" CACHE STRING "" FORCE)
SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS}" CACHE STRING "" FORCE)

add_definitions(-DIOTLAB_M3)
add_definitions(-DSTM32F10X_HD)
add_definitions(-DUSE_STDPERIPH_DRIVER)
add_definitions(-DUSE_STM32_DISCOVERY)
add_definitions(-DHSE_VALUE=16000000)
