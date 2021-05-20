/*
 * MIT License
 *
 * Copyright (c) 2021 Lena Voytek
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * GPIODriver
 *
 * This module acts as a parent class for drivers that control GPIO signals on
 * specific boards.
 */

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "Debug.h"

#include "GPIODriver.h"

#define GPIO_EXPORT_DIRECTORY "/sys/class/gpio/export"
#define GPIO_DIRECTORY_PREFIX "/sys/class/gpio/gpio"

/**
 * Initialize a pin in the Linux filesystem
 * @param pin The GPIO pin number
 */
void GPIO_init(PIN pin)
{
	char gpio_pin_string[5];
	snprintf(gpio_pin_string, 5, "%d", pin);

	int exportFile = open(GPIO_EXPORT_DIRECTORY, O_WRONLY | O_SYNC);
	if(exportFile < 0)
	{
		ERROR_PRINTLN("Unable to open GPIO export file");
		return;
	}

	int err = write(exportFile, gpio_pin_string, strlen(gpio_pin_string));
	if(err < 0) { ERROR_PRINTLN("GPIO export failed: return %d", errno); }

	close(exportFile);
}

/**
 * Set the GPIO mode to input or output
 * @param pin the GPIO pin number
 * @param mode the input or output mode
 */
void GPIO_pin_mode(PIN pin, PIN_MODE mode)
{
	char gpio_mode_filename[35];
	snprintf(gpio_mode_filename, 35, "%s%d/direction", GPIO_DIRECTORY_PREFIX, pin);

	int gpio_mode_file = open(gpio_mode_filename, O_WRONLY | O_SYNC);
	if(gpio_mode_file < 0)
	{
		ERROR_PRINTLN("Unable to open pinmode set file");
		return;
	}

	int err = 0;

	switch(mode)
	{
		case PIN_MODE_INPUT:
			err = write(gpio_mode_file, "in", 2);
			break;
		case PIN_MODE_OUTPUT:
			err = write(gpio_mode_file, "out", 3);
			break;
		default:
			ERROR_PRINTLN("Pin mode not implemented");
			break;
	}

	if(err < 0) { ERROR_PRINTLN("Unable to set pinmode"); }

	close(gpio_mode_file);
}

/**
 * Set a GPIO high or low
 * @param pin the gpio pin number
 * @param val the value, high or low
 */
void GPIO_digital_write(PIN pin, GPIO_LEVEL val)
{
	char gpio_value_filename[35];
	snprintf(gpio_value_filename, 34, "%s%d/value", GPIO_DIRECTORY_PREFIX, pin);

	int gpio_value_file = open(gpio_value_filename, O_WRONLY | O_SYNC);
	if(gpio_value_file < 0)
	{
		ERROR_PRINTLN("Unable to open GPIO value set file");
		return;
	}

	char gpio_val_string[2];
	snprintf(gpio_val_string, 2, "%d", val);

	int err = write(gpio_value_file, gpio_val_string, 1);
	if(err < 0) { ERROR_PRINTLN("GPIO val write failed: return %d", errno); }

	close(gpio_value_file);
}

/**
 * Read high or low from a GPIO
 * @param pin the gpio pin number
 * @return high or low
 */
GPIO_LEVEL GPIO_digital_read(PIN pin)
{
	char gpio_value_filename[35];
	snprintf(gpio_value_filename, 34, "%s%d/value", GPIO_DIRECTORY_PREFIX, pin);

	int gpio_value_file = open(gpio_value_filename, O_RDONLY);
	if(gpio_value_file < 0)
	{
		ERROR_PRINTLN("Unable to open GPIO value get file");
		return GPIO_LEVEL_INVALID;
	}

	char output_val;

	int err = read(gpio_value_file, &output_val, 1);
	if(err < 0)
	{
		ERROR_PRINTLN("GPIO val read failed: return %d", errno);
		return GPIO_LEVEL_INVALID;
	}

	close(gpio_value_file);

	switch(output_val)
	{
		case '0':
			return GPIO_LOW;
		case '1':
			return GPIO_HIGH;
		default:
			ERROR_PRINTLN("GPIO value %c does not exist", output_val);
			return GPIO_LEVEL_INVALID;
	}
}