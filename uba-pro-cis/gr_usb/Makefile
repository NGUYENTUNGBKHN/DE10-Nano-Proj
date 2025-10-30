# Makefile created library project
TARGET_NAME:=fatfs
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

ASMFLAG=$(CCFLAGS)

SRC_EXCLUDE=\


SRC_INCLUDE+=$(wildcard $(ROOT_PROJ)/src/*.c)
SRC_INCLUDE+=$(wildcard $(ROOT_PROJ)/src/com_custom/*.c)
SRC_INCLUDE+=$(wildcard $(ROOT_PROJ)/src/com_dfu/*.c)
SRC_INCLUDE+=$(wildcard $(ROOT_PROJ)/src/com_hid/*.c)
SRC_INCLUDE+=$(wildcard $(ROOT_PROJ)/src/fisf/fsiftest/*.c)
SRC_INCLUDE+=$(wildcard $(ROOT_PROJ)/src/fisf/src/*.c)
SRC_INCLUDE+=$(wildcard $(ROOT_PROJ)/src/gr_comm/*.c)
SRC_INCLUDE+=$(wildcard $(ROOT_PROJ)/src/gr_file_v1.31-201229/src/grp_fs/base/*.c)
SRC_INCLUDE+=$(wildcard $(ROOT_PROJ)/src/gr_file_v1.31-201229/src/grp_fs/lib/grp_char/*.c)
SRC_INCLUDE+=$(wildcard $(ROOT_PROJ)/src/gr_file_v1.31-201229/src/grp_fs/lib/grp_mem/*.c)
SRC_INCLUDE+=$(wildcard $(ROOT_PROJ)/src/gr_file_v1.31-201229/src/grp_fs/lib/grp_stdio/*.c)
SRC_INCLUDE+=$(wildcard $(ROOT_PROJ)/src/gr_file_v1.31-201229/src/grp_fs/lib/grp_time_lib/*.c)
SRC_INCLUDE+=$(wildcard $(ROOT_PROJ)/src/gr_file_v1.31-201229/src/mdep_vos2.xx/base/*.c)
SRC_INCLUDE+=$(wildcard $(ROOT_PROJ)/src/gr_file_v1.31-201229/src/mdep_vos2.xx/lib/grp_mem/*.c)
SRC_INCLUDE+=$(wildcard $(ROOT_PROJ)/src/gr_file_v1.31-201229/src/mdep_vos2.xx/lib/grp_sem/*.c)
SRC_INCLUDE+=$(wildcard $(ROOT_PROJ)/src/gr_file_v1.31-201229/src/mdep_vos2.xx/lib/grp_time/*.c)
SRC_INCLUDE+=$(wildcard $(ROOT_PROJ)/src/gr_usb_d/common/*.c)
SRC_INCLUDE+=$(wildcard $(ROOT_PROJ)/src/gr_usb_d/peri/pcd/*.c)
SRC_INCLUDE+=$(wildcard $(ROOT_PROJ)/src/gr_usb_d/peri/perid/*.c)
SRC_INCLUDE+=$(wildcard $(ROOT_PROJ)/src/gr_usb_h/hcm/bus/*.c)
SRC_INCLUDE+=$(wildcard $(ROOT_PROJ)/src/gr_usb_h/hcm/cmem/*.c)
SRC_INCLUDE+=$(wildcard $(ROOT_PROJ)/src/gr_usb_h/hcm/ctr/*.c)
SRC_INCLUDE+=$(wildcard $(ROOT_PROJ)/src/gr_usb_h/hcm/hcd/*.c)
SRC_INCLUDE+=$(wildcard $(ROOT_PROJ)/src/gr_usb_h/usbm/cnf_sft/*.c)
SRC_INCLUDE+=$(wildcard $(ROOT_PROJ)/src/gr_usb_h/usbm/common/*.c)
SRC_INCLUDE+=$(wildcard $(ROOT_PROJ)/src/gr_usb_h/usbm/usbd/*.c)
SRC_INCLUDE+=$(wildcard $(ROOT_PROJ)/src/gr_usb_hdcm/*.c)
SRC_INCLUDE+=$(wildcard $(ROOT_PROJ)/src/gr_vos/*.c)
SRC_INCLUDE+=$(wildcard $(ROOT_PROJ)/src/ms_c/msctest/*.c)
SRC_INCLUDE+=$(wildcard $(ROOT_PROJ)/src/ms_c/src/atapi/*.c)
SRC_INCLUDE+=$(wildcard $(ROOT_PROJ)/src/ms_c/src/bot/*.c)
SRC_INCLUDE+=$(wildcard $(ROOT_PROJ)/src/ms_c/src/cbi/*.c)
SRC_INCLUDE+=$(wildcard $(ROOT_PROJ)/src/ms_c/src/com/*.c)
SRC_INCLUDE+=$(wildcard $(ROOT_PROJ)/src/ms_c/src/scsi/*.c)
SRC_INCLUDE+=$(wildcard $(ROOT_PROJ)/src/ms_c/src/sff8070i/*.c)
SRC_INCLUDE+=$(wildcard $(ROOT_PROJ)/src/ms_c/src/ufi/*.c)

SRC_FILES:=$(filter-out $(SRC_EXCLUDE), $(SRC_INCLUDE))

INC_FOLDERS=\
	$(ROOT_PROJ)/src\
	$(ROOT_PROJ)/src/com_custom\
	$(ROOT_PROJ)/src/com_dfu\
	$(ROOT_PROJ)/src/com_hid\
	$(ROOT_PROJ)/src/fsif/fsiftest\
	$(ROOT_PROJ)/src/fsif/src\
	$(ROOT_PROJ)/src/gr_comm\
	$(ROOT_PROJ)/src/gr_file_v1.31-201229/src/grp_fs/base\
	$(ROOT_PROJ)/src/gr_file_v1.31-201229/src/grp_fs/include\
	$(ROOT_PROJ)/src/gr_file_v1.31-201229/src/grp_fs/lib/grp_char\
	$(ROOT_PROJ)/src/gr_file_v1.31-201229/src/grp_fs/lib/grp_mem\
	$(ROOT_PROJ)/src/gr_file_v1.31-201229/src/mdep_vos2.xx/include\
	$(ROOT_PROJ)/src/gr_usb_d/common\
	$(ROOT_PROJ)/src/gr_usb_d/peri/pcd\
	$(ROOT_PROJ)/src/gr_usb_d/peri/perid\
	$(ROOT_PROJ)/src/gr_usb_h/hcm/bus\
	$(ROOT_PROJ)/src/gr_usb_h/hcm/cmem\
	$(ROOT_PROJ)/src/gr_usb_h/hcm/ctr\
	$(ROOT_PROJ)/src/gr_usb_h/hcm/hcd\
	$(ROOT_PROJ)/src/gr_usb_h/usbm/cnf_sft\
	$(ROOT_PROJ)/src/gr_usb_h/usbm/common\
	$(ROOT_PROJ)/src/gr_usb_h/usbm/usbd\
	$(ROOT_PROJ)/src/gr_usb_hdcm\
	$(ROOT_PROJ)/src/gr_vos\
	$(ROOT_PROJ)/src/ms_c/msctest\
	$(ROOT_PROJ)/src/ms_c/src/atapi\
	$(ROOT_PROJ)/src/ms_c/src/bot\
	$(ROOT_PROJ)/src/ms_c/src/cbi\
	$(ROOT_PROJ)/src/ms_c/src/com\
	$(ROOT_PROJ)/src/ms_c/src/scsi\
	$(ROOT_PROJ)/src/ms_c/src/sff8070i\
	$(ROOT_PROJ)/src/ms_c/src/ufi\
	

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


