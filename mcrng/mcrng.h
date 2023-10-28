/**
 *   Copyright (C) 2014-2023 TectroLabs LLC, https://tectrolabs.com
 *
 *    Permission is hereby granted, free of charge, to any person obtaining
 *    a copy of this software and associated documentation files (the "Software"),
 *    to deal in the Software without restriction, including without limitation
 *    the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *    and/or sell copies of the Software, and to permit persons to whom the Software
 *    is furnished to do so, subject to the following conditions:
 *
 *    The above copyright notice and this permission notice shall be included in
 *    all copies or substantial portions of the Software.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 *    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 *    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 *    OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 *    @file mcrng.h
 *    @author Andrian Belinski
 *    @date 10/28/2023
 *    @version 1.1
 *
 *    @brief downloads random bytes from MicroRNG device through SPI interface on Raspberry PI 3+ or other Linux-based single-board computers.
 *
 */
#ifndef MCRNG_H_
#define MCRNG_H_

#include "MicroRngSPI.h"
#include <unistd.h>

#include <stdlib.h>
#include <fcntl.h>

#define MCR_BUFF_FILE_SIZE_BYTES (32000)
#define DEFAULT_SPI_DEV_PATH "/dev/spidev0.0"

/**
 * Total number of random bytes needed (a command line argument) max 100000000000 bytes
 */
static int64_t numGenBytes = -1;

/**
 * File name for recording the random bytes (a command line argument)
 */
static char *filePathName = NULL;

/**
 * SPI device path
 */
static char devicePath[256];

/**
 * Max SPI master clock frequency in Hz
 */
static uint32_t maxSpiMasterClock = 250000;

static FILE *pOutputFile = NULL;
static bool isOutputToStandardOutput = false;
static MicroRngSPI spi;

/**
 * Function Declarations
 */
static void displayUsage();
static int processArguments(int argc, char **argv);
static bool validateArgumentCount(int curIdx, int actualArgumentCount);
static int parseDevicePath(int idx, int argc, char **argv);
static int processDownloadRequest();
static int handleDownloadRequest();
static void writeBytes(const uint8_t *bytes, uint32_t numBytes);

#endif /* MCRNG_H_ */
