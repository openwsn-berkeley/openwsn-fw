#!/bin/bash

if [ -d "./bsp/boards/nrf52840/tools/nrfjprog" ]; then

  echo -e "\n\t FLASHING $1 to nrf52840dk\n"

  ./bsp/boards/nrf52840/tools/nrfjprog/nrfjprog -f nrf52 --program $1 --sectorerase
  ./bsp/boards/nrf52840/tools/nrfjprog/nrfjprog -f nrf52 --reset

else

  echo -e "\n\n[WARNING]\n"
  echo -e "To be able to flash the Nordic nRF52840 Development Kit you will need to install"
  echo -e "both the latest JLink Software (https://www.segger.com/downloads/jlink/) and"
  echo -e "nRF5x-Command-Line-Tools (https://www.nordicsemi.com/eng/Products/nRF52840#Downloads)"
  echo -e "into the directory './bsp/boards/nrf52840/tools/nrfjprog'.\n"

fi
