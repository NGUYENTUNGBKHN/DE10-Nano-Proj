# Makefile created library project
TARGET_NAME:=JSLWare
BUILD_DIR:=_build
MAKE_COMMON:=../common
LIB_OUTPUT_PATH = $(BUILD_DIR)/$(TARGET_NAME).a
# This is the root project directory and the core directory
ROOT_PROJ:=./
# Core directory for the project



# This is the Flags for the compiler
CPU=-mcpu=cortex-a9 -marm
FPU=-mfloat-abi=hard -mfpu=vfpv3-fp16
OPT=-O0 -g
CCFLAGS+= $(CPU) $(FPU) $(FLOAT) $(OPT)
CCFLAGS+= -mno-unaligned-access -std=c99 -MD -MMD
CCFLAGS+= -DLIB_JSL -DPRJ_OS_THREADX_UITRON -DPRJ_IVIZION2 -D_DEBUG_FRAM_MEASURE_POST_DELAY

ASMFLAG=$(CCFLAGS)

SRC_EXCLUDE+=$(wildcard $(ROOT_PROJ)/src/cortex_a9/cpu_api_ds5.asm)


SRC_INCLUDE+=$(wildcard $(ROOT_PROJ)/src/*/*.c)
SRC_INCLUDE+=$(wildcard $(ROOT_PROJ)/src/*/*/*.c)


SRC_FILES:=$(filter-out $(SRC_EXCLUDE), $(SRC_INCLUDE))

INC_FOLDERS=\
	$(ROOT_PROJ)/inc\
	$(ROOT_PROJ)/src\
	$(ROOT_PROJ)/src/gpio\


.PHONY:	default help

default: $(TARGET_NAME)

# Include common makefile. File contains TOOLCHAIN directory and rules for building targets
include $(MAKE_COMMON)/Makefile.common

# Call function to define targets 
$(call define_library, $(TARGET_NAME), $(LIB_OUTPUT_PATH))

define create_library
@echo Creating library: $($@)
$(NO_ECHO)$(AR) $($@) $^
@echo Done
endef
$(TARGET_NAME):
	$(create_library)

# Help target
help:
	@echo  help


