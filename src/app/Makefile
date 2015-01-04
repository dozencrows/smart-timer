# This makefile reuses the serial one, but runs it in a sibling directory
include ../common/rules2.mk

firmware.elf: main.o timer_controller.o timer.o buzzer.o lcd.o timers.o gcc_startup_lpc8xx.o
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) $^

ifneq ($(SERIAL),)
firmware.elf: printf-stdarg.o
endif

