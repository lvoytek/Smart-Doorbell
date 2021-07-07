# Copyright (c) 2021 Lena Voytek
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

# PREFIX is environment variable, but if it is not set, then set default value
ifeq ($(PREFIX),)
    PREFIX := /usr/local
endif

SHELL := /bin/bash

LIBARGS ?= -c -fpic
CCFLAGS ?= -Wall -Werror

DEFINES ?= DEBUG

OUTDIR ?= build

CCFLAGS += "-O"

.PHONY:all
all:$(OUTDIR)/smart-doorbell

# Smart Doorbell CLI app creation
$(OUTDIR)/smart-doorbell:$(OUTDIR)/libCamera.so $(OUTDIR)/include/Camera.h $(OUTDIR)/libButton.so $(OUTDIR)/include/Button.h
	$(CC) -Wl,-R -Wl,$(CURDIR)/$(OUTDIR) $(CCFLAGS) -pthread -D$(DEFINES) -I$(OUTDIR)/include -L$(OUTDIR) -lCamera -lTimer -lButton -lGPIO -li2c -lI2C -lSPI -I$(OUTDIR)/include -o $@ src/main/SmartDoorbellCLI.c

# ArduCAM Library
$(OUTDIR)/libCamera.so:$(OUTDIR)/libTimer.so $(OUTDIR)/include/Timer.h $(OUTDIR)/libGPIO.so $(OUTDIR)/include/GPIODriver.h $(OUTDIR)/libI2C.so $(OUTDIR)/include/I2CDriver.h $(OUTDIR)/libSPI.so $(OUTDIR)/include/SPIDriver.h src/camera
	$(CC) $(LIBARGS) $(CCFLAGS) -D$(DEFINES) -L$(OUTDIR) -lTimer -lGPIO -lI2C -lSPI -I$(OUTDIR)/include src/camera/Camera.c -o $(OUTDIR)/camera.o
	$(CC) -shared -o $@ $(OUTDIR)/camera.o

$(OUTDIR)/include/Camera.h:src/camera
	cp src/camera/ArduCAM.h $(OUTDIR)/include/
	cp src/camera/Camera.h $(OUTDIR)/include/
	cp src/camera/ov5642_regs.h $(OUTDIR)/include/

# SPI Library
$(OUTDIR)/libSPI.so:$(OUTDIR)/include/Debug.h $(OUTDIR)/libGPIO.so $(OUTDIR)/include/GPIODriver.h src/SPI
	$(CC) $(LIBARGS) $(CCFLAGS) -D$(DEFINES) -L$(OUTDIR) -lGPIO -I$(OUTDIR)/include src/SPI/SPIDriver.c -o $(OUTDIR)/SPI.o
	$(CC) -shared -o $@ $(OUTDIR)/SPI.o

$(OUTDIR)/include/SPIDriver.h:src/SPI
	cp src/SPI/SPIDriver.h $(OUTDIR)/include/

# I2C Library
$(OUTDIR)/libI2C.so:$(OUTDIR)/include/Debug.h src/I2C
	$(CC) $(LIBARGS) $(CCFLAGS) -D$(DEFINES) -L$(OUTDIR) -li2c -I$(OUTDIR)/include src/I2C/I2CDriver.c -o $(OUTDIR)/I2C.o
	$(CC) -shared -o $@ $(OUTDIR)/I2C.o

$(OUTDIR)/include/I2CDriver.h:src/I2C
	cp src/I2C/I2CDriver.h $(OUTDIR)/include/

# Button Library
$(OUTDIR)/libButton.so:$(OUTDIR)/include/Debug.h $(OUTDIR)/include/Timer.h $(OUTDIR)/libGPIO.so src/button
	$(CC) $(LIBARGS) $(CCFLAGS) -D$(DEFINES) -L$(OUTDIR) -lTimer -lGPIO -I$(OUTDIR)/include src/button/Button.c -o $(OUTDIR)/Button.o
	$(CC) -shared -o $@ $(OUTDIR)/Button.o

$(OUTDIR)/include/Button.h:src/button
	cp src/button/Button.h $(OUTDIR)/include/

# GPIO Library
$(OUTDIR)/libGPIO.so:$(OUTDIR)/include/Debug.h src/GPIO
	$(CC) $(LIBARGS) $(CCFLAGS) -D$(DEFINES) -L$(OUTDIR) -I$(OUTDIR)/include src/GPIO/GPIODriver.c -o $(OUTDIR)/GPIO.o
	$(CC) -shared -o $@ $(OUTDIR)/GPIO.o

$(OUTDIR)/include/GPIODriver.h:src/GPIO
	cp src/GPIO/GPIODriver.h $(OUTDIR)/include/

# Timer Library
$(OUTDIR)/libTimer.so:$(OUTDIR)/include/Debug.h src/timer
	$(CC) $(LIBARGS) $(CCFLAGS) -D$(DEFINES) -L$(OUTDIR) -I$(OUTDIR)/include src/timer/Timer.c -o $(OUTDIR)/timer.o
	$(CC) -shared -o $@ $(OUTDIR)/timer.o

$(OUTDIR)/include/Timer.h:src/timer
	cp src/timer/Timer.h $(OUTDIR)/include/

# Debug file
$(OUTDIR)/include/Debug.h:src/board
	mkdir -p $(OUTDIR)/
	mkdir -p $(OUTDIR)/include
	cp src/board/Debug.h $(OUTDIR)/include/

.PHONY:install
install:
	install -d $(DESTDIR)$(PREFIX)/lib/
	install -m 644 $(OUTDIR)/libCamera.so $(DESTDIR)$(PREFIX)/lib/
	install -m 644 $(OUTDIR)/libSPI.so $(DESTDIR)$(PREFIX)/lib/
	install -m 644 $(OUTDIR)/libI2C.so $(DESTDIR)$(PREFIX)/lib/
	install -m 644 $(OUTDIR)/libButton.so $(DESTDIR)$(PREFIX)/lib/
	install -m 644 $(OUTDIR)/libGPIO.so $(DESTDIR)$(PREFIX)/lib/
	install -m 644 $(OUTDIR)/libTimer.so $(DESTDIR)$(PREFIX)/lib/
	install -d $(DESTDIR)$(PREFIX)/include/
	install -m 644 $(OUTDIR)/include/Debug.h $(DESTDIR)$(PREFIX)/include/
	install -m 644 $(OUTDIR)/include/SPIDriver.h $(DESTDIR)$(PREFIX)/include/
	install -m 644 $(OUTDIR)/include/Camera.h $(DESTDIR)$(PREFIX)/include/
	install -m 644 $(OUTDIR)/include/I2CDriver.h $(DESTDIR)$(PREFIX)/include/
	install -m 644 $(OUTDIR)/include/Button.h $(DESTDIR)$(PREFIX)/include/
	install -m 644 $(OUTDIR)/include/GPIODriver.h $(DESTDIR)$(PREFIX)/include/
	install -m 644 $(OUTDIR)/include/Timer.h $(DESTDIR)$(PREFIX)/include/
	install -d $(DESTDIR)$(PREFIX)/bin/
	install -m 644 $(OUTDIR)/smart-doorbell $(DESTDIR)$(PREFIX)/bin/

.PHONY:clean
clean:
	rm -rf $(OUTDIR)/
