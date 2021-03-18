if ("${BOARD}" STREQUAL "python")
    add_library(${PROJECT} SHARED)

    set_target_properties(${PROJECT} PROPERTIES PREFIX "" OUTPUT_NAME openmote LINKER_LANGUAGE C)

    target_include_directories(${PROJECT} PRIVATE ${Python3_INCLUDE_DIRS})
    target_link_libraries(${PROJECT} PRIVATE ${Python3_LIBRARIES})
else ()
    add_executable(${PROJECT} "")

    add_custom_command(
            TARGET ${PROJECT}
            POST_BUILD COMMAND ${SIZE_TOOL} ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT}
            POST_BUILD COMMAND ${OBJCOPY} -O ihex "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT}" "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT}.ihex")

    if (NOT ${BOOTLOAD} STREQUAL "")
        string(REPLACE " " ";" MOTE_LIST ${BOOTLOAD})
        foreach (MOTE ${MOTE_LIST})
            string(REPLACE "/dev/" "" MOTE_ID ${MOTE})
            add_custom_target("flash-mote-${MOTE_ID}" ALL
                    COMMAND ${Python3_EXECUTABLE} ${BOOTLOADER} ${ARGS} ${BIN_FILE} -p ${MOTE}
                    DEPENDS ${BIN_FILE})
            add_dependencies("flash-mote-${MOTE_ID}" ${PROJECT})
        endforeach ()
    endif ()

endif ()


target_sources(${PROJECT} PUBLIC openwsn.c)

target_link_libraries(${PROJECT} PUBLIC bsp)
target_link_libraries(${PROJECT} PUBLIC kernel)
target_link_libraries(${PROJECT} PUBLIC opendrivers)
target_link_libraries(${PROJECT} PUBLIC openstack)
target_link_libraries(${PROJECT} PUBLIC openapps)
target_link_libraries(${PROJECT} PUBLIC openweb)
