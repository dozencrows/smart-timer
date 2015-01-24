# This makefile reuses the serial one, but runs it in a sibling directory
include ../common/rules2.mk

CFLAGS += -DFIXED_CLOCK_RATE_HZ=12000000 -DFIXED_UART_BAUD_RATE=115200

firmware.elf: main.o timer_controller.o button_input.o timer.o buzzer.o backlight.o lcd.o timers.o mrt_interrupt.o mcp.o gcc_startup_lpc8xx.o
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) $^

