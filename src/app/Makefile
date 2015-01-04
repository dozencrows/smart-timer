# This makefile reuses the serial one, but runs it in a sibling directory
include ../common/rules2.mk

firmware.elf: main.o timer_controller.o timer.o buzzer.o backlight.o lcd.o timers.o mrt_interrupt.o gcc_startup_lpc8xx.o
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) $^

ifneq ($(SERIAL),)
firmware.elf: printf-stdarg.o
endif

