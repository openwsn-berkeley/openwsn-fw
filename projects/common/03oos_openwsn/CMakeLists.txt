add_executable(${PROJECT} "")

target_sources(${PROJECT} PUBLIC openwsn.c openwsn.h)

target_link_libraries(${PROJECT} PUBLIC bsp)
target_link_libraries(${PROJECT} PUBLIC kernel)
target_link_libraries(${PROJECT} PUBLIC openstack)

add_custom_command(
        TARGET "${PROJECT}"
        POST_BUILD COMMAND ${SIZE_TOOL} ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT})