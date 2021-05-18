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
 * I2CDriver
 *
 * This module acts as a parent class for drivers that control the I2C protocol on
 * specific boards
 */

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>

#include "Debug.h"

#include "I2CDriver.h"

static int i2c_file;
char	   i2c_filename[20];

/**
 * Initialize I2C bus of given number for transactions with a slave device with a given address
 * @param i2c_bus The I2C bus number
 * @param address The slave device address
 */
void I2C_init(int i2c_bus, unsigned char address)
{
	snprintf(i2c_filename, 19, "/dev/i2c-%d", i2c_bus);
	i2c_file = open(i2c_filename, O_RDWR);

	if(i2c_file < 0)
	{
		ERROR_PRINTLN("%19s does not exist.", i2c_filename);
		return;
	}

	if(ioctl(i2c_file, I2C_SLAVE, address) < 0)
	{
		ERROR_PRINTLN("Cannot change I2C slave address.");
	}
}

/**
 * Close I2C bus and cleanup
 */
void I2C_shutdown()
{
	if(close(i2c_file) < 0) { ERROR_PRINTLN("I2C Bus close failure"); }
}

/**
 * Write a byte of data to a register with an 8-bit address
 * @param data The byte of data to write
 */
void I2C_write(unsigned char reg, unsigned char data)
{
	if(i2c_file < 0)
	{
		ERROR_PRINTLN("I2C unavailable");
		return;
	}

	int err = i2c_smbus_write_byte_data(i2c_file, reg, data);
	if(err < 0)
	{
		ERROR_PRINTLN("I2C Write Failed: Reg - 0x%hx, Data - 0x%hx, return %d", reg, data, err);
	}
}

/**
 * Write data to a register with a 16-bit address
 * @param reg The 16-bit register address
 * @param data The byte of data to write
 */
void I2C_write16(unsigned short reg, unsigned char data)
{
	if(i2c_file < 0)
	{
		ERROR_PRINTLN("I2C unavailable");
		return;
	}

	unsigned char smbus_command = (reg >> 8) & 0xFF;
	unsigned char smbus_data[2];
	smbus_data[0] = reg & 0xFF;
	smbus_data[1] = data;

	int err = i2c_smbus_write_block_data(i2c_file, smbus_command, 2, smbus_data);
	if(err < 0)
	{
		ERROR_PRINTLN("I2C Write Failed: Reg - 0x%hx, Data - 0x%hx, return %d", reg, data, err);
	}
}

/**
 * Read a byte of data from a register with an 8-bit address
 * @param reg The 8-bit register address
 * @return The byte that was read
 */
unsigned char I2C_read(unsigned char reg)
{
	if(i2c_file < 0)
	{
		ERROR_PRINTLN("I2C unavailable");
		return 0;
	}

	int err = i2c_smbus_write_byte(i2c_file, reg);
	if(err < 0)
	{
		ERROR_PRINTLN("I2C Prep for Read Address Write Failed: 0x%hx, return %d", reg, err);
	}

	int read_val = i2c_smbus_read_byte(i2c_file);

	if(read_val < 0)
	{
		ERROR_PRINTLN("I2C Read failed: return %d", read_val);
		return 0;
	}

	return read_val;
}

/**
 * Read a byte of data from a register with a 16-bit address
 * @param reg The 16-bit register address
 * @return The byte that was read
 */
unsigned char I2C_read16(unsigned short reg)
{
	if(i2c_file < 0)
	{
		ERROR_PRINTLN("I2C unavailable");
		return 0;
	}

	int err = i2c_smbus_write_byte_data(i2c_file, (reg >> 8) & 0xFF, reg & 0xFF);
	if(err < 0)
	{
		ERROR_PRINTLN("I2C Prep for Read Address Write Failed: 0x%hx, return %d", reg, err);
	}

	int read_val = i2c_smbus_read_byte(i2c_file);

	if(read_val < 0)
	{
		ERROR_PRINTLN("I2C Read failed: return %d", read_val);
		return 0;
	}

	return read_val;
}