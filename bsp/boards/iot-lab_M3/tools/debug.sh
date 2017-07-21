#!/bin/bash

FILE=$(readlink -f "$0")
BIN_FOLDER=$(dirname "${FILE}")

openocd -f "${BIN_FOLDER}/iot-lab_m3_jtag.cfg" \
	-f "target/stm32f1x.cfg" \
	-c "init" \
	-c "reset halt" \
	-c "targets"

