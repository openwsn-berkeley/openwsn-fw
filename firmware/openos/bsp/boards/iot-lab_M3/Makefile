#===============================================================================
# Build variables
#===============================================================================

PREFIX			= arm-none-eabi-
CC				= @$(PREFIX)gcc
AR				= @$(PREFIX)ar
CFLAGS			= -DHSE_VALUE=\(\(uint32_t\)16000000\) -DSTM32F10X_HD -DUSE_STDPERIPH_DRIVER -ggdb -g3 -std=gnu99 -O0 -Wall -Wstrict-prototypes -mcpu=cortex-m3 $(FPU_USAGE) -mlittle-endian -mthumb -mthumb-interwork -nostartfiles
ASFLAGS			= -ggdb -g3 -mcpu=cortex-m3 $(FPU_USAGE) -mlittle-endian
AS				= @$(PREFIX)as
LINK			= @$(PREFIX)gcc
SIZE			= $(PREFIX)size
OBJCOPY			= $(PREFIX)objcopy

LINKFLAGS		= -DUSE_STDPERIPH_DRIVER -DUSE_STM32_DISCOVERY -g3 -ggdb -mcpu=cortex-m3 $(FPU_USAGE) -mlittle-endian -static -lgcc -mthumb -mthumb-interwork -nostartfiles -T$(BSP_DIR)/stm32_flash.ld

FLASHER			= $(BSP_DIR)/tools/flash.sh
FFLAGS			= $(BIN_DIR)/$(BIN)

DEBUGGER		= $(BSP_DIR)/tools/debug.sh
DFLAGS			= $(BIN_DIR)/$(ELF)

#===============================================================================
# Header files' folders
#===============================================================================

# STM32 CMSIS
INC += -I$(BSP_DIR)/library/STM32F10x_StdPeriph_Lib_V3.5.0/Libraries/STM32F10x_StdPeriph_Driver/inc
INC += -I$(BSP_DIR)/library/STM32F10x_StdPeriph_Lib_V3.5.0/Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x
INC += -I$(BSP_DIR)/library/STM32F10x_StdPeriph_Lib_V3.5.0/Libraries/CMSIS/CM3/CoreSupport

# iot-lab_M3
INC += -I$(BSP_DIR)/configure
INC += -I$(BSP_DIR)

# OpenWSN generic
INC += -I$(OPENOS_DIR)/bsp/boards
INC += -I$(OPENOS_DIR)/bsp/chips
INC += -I$(OPENOS_DIR)/drivers/common
INC += -I$(OPENOS_DIR)/openwsn

# OpenOS
INC += -I$(OPENOS_DIR)/kernel/openos

# OpenWSN network
INC += -I$(OPENOS_DIR)/openwsn/cross-layers
INC += -I$(OPENOS_DIR)/openwsn/02a-MAClow
INC += -I$(OPENOS_DIR)/openwsn/02b-MAChigh
INC += -I$(OPENOS_DIR)/openwsn/02b-MAChigh
INC += -I$(OPENOS_DIR)/openwsn/03a-IPHC
INC += -I$(OPENOS_DIR)/openwsn/03b-IPv6
INC += -I$(OPENOS_DIR)/openwsn/04-TRAN
INC += -I$(OPENOS_DIR)/openwsn/07-App/rreg
INC += -I$(OPENOS_DIR)/openwsn/07-App/rwellknown
INC += -I$(OPENOS_DIR)/openwsn/07-App/rinfo
INC += -I$(OPENOS_DIR)/openwsn/07-App/rleds
INC += -I$(OPENOS_DIR)/openwsn/07-App/r6t
INC += -I$(OPENOS_DIR)/openwsn/07-App/tcpinject
INC += -I$(OPENOS_DIR)/openwsn/07-App/udpinject
INC += -I$(OPENOS_DIR)/openwsn/07-App/ohlone
INC += -I$(OPENOS_DIR)/openwsn/07-App/tcpecho
INC += -I$(OPENOS_DIR)/openwsn/07-App/tcpprint
INC += -I$(OPENOS_DIR)/openwsn/07-App/udpecho
INC += -I$(OPENOS_DIR)/openwsn/07-App/udpprint
INC += -I$(OPENOS_DIR)/openwsn/07-App/tcprand
INC += -I$(OPENOS_DIR)/openwsn/07-App/udprand
INC += -I$(OPENOS_DIR)/openwsn/07-App/udpstorm
INC += -I$(OPENOS_DIR)/openwsn/07-App/udplatency

# Chips
INC += -I$(OPENOS_DIR)/bsp/chips/at86rf231

#===============================================================================
# Source files' folders
#===============================================================================

SRC_DIRS += $(OPENOS_DIR)/bsp/boards/iot-lab_M3
SRC_DIRS += $(OPENOS_DIR)/bsp/boards/iot-lab_M3/configure
SRC_DIRS += $(OPENOS_DIR)/bsp/boards/iot-lab_M3/library/STM32F10x_StdPeriph_Lib_V3.5.0/Libraries/STM32F10x_StdPeriph_Driver/src
SRC_DIRS += $(OPENOS_DIR)/bsp/chips/at86rf231

#SRC_DIRS += $(OPENOS_DIR)/drivers/common
#===============================================================================

BIN = $(PROJECT).hex
ELF = $(PROJECT).elf

OPENOS_PATH	 = $(realpath $(OPENOS_DIR))
CURRENT_PATH = $(realpath .)
PROJECT_PATH = $(subst $(OPENOS_PATH),,$(CURRENT_PATH))

BIN_ROOT_DIR = ./build/
BIN_DIR = ./build/$(PROJECT_PATH)/

OBJ_DIRS = $(addprefix $(BIN_DIR), $(SRC_DIRS)) 

# Get source files from folders
SRC = $(foreach sdir, $(SRC_DIRS), $(wildcard $(sdir)/*.c))

# Add single source files
SRC += $(SRC_FILES)
OBJ_DIRS += $(addprefix $(BIN_DIR), $(dir $(SRC_FILES)))

OBJ = $(patsubst %.c, $(BIN_DIR)%.o, $(SRC))
DEP = $(OBJ:.o=.d)

vpath %.c $(SRC_DIRS)

.PHONY: all checkdirs clean

all: checkdirs $(BIN)

$(BIN): $(OBJ)
	$(AS) $(ASFLAGS) $(INC) -c -o $(BIN_DIR)/startup.o $(BSP_DIR)/startup.s

	$(LINK) $(BIN_DIR)startup.o -o $(BIN_DIR)/$(PROJECT).elf -Wl,--start-group -lm -Wl,--end-group -Wl,-Map=$(BIN_DIR)$(PROJECT).map $(LINKFLAGS) $^

	$(OBJCOPY) -O ihex $(BIN_DIR)$(PROJECT).elf $(BIN_DIR)$(PROJECT).hex

checkdirs: $(OBJ_DIRS)

$(OBJ_DIRS):
	@mkdir -p $@

clean:
	@rm -rf $(BIN_ROOT_DIR)

flash: all
	$(FLASHER) $(FFLAGS)

debug: all
	$(DEBUGGER) $(DFLAGS)


-include $(DEP)

define make-goal
$1/%.o: %.c
	$(CC) $(CFLAGS) $(ASFLAGS) $(INC) -c $$< -o $$@
	$(CC) $(CFLAGS) $(ASFLAGS) $(INC) -MM $$< -MT $$@ -MT $$(<:.c=.o) -MF $$(patsubst %.o, %.d, $$@)
endef
define make-single-goal
$1: $2 
	$(CC) $(CFLAGS) $(ASFLAGS) $(INC) -c $$< -o $$@
	$(CC) $(CFLAGS) $(ASFLAGS) $(INC) -MM $$< -MT $$@ -MT $$(<:.c=.o) -MF $$(patsubst %.o, %.d, $$@)
endef

$(foreach bdir, $(OBJ_DIRS), $(eval $(call make-goal, $(bdir))))
$(foreach srcfile, $(SRC_FILES), $(eval $(call make-single-goal, $(patsubst %.c,%.o,$(addprefix $(BIN_DIR), $(srcfile))), $(srcfile))))

