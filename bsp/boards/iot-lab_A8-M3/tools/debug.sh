#!/bin/bash

FILE=$(readlink -f "$0")
BIN_FOLDER=$(dirname "${FILE}")

openocd -f "${BIN_FOLDER}/iot-lab_m3_jtag.cfg" \
	-f "target/stm32f1x.cfg" \
  -c "tcl_port 6333"
  -c "telnet_port 4444"
	-c "init" \
	-c "targets" \
	-c "reset halt"

