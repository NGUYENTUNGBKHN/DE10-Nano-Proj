# Makefile created library project
TARGET_NAME:=hwlib
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
CCFLAGS+= -Dsoc_cv_av -D_USE_STDLIB

ASMFLAG=$(CCFLAGS)

SRC_EXCLUDE+=$(wildcard $(ROOT_PROJ)/src/hwmgr/soc_a10/*.c)
SRC_EXCLUDE+=$(wildcard $(ROOT_PROJ)/src/hwmgr/soc_a10/*.s)
SRC_EXCLUDE+=$(wildcard $(ROOT_PROJ)/src/utils/alt_base.c)
SRC_EXCLUDE+=$(wildcard $(ROOT_PROJ)/src/utils/alt_retarget_gnu.c)


SRC_INCLUDE+=$(wildcard $(ROOT_PROJ)/src/*/*.c)
SRC_INCLUDE+=$(wildcard $(ROOT_PROJ)/src/*/*/*.c)
SRC_INCLUDE+=$(wildcard $(ROOT_PROJ)/src/hwmgr/soc_cv_av/alt_bridge_f2s_gnu.s)



SRC_FILES:=$(filter-out $(SRC_EXCLUDE), $(SRC_INCLUDE))

INC_FOLDERS=\
	$(ROOT_PROJ)/include\
	$(ROOT_PROJ)/include/safeclib\
	$(ROOT_PROJ)/include/soc_cv_av\
	$(ROOT_PROJ)/src/safeclib\
	


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


