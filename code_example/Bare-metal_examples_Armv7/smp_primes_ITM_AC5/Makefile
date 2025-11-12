# Primes Example Makefile
#
# Copyright (c) 2011-2018 Arm Limited (or its affiliates). All rights reserved.
# Use, modification and redistribution of this file is subject to your possession of a
# valid End User License Agreement for the Arm Product of which these examples are part of 
# and your compliance with all applicable terms and conditions of such licence agreement.
#
# This makefile is intended for use with GNU make
# This example is intended to be built with Arm Compiler 5

TARGET=primes.axf

CPU=Cortex-A9

CC=armcc
AS=armasm
LD=armlink
AR=armar
FE=fromelf

# Note: "--arm_only" option needed to workaround issue with debugger
# across instruction set state change in Versatile Express
CFLAGS=-g --cpu=$(CPU) -O1 --arm_only
AFLAGS=-g --cpu=$(CPU)
OBJS=$(patsubst %.c,%.o,$(wildcard *.c))
OBJASM=$(patsubst %.s,%.o,$(wildcard *.s))

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

$(OBJS): %.o: %.c 
	$(CC) -c $(CFLAGS) $< -o $@

$(OBJASM): %.o: %.s 
	$(AS) $(AFLAGS) $< -o $@

evITM.o: ./event_viewer/evITM.h ./event_viewer/evMsg.h ./event_viewer/types.h ./event_viewer/VEA9_itm.h
evMsg.o: ./event_viewer/evMsg.h ./event_viewer/types.h
evprintf.o: ./event_viewer/evMsg.h ./event_viewer/types.h
main.o: v7.h MP_GIC.h MP_Mutexes.h primes.h ./event_viewer/evITM.h ./event_viewer/evMsg.h ./event_viewer/types.h ./event_viewer/evprintf.h
primes.o: MP_Mutexes.h ./event_viewer/evITM.h ./event_viewer/evMsg.h ./event_viewer/types.h ./event_viewer/evprintf.h

$(TARGET): $(OBJS) $(OBJASM) scatter.scat
	$(LD) --entry=Vectors -o $(TARGET) --scatter=scatter.scat --via=keep.txt $(OBJS) $(OBJASM)
