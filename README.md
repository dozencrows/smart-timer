smart-timer
===========
This is a test project for exploration of the NXP LPC810 ARM microcontroller.

It implements a dual countdown timer, capable of up to ten hour countdowns on each timer independently.

There is accompanying hardware that the code requires:
* MCP23008 I2C GPIO expander to interface 8 momentary switches to act as input
* 2N2222 transistor switching a piezo self-oscillating sounder
* MIC2545A smart switch controlling LCD power
* 16 x 2 character LCD display (hd44780 compatible)
* PCF8574 I2C GPIO expander to interface LCD display
* 3.3V power supply
