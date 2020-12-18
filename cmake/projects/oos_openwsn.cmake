# build the oos_openwsn firmware

add_subdirectory(${CMAKE_SOURCE_DIR}/bsp/boards/${BOARD})
add_subdirectory(${CMAKE_SOURCE_DIR}/drivers)
add_subdirectory(${CMAKE_SOURCE_DIR}/openapps)
add_subdirectory(${CMAKE_SOURCE_DIR}/openweb)
add_subdirectory(${CMAKE_SOURCE_DIR}/openstack)
add_subdirectory(${CMAKE_SOURCE_DIR}/kernel)
add_subdirectory(${CMAKE_SOURCE_DIR}/projects/common/03oos_openwsn)