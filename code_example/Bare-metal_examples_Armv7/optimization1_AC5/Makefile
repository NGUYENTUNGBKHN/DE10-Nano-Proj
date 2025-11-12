# optimization1_AC5 Example Makefile
#
# Copyright (c) 2011-2017 Arm Limited (or its affiliates). All rights reserved.
# Use, modification and redistribution of this file is subject to your possession of a
# valid End User License Agreement for the Arm Product of which these examples are part of 
# and your compliance with all applicable terms and conditions of such licence agreement.
#
# This makefile is intended to allow command-line users to build this project outside of Eclipse
# This makefile is NOT used by the Eclipse managed builder project (which creates its own makefiles)
#
# This makefile is intended for use with GNU make
# This example is intended to be built with Arm Compiler 5


TARGET=optimization1_AC5.axf

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
	$(call RM,$(TARGET))

sorts.o: sorts.c
	$(CC) -c -g --cpu=Cortex-A8 -O3 -Otime --arm sorts.c --info=totals

perf.o: perf.c v7_pmu.h
	$(CC) -c -g --cpu=Cortex-A8 -O3 -Otime --arm perf.c

startup.o: startup.s
	$(AS) -g --cpu=Cortex-A8 --apcs=/interwork startup.s

v7_pmu.o: v7_pmu.s
	$(AS) -g --cpu=Cortex-A8 --apcs=/interwork v7_pmu.s

$(TARGET): startup.o v7_pmu.o perf.o sorts.o scatter.scat
	$(LD) startup.o v7_pmu.o perf.o sorts.o --scatter=scatter.scat -o $(TARGET) --entry=StartHere --info=sizes,totals,unused,stack,compression,libraries --list=list.txt --map
	$(FE) -z $(TARGET)
