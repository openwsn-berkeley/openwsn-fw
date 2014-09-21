PROJECT		:= 01bsp_radio_tx
#===============================================================================

OPENOS_DIR	:= ../../../
BSP_DIR		:= $(OPENOS_DIR)/bsp/boards/iot-lab_M3/

#===============================================================================

SRC_DIRS	= $(OPENOS_DIR)/projects/common/01bsp_radio_tx

SRC_DIRS	+= $(OPENOS_DIR)/drivers/common
SRC_DIRS	+= $(OPENOS_DIR)/kernel/openos

SRC_DIRS 	+= $(OPENOS_DIR)/openwsn

SRC_DIRS 	+= $(OPENOS_DIR)/openwsn/cross-layers
SRC_DIRS 	+= $(OPENOS_DIR)/openwsn/02a-MAClow
SRC_DIRS 	+= $(OPENOS_DIR)/openwsn/02b-MAChigh
SRC_DIRS 	+= $(OPENOS_DIR)/openwsn/03a-IPHC
SRC_DIRS 	+= $(OPENOS_DIR)/openwsn/03b-IPv6
SRC_DIRS 	+= $(OPENOS_DIR)/openwsn/04-TRAN

SRC_DIRS 	+= $(OPENOS_DIR)/openwsn/07-App/rreg
SRC_DIRS 	+= $(OPENOS_DIR)/openwsn/07-App/rwellknown
SRC_DIRS 	+= $(OPENOS_DIR)/openwsn/07-App/rinfo

SRC_DIRS 	+= $(OPENOS_DIR)/openwsn/07-App/rleds

SRC_DIRS 	+= $(OPENOS_DIR)/openwsn/07-App/r6t

SRC_DIRS 	+= $(OPENOS_DIR)/openwsn/07-App/tcpinject
SRC_DIRS 	+= $(OPENOS_DIR)/openwsn/07-App/udpinject

SRC_DIRS 	+= $(OPENOS_DIR)/openwsn/07-App/ohlone
SRC_DIRS 	+= $(OPENOS_DIR)/openwsn/07-App/tcpecho
SRC_DIRS 	+= $(OPENOS_DIR)/openwsn/07-App/tcpprint
SRC_DIRS 	+= $(OPENOS_DIR)/openwsn/07-App/udpecho
SRC_DIRS 	+= $(OPENOS_DIR)/openwsn/07-App/udpprint
SRC_DIRS 	+= $(OPENOS_DIR)/openwsn/07-App/tcprand
SRC_DIRS 	+= $(OPENOS_DIR)/openwsn/07-App/udprand
SRC_DIRS 	+= $(OPENOS_DIR)/openwsn/07-App/udpstorm
SRC_DIRS 	+= $(OPENOS_DIR)/openwsn/07-App/udplatency

#===============================================================================

include $(BSP_DIR)/Makefile
