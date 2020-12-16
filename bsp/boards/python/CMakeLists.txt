#######################
## Python Packaging  ##
#######################
find_package(Python3 COMPONENTS Interpreter Development)

set(SETUP_PY_IN "setup.py.in")
set(SETUP_PY "${CMAKE_CURRENT_BINARY_DIR}/openwsn/setup.py")
set(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/build/timestamp")

configure_file(${SETUP_PY_IN} ${SETUP_PY})

add_custom_command(OUTPUT ${OUTPUT}
        COMMAND ${Python3_EXECUTABLE} ${SETUP_PY} build
        COMMAND ${CMAKE_COMMAND} -E touch ${OUTPUT}
        DEPENDS ${SOURCES})

add_custom_target(${PROJECT} ALL DEPENDS ${OUTPUT} SOURCES ${SOURCES})

add_custom_target(install_${PROJECT} ALL DEPENDS ${OUTPUT} COMMAND ${Python3_EXECUTABLE} ${SETUP_PY} install --user --prefix=)
