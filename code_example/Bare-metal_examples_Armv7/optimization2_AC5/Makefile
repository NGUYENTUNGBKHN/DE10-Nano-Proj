# optimization2_AC5 Example Makefile
#
# Copyright (c) 2011 Arm Limited (or its affiliates). All rights reserved.
# Use, modification and redistribution of this file is subject to your possession of a
# valid End User License Agreement for the Arm Product of which these examples are part of 
# and your compliance with all applicable terms and conditions of such licence agreement.
#
# This makefile is intended for use with GNU make
# This example is intended to be built with Arm Compiler 5

TARGET=optimization2_AC5.axf

CC=armcc
AS=armasm
LD=armlink
AR=armar
FE=fromelf

# Select build rules based on Windows or Unix
ifdef WINDIR
DONE=@if exist $(1) echo Build completed.
RM=if exist $(1) del /q $(1)
SHELL=$(WINDIR)\system32\cmd.exe
else
ifdef windir
DONE=@if exist $(1) echo Build completed.
RM=if exist $(1) del /q $(1)
SHELL=$(windir)\system32\cmd.exe
else
DONE=@if [ -f $(1) ]; then echo Build completed.; fi
RM=rm -f $(1)
endif
endif


all: $(TARGET)
	$(call DONE,$(TARGET))

rebuild: clean all

clean:
	$(call RM,*.o)
	$(call RM,feedback.txt)
	$(call RM,$(TARGET))


# Linker feedback requires the project to be built at least twice to make use of the latest feedback.txt
# Normally, --feedback=feedback.txt just needs to be added to your existing single compile and link steps in your makefile.
# In this makefile, for convenience, an second compile and link iteration is deliberately done, to save having to invoke make twice

# Link with specific base address --ro-base=0x80000000 to suit BeagleBoard memory layout
# BeagleBoard has 256MB DDR SDRAM in its POP device at 0x80000000 to 0x8FFFFFFF
# Alternatively, OMAP3530 has 64KB internal SRAM, from 0x40200000 to 0x4020FFFF

$(TARGET): testmain.c testsub.c
# Compile first time
	$(CC) -c -g --cpu=Cortex-A8 -O3 -Otime --c99 testmain.c testsub.c --multifile --vectorize
# Link first time, to create a feedback file
	$(LD) testmain.o testsub.o -o $(TARGET) --ro-base=0x80000000 --entry=StartHere --feedback=feedback.txt
# Compile second time, using the feedback file
	$(CC) -c -g --cpu=Cortex-A8 -O3 -Otime --c99 testmain.c testsub.c --feedback=feedback.txt --multifile --vectorize
# Link second time, to create an updated feedback file for subsequent iterations...
	$(LD) testmain.o testsub.o -o $(TARGET) --ro-base=0x80000000 --entry=StartHere --info=totals,unused,summarystack --feedback=feedback.txt
