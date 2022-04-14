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

target_sources(${PROJECT} PUBLIC 01bsp_radio_tx.c)
target_link_libraries(${PROJECT} PUBLIC bsp)
















