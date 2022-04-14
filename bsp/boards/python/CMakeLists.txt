add_library(bsp SHARED "")

file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

if (WIN32)
    add_definitions(-DCOMPILE_DLL)
endif ()

target_sources(bsp PRIVATE ${SOURCES})

target_include_directories(bsp
        PUBLIC
        ${CMAKE_SOURCE_DIR}/inc
        ..
        .
        PRIVATE
        ${Python3_INCLUDE_DIRS})

target_link_libraries(bsp PRIVATE ${Python3_LIBRARIES})
