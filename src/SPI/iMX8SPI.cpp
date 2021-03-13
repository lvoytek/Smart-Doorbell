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
 * iMX8SPI
 *
 * This module is an SPI protocol driver for i.MX 8 boards
 */

#include "iMX8SPI.h"
#include "iMX8.h"

#include "fsl_dspi.h"

#define TRANSFER_BAUDRATE 500000U

void iMX8SPI::init(PIN csPin, unsigned int frequency, int settings)
{
	dspi_master_config_t masterConfig;

	masterConfig.whichCtar								  = kDSPI_Ctar0;
	masterConfig.ctarConfig.baudRate					  = TRANSFER_BAUDRATE;
	masterConfig.ctarConfig.bitsPerFrame				  = 8U;
	masterConfig.ctarConfig.cpol						  = kDSPI_ClockPolarityActiveHigh;
	masterConfig.ctarConfig.cpha						  = kDSPI_ClockPhaseFirstEdge;
	masterConfig.ctarConfig.direction					  = kDSPI_MsbFirst;
	masterConfig.ctarConfig.pcsToSckDelayInNanoSec		  = 1000000000U / TRANSFER_BAUDRATE;
	masterConfig.ctarConfig.lastSckToPcsDelayInNanoSec	  = 1000000000U / TRANSFER_BAUDRATE;
	masterConfig.ctarConfig.betweenTransferDelayInNanoSec = 1000000000U / TRANSFER_BAUDRATE;

	masterConfig.whichPcs			= kDSPI_Pcs0;
	masterConfig.pcsActiveHighOrLow = kDSPI_PcsActiveLow;

	masterConfig.enableContinuousSCK		= false;
	masterConfig.enableRxFifoOverWrite		= false;
	masterConfig.enableModifiedTimingFormat = false;
	masterConfig.samplePoint				= kDSPI_SckToSin0Clock;

	DSPI_MasterInit(SPI2, &masterConfig, frequency);
}

char iMX8SPI::spiTransfer(char toSend)
{
	char received;
	DSPI_MasterTransferNonBlocking(SPI2, &toSend, &received);
	return received;
}

short iMX8SPI::spiTransfer16(short toSend)
{
	short rec;
	rec = this->spiTransfer((toSend & 0xFF00) >> 8);	// send data MSB first
	rec = (rec << 8) | this->spiTransfer(toSend & 0xFF);
	return rec;
}

void iMX8SPI::csHigh() { this->gpioDriver.digitalWrite(this->csPin, GPIO_HIGH); }

void iMX8SPI::csLow() { this->gpioDriver.digitalWrite(this->csPin, GPIO_LOW); }