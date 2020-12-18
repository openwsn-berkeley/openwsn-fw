cmake_minimum_required(VERSION 3.14)

project(OpenWSN-FW VERSION 1.0 LANGUAGES C ASM)

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

# choose project and board
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
if ("${BOARD}" STREQUAL "python")
    add_definitions("-DPYTHON_BOARD")
    set(BUILD_SHARED_LIBS ON)
    set_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS TRUE)
else ()
    set_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS FALSE)
    set(BUILD_SHARED_LIBS OFF)
endif ()

if ("${BOARD}" STREQUAL "" OR "${PROJECT}" STREQUAL "")
    return()
endif ()

# search for build options
include(options)

# include the generic definitions used by all libraries
include_directories(${CMAKE_SOURCE_DIR}/inc)

# include the projects
include(projects/${PROJECT})

message(STATUS "OpenWSN-FW:............VERSION-${PROJECT_VERSION}")
message(STATUS "BOARD:.................${BOARD}")
message(STATUS "PROJECT:...............${PROJECT}")
message(STATUS "BUILD_SHARED_LIBS......${BUILD_SHARED_LIBS}")
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
