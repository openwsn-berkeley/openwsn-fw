find_package(Python3 COMPONENTS Interpreter Development)

if(Python3_Development_FOUND AND Python3_Interpreter_FOUND)
    message(STATUS "Found Python3 INCLUDE DIRS: ${Python3_INCLUDE_DIRS}")
    message(STATUS "Found Python3 LIBRARIES DIRS: ${Python3_LIBRARY_DIRS}")
    message(STATUS "Found Python3 LIBRARIES: ${Python3_LIBRARIES}")
else()
    message(FATAL_ERROR "Could not find Python Interpreter or Development files")
endif ()

add_library(bsp "")

file(GLOB HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/../*.h")
file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

target_sources(bsp PRIVATE ${SOURCES})

target_include_directories(bsp
        PUBLIC ..
        PUBLIC .
        ${Python3_INCLUDE_DIRS})

# target_link_libraries(bsp ${Python3_LIBRARIES})
