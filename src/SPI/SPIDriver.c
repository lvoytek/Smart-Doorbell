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
 * SPIDriver
 *
 * This module acts as a parent class for drivers that control the SPI protocol on
 * specific boards
 */

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#include "Debug.h"

#include "SPIDriver.h"

static int spi_file;
char	   spi_filename[20];

char tx_buf[256];
char rx_buf[256];

struct spi_ioc_transfer xfer;

/**
 * Initialize SPI bus of given number for transactions with a slave device at a given frequency
 * @param spi_bus The SPI bus number
 * @param frequency The clock frequency in Hz
 */
void SPI_init(unsigned int spi_bus, unsigned int spi_cs, unsigned int frequency)
{
	__u8 bits_per_word = 8;
	__u8 mode		   = SPI_MODE_0;

	snprintf(spi_filename, 19, "/dev/spidev%u.%u", spi_bus, spi_cs);
	spi_file = open(spi_filename, O_RDWR);

	if(spi_file < 0)
	{
		ERROR_PRINTLN("%19s does not exist.", spi_filename);
		return;
	}

	if(ioctl(spi_file, SPI_IOC_WR_MODE, &mode) < 0) { ERROR_PRINTLN("Cannot set mode %uh", mode); }

	if(ioctl(spi_file, SPI_IOC_WR_BITS_PER_WORD, &bits_per_word) < 0)
	{
		ERROR_PRINTLN("Cannot set %hu bits per word.", bits_per_word);
	}

	if(ioctl(spi_file, SPI_IOC_WR_MAX_SPEED_HZ, &frequency) < 0)
	{
		ERROR_PRINTLN("Cannot set max SPI speed of %u.", frequency);
	}

	xfer.tx_buf		   = (unsigned long) tx_buf;
	xfer.rx_buf		   = (unsigned long) rx_buf;
	xfer.delay_usecs   = 0;
	xfer.speed_hz	   = frequency;
	xfer.bits_per_word = bits_per_word;
}

/**
 * Close SPI bus and cleanup
 */
void SPI_shutdown()
{
	if(close(spi_file) < 0) { ERROR_PRINTLN("SPI Bus close failure"); }
}

unsigned char SPI_transfer(unsigned char toSend)
{
	if(spi_file < 0)
	{
		ERROR_PRINTLN("SPI unavailable");
		return 0;
	}

	tx_buf[0] = toSend;
	rx_buf[0] = 0;
	xfer.len  = 1;

	if(ioctl(spi_file, SPI_IOC_MESSAGE(1), &xfer) < 0)
	{
		ERROR_PRINTLN("SPI single byte transfer failed");
		return 0;
	}

	return rx_buf[0];
}

unsigned short SPI_transfer16(unsigned short toSend)
{
	if(spi_file < 0)
	{
		ERROR_PRINTLN("SPI unavailable");
		return 0;
	}

	tx_buf[0] = (unsigned char) ((toSend >> 8) & 0xFF);
	tx_buf[1] = (unsigned char) (toSend & 0xFF);

	rx_buf[0] = 0;
	rx_buf[1] = 0;

	xfer.len = 2;

	if(ioctl(spi_file, SPI_IOC_MESSAGE(2), &xfer) < 0)
	{
		ERROR_PRINTLN("SPI 2 byte transfer failed");
		return 0;
	}

	unsigned short output = (((unsigned short) rx_buf[0]) << 8) | rx_buf[1];
	return output;
}