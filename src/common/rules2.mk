# Minimal gcc makefile for LPC810 with serial output and printf

# default Linux USB device name for upload
TTY ?= /dev/ttyUSB*

# default is to include the serial output code and connect after upload
SERIAL ?= -term

# use the arm cross compiler, not std gcc
TRGT = /home/ntuckett/NXP-LPC/gcc-arm-none-eabi-4_9-2014q4/bin/arm-none-eabi-
CC = $(TRGT)gcc
CXX = $(TRGT)g++
CP = $(TRGT)objcopy

# compiler and linker settings
CFLAGS = -mcpu=cortex-m0plus -mthumb -I../common -I.. -Os -ggdb
CXXFLAGS = $(CFLAGS) -fno-rtti -fno-exceptions
LDFLAGS = -Wl,--script=../common/LPC810.ld -nostartfiles -Wl,-Map=firmware.map

# permit including this makefile from a sibling directory
vpath %.c ../common
vpath %.cpp ../common
vpath %.c ../util
vpath %.cpp ../util

# default target
#upload: firmware.bin
#	/home/ntuckett/NXP-LPC/lpc21isp_197/lpc21isp $(SERIAL) -control -donotstart -bin $< $(TTY) 115200 0
all: firmware.bin

%.bin: %.elf
	$(CP) -O binary $< $@

clean:
	rm -f *.o *.elf

# these target names don't represent real files
.PHONY: clean
