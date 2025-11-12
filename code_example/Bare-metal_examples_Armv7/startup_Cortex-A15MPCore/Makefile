# Copyright (c) 2011-2018 Arm Limited (or its affiliates). All rights reserved.
# Use, modification and redistribution of this file is subject to your possession of a
# valid End User License Agreement for the Arm Product of which these examples are part of 
# and your compliance with all applicable terms and conditions of such licence agreement.
#
# This makefile is intended to allow command-line users to build this project outside of Eclipse
# This makefile is NOT used by the Eclipse managed builder project (which creates its own makefiles)
#
# This makefile is intended for use with GNU make
# This example is intended to be built with Arm Compiler 5


TARGET=startup_Cortex-A15MPCore.axf

CPU=Cortex-A15

CC=armcc
AS=armasm
LD=armlink
AR=armar
FE=fromelf

CFLAGS=-g --cpu=$(CPU) -O1
AFLAGS=-g --cpu=$(CPU) --apcs=/interwork

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
	$(call RM,$(TARGET))

main.o: main.c v7.h MP_VGIC.h MP_Mutexes.h
	$(CC) -c $(CFLAGS) main.c

sorts.o: sorts.c v7.h
	$(CC) -c $(CFLAGS) sorts.c

startup.o: startup.s
	$(AS) $(AFLAGS) startup.s

MP_VGIC.o: MP_VGIC.s
	$(AS) $(AFLAGS) MP_VGIC.s

v7.o: v7.s
	$(AS) $(AFLAGS) v7.s

MP_Mutexes.o: MP_Mutexes.s
	$(AS) $(AFLAGS) MP_Mutexes.s

$(TARGET): MP_VGIC.o v7.o MP_Mutexes.o main.o sorts.o startup.o scatter.scat
	$(LD) MP_VGIC.o v7.o MP_Mutexes.o main.o sorts.o startup.o -o $(TARGET) --entry=Vectors --cpu=$(CPU) --scatter=scatter.scat
