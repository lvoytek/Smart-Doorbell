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

#ifndef CAMERA_H
#define CAMERA_H

enum BUFFER_SIZE
{
	JPEG_BUFFER_SIZE = 2 * 1024 * 1024,
	CMD_BUFFER_SIZE	 = 512
};

enum CHIPID_LEVEL
{
	CHIPID_HIGH = 0x300a,
	CHIPID_LOW	= 0x300b
};

typedef enum
{
	IMG_BMP = 0,
	IMG_JPEG,
	IMG_RAW
} IMAGE_TYPE;

typedef enum
{
	RES_320x240 = 0,	// 320x240
	RES_640x480,		// 640x480
	RES_1024x768,		// 1024x768
	RES_1280x960,		// 1280x960
	RES_1600x1200,		// 1600x1200
	RES_2048x1536,		// 2048x1536
	RES_2592x1944		// 2592x1944
} RESOLUTION;

typedef enum
{
	MODE_AUTO = 0,
	MODE_SUNNY,
	MODE_CLOUDY,
	MODE_OFFICE,
	MODE_HOME
} LIGHT_MODE;

typedef enum
{
	MODE_ADVANCED_AWB = 0,
	MODE_SIMPLE_AWB,
	MODE_MANUAL_DAY,
	MODE_MANUAL_A,
	MODE_MANUAL_CWF,
	MODE_MANUAL_CLOUDY
} ADVANCED_LIGHT_MODE;

typedef enum
{
	SAT_4 = 0,
	SAT_3,
	SAT_2,
	SAT_1,
	SAT_0,
	SAT_NEG_1,
	SAT_NEG_2,
	SAT_NEG_3,
	SAT_NEG_4
} COLOR_SATURATION;

typedef enum
{
	BRIGHTNESS_4 = 0,
	BRIGHTNESS_3,
	BRIGHTNESS_2,
	BRIGHTNESS_1,
	BRIGHTNESS_0,
	BRIGHTNESS_NEG_1,
	BRIGHTNESS_NEG_2,
	BRIGHTNESS_NEG_3,
	BRIGHTNESS_NEG_4
} BRIGHTNESS;

typedef enum
{
	CONTRAST_4 = 0,
	CONTRAST_3,
	CONTRAST_2,
	CONTRAST_1,
	CONTRAST_0,
	CONTRAST_NEG_1,
	CONTRAST_NEG_2,
	CONTRAST_NEG_3,
	CONTRAST_NEG_4
} CONTRAST;

typedef enum
{
	DEG_NEG_180 = 0,
	DEG_NEG_150,
	DEG_NEG_120,
	DEG_NEG_90,
	DEG_NEG_60,
	DEG_NEG_30,
	DEG_0,
	DEG_30,
	DEG_60,
	DEG_90,
	DEG_120,
	DEG_150
} ROTATION;

typedef enum
{
	EFFECT_BLUISH,
	EFFECT_GREENISH,
	EFFECT_REDDISH,
	EFFECT_BW,
	EFFECT_NEGATIVE,
	EFFECT_NORMAL,
	EFFECT_SEPIA,
} SPECIAL_EFFECTS;

typedef enum
{
	EXP_NEG_17_EV = 0,
	EXP_NEG_13_EV,
	EXP_NEG_10_EV,
	EXP_NEG_07_EV,
	EXP_NEG_03_EV,
	EXP_DEFAULT,
	EXP_07_EV,
	EXP_10_EV,
	EXP_13_EV,
	EXP_17_EV,
	EXP_03_EV
} EXPOSURE;

typedef enum
{
	SHARP_AUTO_DEFAULT = 0,
	SHARP_AUTO_1,
	SHARP_AUTO_2,
	SHARP_MANUAL_OFF,
	SHARP_MANUAL_1,
	SHARP_MANUAL_2,
	SHARP_MANUAL_3,
	SHARP_MANUAL_4,
	SHARP_MANUAL_5
} SHARPNESS_TYPE;

typedef enum
{
	SHARP_1 = 0,
	SHARP_2,
	SHARP_3,
	SHARP_4,
	SHARP_5,
	SHARP_6,
	SHARP_7,
	SHARP_8,
	SHARP_AUTO
} SHARPNESS_LEVEL;

typedef enum
{
	EV_3 = 0,
	EV_2,
	EV_1,
	EV_0,
	EV_NEG_1,
	EV_NEG_2,
	EV_NEG_3
} EV;

enum FLIP
{
	FLIP_HORIZONTAL = 0,
	FLIP_VERTICAL,
	FLIP_BOTH
};

enum QUALITY
{
	QUALITY_HIGH = 0,
	QUALITY_DEFAULT,
	QUALITY_LOW
};

enum COLOR_TYPE
{
	COLOR_BAR = 0,
	COLOR_SQUARE,
	COLOR_BW_SQUARE,
	COLOR_DLI
};

enum NIGHT_MODE
{
	NIGHT_MODE_ON = 0,
	NIGHT_MODE_OFF
};

enum FRAMERATE_DETECT
{
	FRAMERATE_OFF = 0,
	FRAMERATE_MANUAL_50HZ,
	FRAMERATE_MANUAL_60HZ,
	FRAMERATE_AUTO_DETECT
};

void Camera_init(int i2c_bus, unsigned int spi_bus, unsigned int spi_cs);
void Camera_shutdown();

void Camera_set_image_format(IMAGE_TYPE img_format);
void Camera_set_resolution(RESOLUTION res);
void Camera_set_color_saturation(COLOR_SATURATION sat);
void Camera_set_brightness(BRIGHTNESS level);
void Camera_set_special_effect(SPECIAL_EFFECTS effect);
void Camera_set_sharpness_type(SHARPNESS_TYPE sharpness);

void Camera_reset_firmware();
void Camera_single_capture();
void Camera_start_capture();

#endif