OpenWSN firmware: stuff that runs on a mote

Part of UC Berkeley's OpenWSN project, http://www.openwsn.org/.

Build status
------------

|              builder                                                                                                                 |      build                   | outcome
| ------------------------------------------------------------------------------------------------------------------------------------ | ---------------------------- | ------- 
| [Travis](https://travis-ci.org/openwsn-berkeley/openwsn-fw)                                                                          | compile                      | [![Build Status](https://travis-ci.org/openwsn-berkeley/openwsn-fw.png?branch=develop)](https://travis-ci.org/openwsn-berkeley/openwsn-fw)
| [OpenWSN builder](http://builder.openwsn.org/job/Firmware/board=telosb,label=master,project=oos_openwsn,toolchain=mspgcc/)           | compile (TelosB)             | [![Build Status](http://builder.openwsn.org/job/Firmware/board=telosb,label=master,project=oos_openwsn,toolchain=mspgcc/badge/icon/)](http://builder.openwsn.org/job/Firmware/board=telosb,label=master,project=oos_openwsn,toolchain=mspgcc/)
| [OpenWSN builder](http://builder.openwsn.org/job/Firmware/board=gina,label=master,project=oos_openwsn,toolchain=mspgcc/)             | compile (GINA)               | [![Build Status](http://builder.openwsn.org/job/Firmware/board=gina,label=master,project=oos_openwsn,toolchain=mspgcc/badge/icon/)](http://builder.openwsn.org/job/Firmware/board=gina,label=master,project=oos_macpong,toolchain=mspgcc/)
| [OpenWSN builder](http://builder.openwsn.org/job/Firmware/board=wsn430v13b,label=master,project=oos_openwsn,toolchain=mspgcc/)       | compile (wsn430v13b)         | [![Build Status](http://builder.openwsn.org/job/Firmware/board=wsn430v13b,label=master,project=oos_openwsn,toolchain=mspgcc/badge/icon/)](http://builder.openwsn.org/job/Firmware/board=wsn430v13b,label=master,project=oos_macpong,toolchain=mspgcc/)
| [OpenWSN builder](http://builder.openwsn.org/job/Firmware/board=wsn430v14,label=master,project=oos_openwsn,toolchain=mspgcc/)        | compile (wsn430v14)          | [![Build Status](http://builder.openwsn.org/job/Firmware/board=wsn430v14,label=master,project=oos_openwsn,toolchain=mspgcc/badge/icon/)](http://builder.openwsn.org/job/Firmware/board=wsn430v14,label=master,project=oos_macpong,toolchain=mspgcc/)
| [OpenWSN builder](http://builder.openwsn.org/job/Firmware/board=Z1,label=master,project=oos_openwsn,toolchain=mspgcc/)               | compile (Z1)                 | [![Build Status](http://builder.openwsn.org/job/Firmware/board=z1,label=master,project=oos_openwsn,toolchain=mspgcc/badge/icon/)](http://builder.openwsn.org/job/Firmware/board=z1,label=master,project=oos_macpong,toolchain=mspgcc/)
| [OpenWSN builder](http://builder.openwsn.org/job/Firmware/board=openmote-cc2538,label=master,project=oos_openwsn,toolchain=armgcc/)  | compile (OpenMote-CC2538)    | [![Build Status](http://builder.openwsn.org/job/Firmware/board=openmote-cc2538,label=master,project=oos_openwsn,toolchain=armgcc/badge/icon)](http://builder.openwsn.org/job/Firmware/board=openmote-cc2538,label=master,project=oos_openwsn,toolchain=armgcc/)
| [OpenWSN builder](http://builder.openwsn.org/job/Firmware/board=OpenMoteSTM,label=master,project=oos_openwsn,toolchain=armgcc/)      | compile (OpenMoteSTM)        | [![Build Status](http://builder.openwsn.org/job/Firmware/board=openmotestm,label=master,project=oos_openwsn,toolchain=armgcc/badge/icon)](http://builder.openwsn.org/job/Firmware/board=openmotestm,label=master,project=oos_openwsn,toolchain=armgcc/)
| [OpenWSN builder](http://builder.openwsn.org/job/Firmware/board=IoT-LAB_M3,label=master,project=oos_openwsn,toolchain=armgcc/)       | compile (IoT-LAB_M3)         | [![Build Status](http://builder.openwsn.org/job/Firmware/board=iot-lab_M3,label=master,project=oos_openwsn,toolchain=armgcc/badge/icon)](http://builder.openwsn.org/job/Firmware/board=iot-lab_M3,label=master,project=oos_openwsn,toolchain=armgcc/)
| [OpenWSN builder](http://builder.openwsn.org/job/Firmware/board=Python,label=master,project=oos_openwsn,toolchain=gcc/)              | compile (Python, simulation) | [![Build Status](http://builder.openwsn.org/job/Firmware/board=python,label=master,project=oos_openwsn,toolchain=gcc/badge/icon)](http://builder.openwsn.org/job/Firmware/board=python,label=master,project=oos_openwsn,toolchain=gcc/)
| [OpenWSN builder](http://builder.openwsn.org/job/Docs/)                                                                              | publish documentation        | [![Build Status](http://builder.openwsn.org/job/Docs/badge/icon)](http://builder.openwsn.org/job/Docs/)

Documentation
-------------

- overview: https://openwsn.atlassian.net/wiki/
- source code: http://openwsn-berkeley.github.io/firmware/
