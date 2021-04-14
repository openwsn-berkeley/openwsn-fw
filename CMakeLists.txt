cmake_minimum_required(VERSION 3.13)

project(OpenWSN-FW VERSION 2.0 LANGUAGES C ASM)

# set the cmake module path for easy includes
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# find Unix-like OS
if (UNIX AND NOT APPLE)
    set(LINUX TRUE)
elseif (UNIX AND APPLE)
    set(DARWIN TRUE)
endif ()

# set cmake policies for backwards compatibility (relative paths to absolute).
if (POLICY CMP0076)
    cmake_policy(SET CMP0076 NEW)
endif ()

if (POLICY CMP0057)
    cmake_policy(SET CMP0057 NEW)
endif ()

if (NOT PYTHON_VERSION)
    # Python3 Interpreter is always need (for flashing motes and exporting the firmware defines from opendefs & sixtop)
    find_package(Python3 COMPONENTS Interpreter REQUIRED)
else ()
    find_package(Python3 ${PYTHON_VERSION} EXACT COMPONENTS Interpreter REQUIRED)
endif ()

# If PROJECT not set on cmd or by the IDE set to oos_openwsn (FORCE overwrites the cache value)
if (NOT PROJECT)
    set(PROJECT "oos_openwsn" CACHE STRING "Select a project" FORCE)
endif ()
set_property(CACHE PROJECT PROPERTY STRINGS "oos_openwsn")

# If BOARD not set on cmd or by the IDE set to python (FORCE overwrites the cache value)
if (NOT BOARD)
    set(BOARD "python" CACHE STRING "Select a board" FORCE)
endif ()
set_property(CACHE BOARD PROPERTY STRINGS "python" "openmote-cc2538" "iot-lab_M3")

# If CMAKE_BUILD_TYPE not set on cmd or by the IDE set to Debug (FORCE overwrites the cache value)
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the build type: Debug, Release, and MinSizeRel." FORCE)
endif ()
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel")

# if BOOTLOAD is not set on the cmd line (or by the IDE), set to empty string
if (NOT BOOTLOAD)
    if (WIN32)
        set(BOOTLOAD "" CACHE STRING "Select COM* ports to flash." FORCE)
    elseif (LINUX)
        set(BOOTLOAD "" CACHE STRING "Select /dev/ttyUSB* ports to flash." FORCE)
    else ()
        set(BOOTLOAD "" CACHE STRING "Select /dev/tty.usbserial-* ports to flash." FORCE)
    endif ()
endif ()

# sanitizer flag when building for python and using Clang compiler
if (CMAKE_C_COMPILER_ID STREQUAL "Clang" AND "${BOARD}" STREQUAL "python")
    option(SANITIZERS "Enable sanitizing of the *.so/*.dylib libraries in the Python extension" OFF)

    if (SANITIZERS)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -shared-libasan -fno-omit-frame-pointer")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address -shared-libasan -fno-omit-frame-pointer")
    endif ()
else ()
    set(SANITIZERS OFF)
endif ()

# parse the opendefs and sixtop definitions (always run)
add_custom_target(def-parser ALL COMMAND ${Python3_EXECUTABLE} def_exporter.py WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/cmake)

# override shared_libs capabilities in case we are switching from cross-compile build to native build or vice-versa
if ("${BOARD}" STREQUAL "python")
    add_definitions("-DPYTHON_BOARD")
    set(BUILD_SHARED_LIBS ON)
    set_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS TRUE)

    # Add these flags to the system compiler when building for Python board
    if ((CMAKE_C_COMPILER_ID STREQUAL "Clang" OR CMAKE_C_COMPILER_ID STREQUAL "GNU") AND "${BOARD}" STREQUAL "python")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic -Wall -Wextra -Wunused -Wuninitialized")
    endif ()

    if (WIN32)
        set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
    endif ()

    if (NOT "${BOOTLOAD}" STREQUAL "")
        message(WARNING "Python boards cannot be flashed")
    endif ()

    if (NOT PYTHON_VERSION)
        # Python3 Interpreter is always need (for flashing motes and exporting the firmware defines from opendefs & sixtop)
        find_package(Python3 COMPONENTS Development REQUIRED)
    else ()
        find_package(Python3 ${PYTHON_VERSION} EXACT COMPONENTS Development REQUIRED)
    endif ()

    message(STATUS "Found Python3 INCLUDE DIRS: ${Python3_INCLUDE_DIRS}")
    message(STATUS "Found Python3 LIBRARIES DIRS: ${Python3_LIBRARY_DIRS}")
    message(STATUS "Found Python3 LIBRARIES: ${Python3_LIBRARIES}")

    link_directories(BEFORE ${Python3_LIBRARY_DIRS})
else ()
    set_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS FALSE)
    set(BUILD_SHARED_LIBS OFF)
endif ()

if ("${BOARD}" STREQUAL "" OR "${PROJECT}" STREQUAL "")
    return()
endif ()

# Pull in build options
include(options)

# Pull in cmake file responsible for building the project
include(projects/${PROJECT})

# general build settings
message("\n*** GENERAL BUILD SETTINGS ***")
message(STATUS "OPENWSN-FW:..................VERSION-${PROJECT_VERSION}")
message(STATUS "COMPILER TOOLCHAIN:..........${CMAKE_C_COMPILER}")
message(STATUS "COMPILER VERSION:............${CMAKE_C_COMPILER_VERSION}")
message(STATUS "CMAKE_GENERATOR:.............${CMAKE_GENERATOR}")
message(STATUS "PYTHON3 VERSION:.............${Python3_VERSION}")
message(STATUS "BOARD:.......................${BOARD}")
message(STATUS "PROJECT:.....................${PROJECT}")
message(STATUS "BUILD_SHARED_LIBS............${BUILD_SHARED_LIBS}")
message(STATUS "CMAKE_BUILD_TYPE:............${CMAKE_BUILD_TYPE}")
message(STATUS "SANITIZERS:..................${SANITIZERS}")
message(STATUS "PROGRAM MOTES:...............[${BOOTLOAD}]")

# project-wide settings
message("\n*** PROJECT-WIDE OPTIONS ***")
message(STATUS "PRINTF:......................${OPT-PRINTF}")
message(STATUS "LOG LEVEL:...................${LOG_LEVEL}")
message(STATUS "CRYPTO HARDWARE:.............${OPT-CRYPTO-HW}")

# stack settings
message("\n*** OPENSTACK OPTIONS ***")
message(STATUS "CHANNEL HOPPING:.............${IEEE154E_CHANNEL}")
message(STATUS "ADAPTIVE-MSF:................${OPT-MSF}")
message(STATUS "FORCE TOPOLOGY:..............${OPT-FORCE-TOPO}")
message(STATUS "L2 SECURITY:.................${OPT-L2-SEC}")
message(STATUS "6LOWPAN-FRAG:................${OPT-FRAG}")
message(STATUS "PING:........................${OPT-PING}")
message(STATUS "UDP:.........................${OPT-UDP}")
message(STATUS "PACKETQUEUE_LENGTH:..........${PACKETQUEUE_LENGTH}")
message(STATUS "PANID:.......................${PANID}")
message(STATUS "DAGROOT:.....................${OPT-DAGROOT}")

# web settings
message("\n*** OPENWEB OPTIONS ***")
message(STATUS "COAP:........................${OPT-COAP}")
message(STATUS "DEFAULT COAP PORT:...........${DEFAULT_COAP_PORT}")

# app settings
message("\n*** OPENAPPS OPTIONS ***")
message(STATUS "CJOIN:.......................${OPT-CJOIN}")
message(STATUS "CSTORM:......................${OPT-CSTORM}")
message(STATUS "CEXAMPLE:....................${OPT-CEXAMPLE}")
message(STATUS "CLED:........................${OPT-CLED}")
message(STATUS "CINFRARED:...................${OPT-CINFRARED}")
message(STATUS "CINFO:.......................${OPT-CINFO}")
message(STATUS "C6T:.........................${OPT-C6T}")
message(STATUS "UECHO:.......................${OPT-UECHO}")
message(STATUS "RRT:.........................${OPT-RRT}")
message(STATUS "WELLKNOWN....................${OPT-WELLKNOWN}")
message("")
