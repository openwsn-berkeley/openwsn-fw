# OpenWSN firmware
This repository is a fork of the main OpenWSN project. The OpenWSN project aims at building a reference implementation of the 6TiSCH networking stack. The OpenWSN-FW project works in tandem with [OpenVisualizer](https://github.com/TimothyClaeys/openvisualizer) (branch: `python3`). 

This fork currently supports three target platforms:

* **python** compiles the firmware code as a Python C extension. It can then be used to emulated several motes and simulate a 6TiSCH network through [OpenVisualizer](https://github.com/TimothyClaeys/openvisualizer).
* **openmote-cc2538** a Cortex-M3 based SoC ([cc2538](https://www.ti.com/lit/ug/swru319c/swru319c.pdf?ts=1602851801717&ref_url=https%253A%252F%252Fwww.google.com%252F)) with an IEEE 802.15.4 compliant radio
* **iot-lab_M3** a Cortex-M3 based CPU with the AR86RF231 radio

| ![openv-client](docs/images/stack.png) |
| :----------------------------------------------------------: |
| *Overview of the different modules of the OpenWSN stack* |

## Building

To build the project we use `cmake`. Before you can start compiling, you need to configure the build process (choose a target platform, a project and the development toolchain). 

### Python board

The following command shows how to configure and build for the `python` board (here we can omit the toolchain parameter).

```bash
$ mkdir build && cd build
$ cmake .. -DBOARD=python -DPROJECT=oos_openwsn
$ cmake --build build
```

To install the compiled Python C extension in the Python site-packages (so you can import it in your Python code), run:

```bash
$ cmake --build . --target install_oos_openwsn
```

To test if everything went well, run:

```bash
$ python3 -c "import openwsn"
```

If the command returns without error, the module was installed correctly.

### OpenMote-CC2538

To compile for the openmote-cc2538 platform we can use the following commands (inside the `build` directory):

```bash
$ cmake .. -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/cc2538.cmake -DBOARD=openmote-cc2538 -DPROJECT=oos_openwsn
$ cmake --build build
```

## Build status

| Builder |    Target    | Build status |
|:----------|:----------------|:-------------|
|[Travis](https://travis-ci.org/github/TimothyClaeys/openwsn-fw)|`Python`			   |[![Build Status](https://travis-ci.org/TimothyClaeys/openwsn-fw.svg?branch=pr_openwsn_riot)](https://travis-ci.org/TimothyClaeys/openwsn-fw)|
|[Travis](https://travis-ci.org/github/TimothyClaeys/openwsn-fw)|`OpenMote-CC2538`   |[![Build Status](https://travis-ci.org/TimothyClaeys/openwsn-fw.svg?branch=pr_openwsn_riot)](https://travis-ci.org/TimothyClaeys/openwsn-fw)|
|[Travis](https://travis-ci.org/github/TimothyClaeys/openwsn-fw)|`IoT-LAB_M3`        |[![Build Status](https://travis-ci.org/TimothyClaeys/openwsn-fw.svg?branch=pr_openwsn_riot)](https://travis-ci.org/TimothyClaeys/openwsn-fw)|


