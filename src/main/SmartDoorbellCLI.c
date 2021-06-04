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
 * SmartDoorbellCLI
 *
 * This file contains the main entry and setup for the CLI version of the Smart
 * Doorbell application
 */

#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdio.h>

#include <Camera.h>
#include <Button.h>

#define SMART_DOORBELL_VERSION "1.00"

const int doorbell_button_gpio	   = 86;
const int doorbell_video_runtime_s = 30;

char runonce = 0;

void * camera_thread_handler(void * arg);
void * doorbell_thread_handler(void * arg);

int main(int argc, char * argv[])
{
	pthread_t doorbell_thread;

	for(int i = 1; i < argc; i++)
	{
		// Exit application after one doorbell press and video feed
		if(strncmp(argv[i], "-s", 2) == 0 || strncmp(argv[i], "--single", 8) == 0)
		{
			runonce = 1;
			break;
		}
		// Show help menu
		else if(strncmp(argv[i], "-h", 2) == 0 || strncmp(argv[i], "--help", 6) == 0)
		{
			printf(
				"See the README file at https://github.com/lvoytek/Smart-Doorbell for setup "
				"information\n"
				"Options:\n"
				"  -s, --single\t\t\tExit the application after the doorbell is pressed and the "
				"video feed ends\n"
				"  -h, --help\t\t\tDisplay this screen and exit\n"
				"  -v, --version\t\tDisplay the software version number and exit\n");
			return 0;
		}
		// Show version
		else if(strncmp(argv[i], "-v", 2) == 0 || strncmp(argv[i], "--version", 9) == 0)
		{
			printf("Smart Doorbell version %s\n", SMART_DOORBELL_VERSION);
			return 0;
		}
	}

	do {
		pthread_create(&doorbell_thread, NULL, doorbell_thread_handler, NULL);
		pthread_join(doorbell_thread, NULL);
	} while(runonce == 0);
}

/**
 * Function for handling use of doorbell on its own thread
 * @param arg Unused
 * @return Unused
 */
void * doorbell_thread_handler(void * arg)
{
	pthread_t camera_thread;

	Camera_init(2, 1, 0);
	Button_init(doorbell_button_gpio);
	Button_wait_for_press(doorbell_button_gpio);

	// Run doorbell video for 30 seconds
	pthread_create(&camera_thread, NULL, camera_thread_handler, NULL);
	sleep(doorbell_video_runtime_s);
	pthread_kill(camera_thread, 0);

	Camera_shutdown();

	return 0;
}

/**
 * Function for handling use of camera video on its own thread
 * @param arg Unused
 * @return Unused
 */
void * camera_thread_handler(void * arg)
{
	while(1)
	{
		Camera_single_capture();
		Camera_save_capture_to_file("image.jpg");
	}

	return 0;
}
