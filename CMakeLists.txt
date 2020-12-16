cmake_minimum_required(VERSION 3.14)

project(OpenWSN-FW VERSION 1.0 LANGUAGES C ASM)
message(STATUS "OpenWSN-FW:............VERSION-${PROJECT_VERSION}")

include(ExternalProject)

# set the cmake module path for easy includes
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# set cmake policies for backwards compatibility
if (POLICY CMP0076)
    cmake_policy(SET CMP0076 NEW)
endif ()

if (POLICY CMP0057)
    cmake_policy(SET CMP0057 NEW)
endif ()

# all supported projects
set(ALL_PROJECTS oos_openwsn)

if (NOT PROJECT)
    set(PROJECT "oos_openwsn" CACHE STRING "Select a project" FORCE)
endif ()

set_property(CACHE PROJECT PROPERTY STRINGS "oos_openwsn")

if (NOT BOARD)
    set(BOARD "python" CACHE STRING "Select a board" FORCE)
endif ()

set_property(CACHE BOARD PROPERTY STRINGS "python" "openmote-cc2538" "iot-lab_M3")

# force default build type to Debug unless already set (by an IDE or on the command line)
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the build type: Debug, Release, and MinSizeRel." FORCE)
endif ()

set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel")

# override shared_libs capabilities in case we are switching from cross-compile build to native build or vice-versa
if (BOARD_PYTHON OR "${BOARD}" STREQUAL "python")
    SET_PROPERTY(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS TRUE)
else ()
    SET_PROPERTY(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS FALSE)
endif ()

# search for build options
include(options)

message(STATUS "PROJECT:...............${PROJECT}")
message(STATUS "CMAKE_BUILD_TYPE:......${CMAKE_BUILD_TYPE}")
message(STATUS "CJOIN:.................${OPT-CJOIN}")
message(STATUS "UECHO:.................${OPT-UECHO}")
message(STATUS "COAP:..................${OPT-COAP}")
message(STATUS "UDP:...................${OPT-UDP}")
message(STATUS "6LOWPAN-FRAG:..........${OPT-FRAG}")
message(STATUS "PING:..................${OPT-PING}")
message(STATUS "ADAPTIVE-MSF:..........${OPT-MSF}")
message(STATUS "DAGROOT:...............${OPT-DAGROOT}")
message(STATUS "PRINTF:................${OPT-PRINTF}")

option(BOARD_CC2538 "Cross-compile for the OpenMote-CC2538 board" OFF)
option(BOARD_PYTHON "Compile for the Python simulation board" OFF)
option(BOARD_IOT_M3 "Compile for the Python simulation board" OFF)

if (BOARD_CC2538)
    set(BOARD "openmote-cc2538")
    message(STATUS "BOARD:.................${BOARD}")

    ExternalProject_Add(OpenMote-CC2538-Builder
            SOURCE_DIR ${CMAKE_SOURCE_DIR}
            BINARY_DIR ${CMAKE_BINARY_DIR}/openmote-cc2538
            CMAKE_CACHE_ARGS
                -DCMAKE_TOOLCHAIN_FILE:FILEPATH=${CMAKE_SOURCE_DIR}/cmake/toolchains/cc2538.cmake
                -DBOARD:STRING=${BOARD}
                -DPROJECT:STRING=${PROJECT}
                -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
                ${OPTIONS}
            INSTALL_COMMAND "")
    return()
endif ()

if (BOARD_IOT_M3)
    set(BOARD "iot-lab_M3")
    message(STATUS "BOARD:.................${BOARD}")

    ExternalProject_Add(IoT-LAB_M3-Builder
            SOURCE_DIR ${CMAKE_SOURCE_DIR}
            BINARY_DIR ${CMAKE_BINARY_DIR}/iot-lab_M3
            CMAKE_CACHE_ARGS
                -DCMAKE_TOOLCHAIN_FILE:FILEPATH=${CMAKE_SOURCE_DIR}/cmake/toolchains/m3.cmake
                -DBOARD:STRING=${BOARD}
                -DPROJECT:STRING=${PROJECT}
                -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
                ${OPTIONS}
            INSTALL_COMMAND "")
    return()
endif ()

if (BOARD_PYTHON)
    set(BOARD "python")
    message(STATUS "BOARD:.................${BOARD}")

    ExternalProject_Add(Python-Builder
            SOURCE_DIR ${CMAKE_SOURCE_DIR}
            BINARY_DIR ${CMAKE_BINARY_DIR}/python
            CMAKE_CACHE_ARGS
                -DBOARD:STRING=${BOARD}
                -DPROJECT:STRING=${PROJECT}
                -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
                ${OPTIONS}
            INSTALL_COMMAND "")
    return()
endif ()

if (BOARD)
    # include the generic definitions used by all libraries
    include_directories(${CMAKE_SOURCE_DIR}/inc)

    # include the projects
    include(projects/${PROJECT})
endif ()


