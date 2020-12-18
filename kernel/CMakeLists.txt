add_library(kernel "")

target_sources(kernel
        PUBLIC
        scheduler.h
        openos/scheduler_types.h
        PRIVATE
        openos/scheduler.c)

target_compile_features(kernel PUBLIC c_std_99)
target_include_directories(kernel
        PUBLIC "."
        PRIVATE "openos")

target_link_libraries(kernel PRIVATE bsp)