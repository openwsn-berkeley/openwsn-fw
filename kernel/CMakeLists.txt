add_library(kernel "")

target_sources(kernel
        PRIVATE
        openos/scheduler.c)

target_compile_features(kernel PUBLIC c_std_99)
target_include_directories(kernel
        PUBLIC
        .
        ${CMAKE_SOURCE_DIR}/inc
        PRIVATE
        openos)

target_link_libraries(kernel PRIVATE bsp)
