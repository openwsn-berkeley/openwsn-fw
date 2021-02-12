if ("${BOARD}" STREQUAL "python")
    add_library(${PROJECT} SHARED)

    set_target_properties(${PROJECT} PROPERTIES PREFIX "" OUTPUT_NAME openmote LINKER_LANGUAGE C)
else ()
    add_executable(${PROJECT} "")

    add_custom_command(
            TARGET "${PROJECT}"
            POST_BUILD COMMAND ${SIZE_TOOL} ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT})
endif ()


target_sources(${PROJECT} PUBLIC openwsn.c openwsn.h)

target_link_libraries(${PROJECT} PUBLIC bsp)
target_link_libraries(${PROJECT} PUBLIC kernel)
target_link_libraries(${PROJECT} PUBLIC opendrivers)
target_link_libraries(${PROJECT} PUBLIC openstack)
target_link_libraries(${PROJECT} PUBLIC openapps)
target_link_libraries(${PROJECT} PUBLIC openweb)
