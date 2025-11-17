NPROC ?= $(shell nproc)
ifeq ($(NPROC),)
NPROC ?= $(shell getconf _NPROCESSORS_ONLN)
endif
NPROC ?= 4
MAKEFLAGS += -j$(NPROC)

SUBDIRS := fatfs gr_usb hwlib JSWare threadx threadx_drv threadx_uitron

.PHONY: default all clean build_fatfs build_gr_usb build_hwlib

default: build_gr_usb build_hwlib build_fatfs

build_fatfs:
	$(MAKE) -C fatfs

build_gr_usb:
	$(MAKE) -C gr_usb

build_hwlib:
	$(MAKE) -C hwlib


all:
	for d in $(SUBDIRS); do \
		"C:/Program Files/Arm/Development Studio 2020.1/bin/make" -C $$d; \
	done


clean_fatfs:
	$(MAKE) -C fatfs clean

clean_gr_usb:
	$(MAKE) -C gr_usb clean

clean_hwlib:
	$(MAKE) -C hwlib clean

clean_JSWare:
	$(MAKE) -C JSWare clean

clean_threadx:
	$(MAKE) -C threadx clean

clean_threadx_drv:
	$(MAKE) -C threadx_drv clean

clean_threadx_uitron:
	$(MAKE) -C threadx_uitron clean
