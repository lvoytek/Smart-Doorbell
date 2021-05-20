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
 * Camera
 *
 * This module controls the setup and intake of live video from the ArduCam 5MP
 * Plus OV5642 Camera
 */

#include <stdio.h>

#include "Debug.h"
#include "I2CDriver.h"
#include "SPIDriver.h"
#include "Timer.h"

#include "Camera.h"
#include "ArduCAM.h"
#include "ov5642_regs.h"

const unsigned char camera_i2c_address = 0x3C;

void		  clearFIFOFlag();
unsigned char readFIFO();
void		  flushFIFO();
unsigned int  readFIFOLength();
void		  setFIFOBurst();

unsigned char readRegister(unsigned char address);
void		  writeRegister(unsigned char address, unsigned char data);

void		  setBit(unsigned char address, unsigned char bit);
void		  clearBit(unsigned char address, unsigned char bit);
unsigned char getBit(unsigned char address, unsigned char bit);

void		  busWrite(int address, int value);
unsigned char busRead(int address);

void wrSensorReg8_8(int regID, int regDat);
void wrSensorRegs8_8(const struct sensor_reg * reglist);
void rdSensorReg8_8(unsigned char regID, unsigned char * regDat);

void wrSensorReg16_8(int regID, int regDat);
void wrSensorRegs16_8(const struct sensor_reg reglist[]);
void rdSensorReg16_8(unsigned int regID, unsigned char * regDat);
void rdSensorRegs16_8(struct sensor_reg reglist[]);

static IMAGE_TYPE format;
char			  read_buffer[JPEG_BUFFER_SIZE];
char			  command_buffer[CMD_BUFFER_SIZE];
char *			  send_buffer;

int current_jpeg_buffer_size;

/**
 * Initialize Camera variables and communication busses
 * @param i2c_bus The I2C bus number
 * @param spi_bus The SPI bus number
 */
void Camera_init(int i2c_bus, unsigned int spi_bus, unsigned int spi_cs)
{
	format = IMG_JPEG;

	I2C_init(i2c_bus, camera_i2c_address);
	SPI_init(spi_bus, spi_cs, 8000000);

	// Check for Camera until SPI exists
	while(1)
	{
		writeRegister(ARDUCHIP_TEST1, 0x55);
		unsigned char temp = readRegister(ARDUCHIP_TEST1);

		if(temp != 0x55)
		{
			ERROR_PRINTLN("Camera SPI unavailable.");
			Timer_delay_ms(1000);
		}
		else
		{
			DEBUG_PRINTLN("Camera SPI online.");
			break;
		}
	}

	unsigned char pid = 0, vid = 0;

	// Check for camera I2C until it exists
	while(1)
	{
		rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid);
		rdSensorReg16_8(OV5642_CHIPID_LOW, &pid);

		if(vid != 0x56 || pid != 0x42)
		{
			ERROR_PRINTLN("Camera I2C unavailable: vid = 0x%x, pid = 0x%x", vid, pid);
			Timer_delay_ms(1000);
		}
		else
		{
			DEBUG_PRINTLN("Camera I2C online.");
			break;
		}
	}

	wrSensorReg16_8(0x3008, 0x80);
	wrSensorRegs16_8(OV5642_QVGA_Preview);

	if(format == IMG_JPEG)
	{
		DEBUG_PRINTLN("Initializing JPEG Format");
		wrSensorRegs16_8(OV5642_JPEG_Capture_QSXGA);
		wrSensorRegs16_8(ov5642_320x240);
		wrSensorReg16_8(0x3818, 0xa8);
		wrSensorReg16_8(0x3621, 0x10);
		wrSensorReg16_8(0x3801, 0xb0);
		wrSensorReg16_8(0x4407, 0x04);
	}
	else
	{
		DEBUG_PRINTLN("Initializing BMP Format");
		unsigned char reg_val;
		wrSensorReg16_8(0x4740, 0x21);
		wrSensorReg16_8(0x501e, 0x2a);
		wrSensorReg16_8(0x5002, 0xf8);
		wrSensorReg16_8(0x501f, 0x01);
		wrSensorReg16_8(0x4300, 0x61);
		rdSensorReg16_8(0x3818, &reg_val);
		wrSensorReg16_8(0x3818, (reg_val | 0x60) & 0xff);
		rdSensorReg16_8(0x3621, &reg_val);
		wrSensorReg16_8(0x3621, reg_val & 0xdf);
	}

	Camera_set_resolution(RES_320x240);
}

/**
 * Shutdown camera and close communication
 */
void Camera_shutdown()
{
	DEBUG_PRINTLN("Shutting down camera");
	I2C_shutdown();
	SPI_shutdown();
}

/**
 * Set the image format to output
 * @param img_format The image format (JPEG or BMP)
 */
void Camera_set_image_format(IMAGE_TYPE img_format) { format = img_format; }

/**
 * Update the camera's output image resolution
 * @param res The resolution option Width x Height
 */
void Camera_set_resolution(RESOLUTION res)
{
	switch(res)
	{
		case RES_320x240:
			DEBUG_PRINTLN("Setting resolution to 320x240");
			wrSensorRegs16_8(ov5642_320x240);
			break;
		case RES_640x480:
			DEBUG_PRINTLN("Setting resolution to 640x480");
			wrSensorRegs16_8(ov5642_640x480);
			break;
		case RES_1024x768:
			DEBUG_PRINTLN("Setting resolution to 1024x768");
			wrSensorRegs16_8(ov5642_1024x768);
			break;
		case RES_1280x960:
			DEBUG_PRINTLN("Setting resolution to 1280x960");
			wrSensorRegs16_8(ov5642_1280x960);
			break;
		case RES_1600x1200:
			DEBUG_PRINTLN("Setting resolution to 1600x1200");
			wrSensorRegs16_8(ov5642_1600x1200);
			break;
		case RES_2048x1536:
			DEBUG_PRINTLN("Setting resolution to 2048x1536");
			wrSensorRegs16_8(ov5642_2048x1536);
			break;
		case RES_2592x1944:
			DEBUG_PRINTLN("Setting resolution to 2592x1944");
			wrSensorRegs16_8(ov5642_2592x1944);
			break;
		default:
			break;
	}
}

/**
 * Update the camera's color saturation level
 * @param sat the saturation level
 */
void Camera_set_color_saturation(COLOR_SATURATION sat)
{
	wrSensorReg16_8(0x5001, 0xff);

	switch(sat)
	{
		case SAT_4:
			wrSensorReg16_8(0x5583, 0x80);
			wrSensorReg16_8(0x5584, 0x80);
			break;
		case SAT_3:
			wrSensorReg16_8(0x5583, 0x70);
			wrSensorReg16_8(0x5584, 0x70);
			break;
		case SAT_2:
			wrSensorReg16_8(0x5583, 0x60);
			wrSensorReg16_8(0x5584, 0x60);
			break;
		case SAT_1:
			wrSensorReg16_8(0x5583, 0x50);
			wrSensorReg16_8(0x5584, 0x50);
			break;
		case SAT_0:
			wrSensorReg16_8(0x5583, 0x40);
			wrSensorReg16_8(0x5584, 0x40);
			break;
		case SAT_NEG_1:
			wrSensorReg16_8(0x5583, 0x30);
			wrSensorReg16_8(0x5584, 0x30);
			break;
		case SAT_NEG_2:
			wrSensorReg16_8(0x5583, 0x20);
			wrSensorReg16_8(0x5584, 0x20);
			break;
		case SAT_NEG_3:
			wrSensorReg16_8(0x5583, 0x10);
			wrSensorReg16_8(0x5584, 0x10);
			break;
		case SAT_NEG_4:
			wrSensorReg16_8(0x5583, 0x00);
			wrSensorReg16_8(0x5584, 0x00);
			break;
		default:
			wrSensorReg16_8(0x5583, 0x40);
			wrSensorReg16_8(0x5584, 0x40);
			break;
	}

	wrSensorReg16_8(0x5580, 0x02);
}

/**
 * Set the camera's brightness offset
 * @param level the brightness level offset
 */
void Camera_set_brightness(BRIGHTNESS level)
{
	wrSensorReg16_8(0x5001, 0xff);

	switch(level)
	{
		case BRIGHTNESS_4:
		case BRIGHTNESS_NEG_4:
			wrSensorReg16_8(0x5589, 0x40);
			break;
		case BRIGHTNESS_3:
		case BRIGHTNESS_NEG_3:
			wrSensorReg16_8(0x5589, 0x30);
			break;
		case BRIGHTNESS_2:
		case BRIGHTNESS_NEG_2:
			wrSensorReg16_8(0x5589, 0x20);
			break;
		case BRIGHTNESS_1:
		case BRIGHTNESS_NEG_1:
			wrSensorReg16_8(0x5589, 0x10);
			break;
		case BRIGHTNESS_0:
		default:
			wrSensorReg16_8(0x5589, 0x00);
			break;
	}

	wrSensorReg16_8(0x5580, 0x04);

	switch(level)
	{
		case BRIGHTNESS_NEG_4:
		case BRIGHTNESS_NEG_3:
		case BRIGHTNESS_NEG_2:
		case BRIGHTNESS_NEG_1:
			wrSensorReg16_8(0x558a, 0x08);
			break;

		case BRIGHTNESS_4:
		case BRIGHTNESS_3:
		case BRIGHTNESS_2:
		case BRIGHTNESS_1:
		case BRIGHTNESS_0:
		default:
			wrSensorReg16_8(0x558a, 0x00);
			break;
	}
}

/**
 * Add a special effect to the camera output
 * @param effect the special effect to add
 */
void Camera_set_special_effect(SPECIAL_EFFECTS effect)
{
	switch(effect)
	{
		case EFFECT_BLUISH:
			wrSensorReg16_8(0x5001, 0xff);
			wrSensorReg16_8(0x5580, 0x18);
			wrSensorReg16_8(0x5585, 0xa0);
			wrSensorReg16_8(0x5586, 0x40);
			break;
		case EFFECT_GREENISH:
			wrSensorReg16_8(0x5001, 0xff);
			wrSensorReg16_8(0x5580, 0x18);
			wrSensorReg16_8(0x5585, 0x60);
			wrSensorReg16_8(0x5586, 0x60);
			break;
		case EFFECT_REDDISH:
			wrSensorReg16_8(0x5001, 0xff);
			wrSensorReg16_8(0x5580, 0x18);
			wrSensorReg16_8(0x5585, 0x80);
			wrSensorReg16_8(0x5586, 0xc0);
			break;
		case EFFECT_BW:
			wrSensorReg16_8(0x5001, 0xff);
			wrSensorReg16_8(0x5580, 0x18);
			wrSensorReg16_8(0x5585, 0x80);
			wrSensorReg16_8(0x5586, 0x80);
			break;
		case EFFECT_NEGATIVE:
			wrSensorReg16_8(0x5001, 0xff);
			wrSensorReg16_8(0x5580, 0x40);
			break;
		case EFFECT_SEPIA:
			wrSensorReg16_8(0x5001, 0xff);
			wrSensorReg16_8(0x5580, 0x18);
			wrSensorReg16_8(0x5585, 0x40);
			wrSensorReg16_8(0x5586, 0xa0);
			break;
		case EFFECT_NORMAL:
			wrSensorReg16_8(0x5001, 0x7f);
			wrSensorReg16_8(0x5580, 0x00);
			break;
	}
}

/**
 * Set the sharpness algorithm of the camera
 * @param sharpness the manual/automatic sharpness method
 */
void Camera_set_sharpness_type(SHARPNESS_TYPE sharpness)
{
	switch(sharpness)
	{
		case SHARP_AUTO_DEFAULT:
			wrSensorReg16_8(0x530A, 0x00);
			wrSensorReg16_8(0x530c, 0x0);
			wrSensorReg16_8(0x530d, 0xc);
			wrSensorReg16_8(0x5312, 0x40);
			break;
		case SHARP_AUTO_1:
			wrSensorReg16_8(0x530A, 0x00);
			wrSensorReg16_8(0x530c, 0x4);
			wrSensorReg16_8(0x530d, 0x18);
			wrSensorReg16_8(0x5312, 0x20);
			break;
		case SHARP_AUTO_2:
			wrSensorReg16_8(0x530A, 0x00);
			wrSensorReg16_8(0x530c, 0x8);
			wrSensorReg16_8(0x530d, 0x30);
			wrSensorReg16_8(0x5312, 0x10);
			break;
		case SHARP_MANUAL_OFF:
			wrSensorReg16_8(0x530A, 0x08);
			wrSensorReg16_8(0x531e, 0x00);
			wrSensorReg16_8(0x531f, 0x00);
			break;
		case SHARP_MANUAL_1:
			wrSensorReg16_8(0x530A, 0x08);
			wrSensorReg16_8(0x531e, 0x04);
			wrSensorReg16_8(0x531f, 0x04);
			break;
		case SHARP_MANUAL_2:
			wrSensorReg16_8(0x530A, 0x08);
			wrSensorReg16_8(0x531e, 0x08);
			wrSensorReg16_8(0x531f, 0x08);
			break;
		case SHARP_MANUAL_3:
			wrSensorReg16_8(0x530A, 0x08);
			wrSensorReg16_8(0x531e, 0x0c);
			wrSensorReg16_8(0x531f, 0x0c);
			break;
		case SHARP_MANUAL_4:
			wrSensorReg16_8(0x530A, 0x08);
			wrSensorReg16_8(0x531e, 0x0f);
			wrSensorReg16_8(0x531f, 0x0f);
			break;
		case SHARP_MANUAL_5:
			wrSensorReg16_8(0x530A, 0x08);
			wrSensorReg16_8(0x531e, 0x1f);
			wrSensorReg16_8(0x531f, 0x1f);
			break;
	}
}

/**
 * Reset camera settings to default
 */
void Camera_reset_firmware()
{
	writeRegister(0x07, 0x80);
	Timer_delay_ms(100);
	writeRegister(0x07, 0x00);
	Timer_delay_ms(100);
	writeRegister(ARDUCHIP_FRAMES, 0x00);
	setBit(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);
}

/**
 * Capture a single image into the read buffer
 */
void Camera_single_capture()
{
	flushFIFO();
	Camera_start_capture();

	while(!getBit(ARDUCHIP_TRIG, CAP_DONE_MASK)) { Timer_delay_us(5); }

	int count				 = readFIFOLength();
	current_jpeg_buffer_size = count;
	int i					 = 0;

	setFIFOBurst();

	while((count--) > 0) { read_buffer[i++] = SPI_transfer(0); }
	DEBUG_PRINTLN("Single image captured, size: %d bytes", current_jpeg_buffer_size);
}

/**
 * Save the most recent camera capture to a given file
 * @param filename the name of the file to save to
 */
void Camera_save_capture_to_file(const char * filename)
{
	FILE * output_file = fopen(filename, "w");

	if(output_file == NULL)
	{
		ERROR_PRINTLN("Failed to open %50s", filename);
		return;
	}

	fwrite(read_buffer, sizeof(char), current_jpeg_buffer_size, output_file);
	fflush(output_file);

	if(fclose(output_file) < 0) { ERROR_PRINTLN("Failed to close the image file"); }

	DEBUG_PRINTLN("Saved last capture to file: %50s", filename);
}

/**
 * Start capturing data on the camera
 */
void Camera_start_capture()
{
	DEBUG_PRINTLN("Starting image capture");
	writeRegister(ARDUCHIP_FIFO, FIFO_START_MASK);
}

/**
 * Clear the flag marking new data in the camera FIFO queue
 */
void clearFIFOFlag() { writeRegister(ARDUCHIP_FIFO, FIFO_CLEAR_MASK); }

/**
 * Read the next byte in the camera's FIFO queue
 * @return the next byte
 */
unsigned char readFIFO() { return busRead(SINGLE_FIFO_READ); }

/**
 * Flush out the current data in the camera's FIFO queue
 */
void flushFIFO()
{
	DEBUG_PRINTLN("Flushing FIFO");
	writeRegister(ARDUCHIP_FIFO, FIFO_CLEAR_MASK);
}

/**
 * Read the current size of the camera's SPI FIFO queue
 * @return the FIFO queue length
 */
unsigned int readFIFOLength()
{
	return (((readRegister(FIFO_SIZE3)) << 16) | ((readRegister(FIFO_SIZE2)) << 8) |
			readRegister(FIFO_SIZE1)) &
		   0x7fffff;
}

/**
 * Set the camera's SPI FIFO queue to burst mode
 */
void setFIFOBurst()
{
	DEBUG_PRINTLN("Starting a FIFO burst read");
	SPI_transfer(BURST_FIFO_READ);
}

/**
 * Read a byte from a camera register
 * @param address the register address
 * @return the data within the register
 */
unsigned char readRegister(unsigned char address) { return busRead(address & 0x7F); }

/**
 * Write a byte to a camera register
 * @param address the register address
 * @param data the data to put in the register
 */
void writeRegister(unsigned char address, unsigned char data) { busWrite(address | 0x80, data); }

/**
 * Set a single bit on a camera register to 1
 * @param address the register address
 * @param bit the bit mask
 */
void setBit(unsigned char address, unsigned char bit)
{
	unsigned char temp = readRegister(address);
	writeRegister(address, temp | bit);
}

/**
 * Set a single bit on a camera register to 0
 * @param address the register address
 * @param bit the bit mask
 */
void clearBit(unsigned char address, unsigned char bit)
{
	unsigned char temp = readRegister(address);
	writeRegister(address, temp & (~bit));
}

/**
 * Get a single bit from a camera register
 * @param address the address of the register
 * @param bit the bit mask for the bit to read
 * @return the bit in its bit location within a byte of zeros
 */
unsigned char getBit(unsigned char address, unsigned char bit)
{
	return readRegister(address) & bit;
}

/**
 * Write to a SPI address on the camera
 * @param address The register/command address over SPI
 * @param value The data to send with the command or to the register
 */
void busWrite(int address, int value) { SPI_transfer16(((address & 0xFF) << 8) | (value & 0xFF)); }

/**
 * Read from an SPI address on the camera
 * @param address the address of the SPI register
 * @return the value within that address
 */
unsigned char busRead(int address)
{
	unsigned short output = SPI_transfer16((address & 0xFF) << 8);
	return output & 0xFF;
}

/**
 * Write to an I2C register with an 8-bit ID
 * @param regID the ID of the register to write to
 * @param regDat the data to write to the register
 */
void wrSensorReg8_8(int regID, int regDat)
{
	unsigned char camera_data[2];
	camera_data[0] = regID & 0xFF;
	camera_data[1] = regDat & 0xFF;

	Timer_delay_us(10);
	I2C_write(camera_data, 2);
	Timer_delay_us(10);
}

/**
 * Write to a set of I2C registers with 8-bit IDs
 * @param reglist the list of register IDs and the data to put in them
 */
void wrSensorRegs8_8(const struct sensor_reg * reglist)
{
	unsigned int regAddress = 0;
	unsigned int regValue	= 0;

	const struct sensor_reg * next = reglist;

	while((regAddress != 0xff) | (regValue != 0xff))
	{
		regAddress = next->reg;
		regValue   = next->val;
		wrSensorReg8_8(regAddress, regValue);

		Timer_delay_us(10);

		next++;
	}
}

/**
 * Read from an I2C register with an 8-bit ID
 * @param regID the ID of the register to read from
 * @param[out] regDat the data from the register
 */
void rdSensorReg8_8(unsigned char regID, unsigned char * regDat)
{
	Timer_delay_us(10);
	I2C_write(&regID, 1);
	Timer_delay_us(10);
	*regDat = I2C_read();
	Timer_delay_us(10);
}

/**
 * Write to an I2C register with a 16-bit ID
 * @param regID the ID of the register to write to
 * @param regDat the data to write to the register
 */
void wrSensorReg16_8(int regID, int regDat)
{
	unsigned char camera_data[3];
	camera_data[0] = (regID >> 8) & 0xFF;
	camera_data[1] = regID & 0xFF;
	camera_data[2] = regDat & 0xFF;

	Timer_delay_us(10);
	I2C_write(camera_data, 3);
	Timer_delay_us(10);
}

/**
 * Write to a set of I2C registers with 16-bit IDs
 * @param reglist the list of register IDs and the data to put in them
 */
void wrSensorRegs16_8(const struct sensor_reg reglist[])
{
	unsigned int regAddress = 0;
	unsigned int regValue	= 0;

	const struct sensor_reg * next = reglist;

	while((regAddress != 0xffff) | (regValue != 0xff))
	{
		regAddress = next->reg;
		regValue   = next->val;
		wrSensorReg16_8(regAddress, regValue);

		Timer_delay_us(10);

		next++;
	}
}

/**
 * Read from an I2C register with a 16-bit ID
 * @param regID the ID of the register to read from
 * @param[out] regDat the data from the register
 */
void rdSensorReg16_8(unsigned int regID, unsigned char * regDat)
{
	unsigned char camera_data[2];
	camera_data[0] = (regID >> 8) & 0xFF;
	camera_data[1] = regID & 0xFF;

	Timer_delay_us(10);
	I2C_write(camera_data, 2);
	Timer_delay_us(10);
	*regDat = I2C_read();
	Timer_delay_us(10);
}

/**
 * Read from a set of I2C registers with 16-bit IDs
 * @param[out] reglist the IDs to read from and their data output
 */
void rdSensorRegs16_8(struct sensor_reg reglist[])
{
	unsigned int  regAddress = 0;
	unsigned int  regValue	 = 0;
	unsigned char rd_out	 = 0;

	struct sensor_reg * next = reglist;

	while((regAddress != 0xffff) | (regValue != 0xff))
	{
		regAddress = next->reg;
		regValue   = next->val;
		rdSensorReg16_8(regAddress, &rd_out);

		next->val = rd_out;

		Timer_delay_us(10);

		next++;
	}
}