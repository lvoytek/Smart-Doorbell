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
 * Button
 *
 * This module controls debounce GPIO button inputs from a given pin
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "GPIODriver.h"
#include "Timer.h"

#include "Debug.h"

#include "Button.h"

/**
 * Initialize a button for reading
 * @param button_pin The GPIO pin number that the button is attached to
 */
void Button_init(PIN button_pin)
{
	GPIO_init(button_pin);
	GPIO_pin_mode(button_pin, PIN_MODE_INPUT);
	DEBUG_PRINTLN("Initialized button on GPIO %d", button_pin);
}

/**
 * Wait for a button press then return
 * @param button_pin The GPIO pin number that the button is attached to
 */
void Button_wait_for_press(PIN button_pin, unsigned int post_press_pause_time_ms)
{
	// Open pin value file
	int pin_value_file;
	int err = GPIO_get_pin_value_file_pointer(button_pin, &pin_value_file);

	if(err < 0)
	{
		ERROR_PRINTLN("GPIO pin value file failed to open, cannot wait for button press");
		return;
	}

	char value_buffer[8];
	int	 current_level = -1;

	// Get the initial input value
	lseek(pin_value_file, 0, SEEK_SET);
	err = read(pin_value_file, value_buffer, sizeof(value_buffer));

	if(err < 0) { ERROR_PRINTLN("Unable to read value file"); }
	else
	{
		current_level = value_buffer[0] - '0';

		if(current_level == 0 || current_level == 1)
		{
			DEBUG_PRINTLN("Waiting for button press, current state is %d", current_level);
		}
		else
		{
			ERROR_PRINTLN("Initial pin value of %d is invalid", current_level);
		}
	}

	while(1)
	{
		// Check for change in value
		lseek(pin_value_file, 0, SEEK_SET);
		err = read(pin_value_file, value_buffer, sizeof(value_buffer));

		if(err < 0) { ERROR_PRINTLN("Unable to read value file"); }
		else
		{
			int new_level = value_buffer[0] - '0';

			if(new_level != current_level)
			{
				DEBUG_PRINTLN("Button Pressed, changed to %d", new_level);
				break;
			}
		}

		Timer_delay_ms(20);
	}

	close(pin_value_file);

	Timer_delay_ms(post_press_pause_time_ms);
}