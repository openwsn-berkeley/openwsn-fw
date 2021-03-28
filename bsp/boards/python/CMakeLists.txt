add_library(bsp "")

file(GLOB HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/../*.h")
file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

target_sources(bsp PRIVATE ${SOURCES})

target_include_directories(bsp
        PUBLIC
        ..
        .
        PRIVATE
        ${Python3_INCLUDE_DIRS})

target_link_libraries(bsp PRIVATE ${Python3_LIBRARIES})

if (DARWIN AND "${BOARD}" STREQUAL "python")
    set_target_properties(bsp PROPERTIES SUFFIX ".so")
endif()
