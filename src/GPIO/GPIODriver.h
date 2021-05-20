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

#ifndef GPIODRIVER_H
#define GPIODRIVER_H

typedef enum
{
	GPIO_LOW  = 0,
	GPIO_HIGH = 1,
	GPIO_LEVEL_INVALID
} GPIO_LEVEL;

typedef enum
{
	PIN_MODE_INPUT,
	PIN_MODE_OUTPUT,
	PIN_MODE_INVALID
} PIN_MODE;

typedef int PIN;

void	   GPIO_init(PIN pin);
void	   GPIO_pin_mode(PIN pin, PIN_MODE mode);
void	   GPIO_digital_write(PIN pin, GPIO_LEVEL val);
GPIO_LEVEL GPIO_digital_read(PIN pin);
int		   GPIO_get_pin_value_file_pointer(PIN pin, int * fp);

#endif