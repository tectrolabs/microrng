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
 *    @file mcrng.cpp
 *    @author Andrian Belinski
 *    @date 10/28/2023
 *    @version 1.2
 *
 *    @brief downloads random bytes from MicroRNG device through SPI interface on Raspberry PI 3+ or other Linux-based single-board computers.
 *
 */
#include "mcrng.h"

/**
 * Display usage message
 *
 */
static void displayUsage() {
    printf("---------------------------------------------------------------------------\n");
    printf("---     TectroLabs - mcrng - MicroRNG download utility Version 1.2      ---\n");
    printf("---     Use with RPI 3+ or other Linux-based single-board computers     ---\n");
    printf("---------------------------------------------------------------------------\n");
    printf("NAME\n");
    printf("     mcrng  - True Random Number Generator MicroRNG download utility \n");
    printf("SYNOPSIS\n");
    printf("     mcrng  [options] \n");
    printf("\n");
    printf("DESCRIPTION\n");
    printf("     Mcrng downloads random bytes from MicroRNG device into a data file.\n");
    printf("\n");
    printf("OPTIONS\n");
    printf("     Operation modifiers:\n");
    printf("\n");
    printf("     -fn FILE, --file-name FILE\n");
    printf("           a FILE name for storing random data. Use STDOUT to send bytes\n");
    printf("           to standard output\n");
    printf("\n");
    printf("     -nb NUMBER, --number-bytes NUMBER\n");
    printf("           NUMBER of random bytes to download, max value 200000000000,\n");
    printf("           skip this option for continuous download of random bytes\n");
    printf("\n");
    printf("     -dp PATH, --device-path PATH\n");
    printf("           SPI device path, default value: /dev/spidev0.0\n");
    printf("\n");
    printf("     -cf NUMBER, --clock-frequency NUMBER\n");
    printf("           SPI master clock frequency in KHz, max value 60000,\n");
    printf("           skip this option for default 250 KHz frequency.\n");
    printf("           Setting this value too high may result in miscommunication.\n");
    printf("           Use 'mcdiag' utility to determine the max frequency.\n");
    printf("EXAMPLES:\n");
    printf("     It may require 'sudo' permissions to run this utility.\n");
    printf("     To download 12 MB of true random bytes to 'rnd.bin' file\n");
    printf("           mcrng  -dd -fn rnd.bin -nb 12000000\n");
    printf("     To download 12 MB of true random bytes to a file using device path\n");
    printf("           mcrng  -dd -fn rnd.bin -nb 12000000 -dp /dev/spidev0.0\n");
    printf("     To download 12 MB of true random bytes to standard output\n");
    printf("           mcrng  -dd -fn STDOUT -nb 12000000 -dp /dev/spidev0.0\n");
    printf("\n");
}

/**
 * Validate command line argument count
 *
 * @param int curIdx
 * @param int actualArgumentCount
 * @return true if run successfully
 */
static bool validateArgumentCount(int curIdx, int actualArgumentCount) {
	if (curIdx >= actualArgumentCount) {
		fprintf(stderr, "\nMissing command line arguments\n\n");
		displayUsage();
		return false;
	}
	return true;
}

/**
 * Parse device path if specified
 *
 * @param int idx - current parameter number
 * @param int argc - number of parameters
 * @param char ** argv - parameters
 * @return int - 0 when successfully parsed
 */
static int parseDevicePath(int idx, int argc, char **argv) {
	if (idx < argc) {
		if (strcmp("-dp", argv[idx]) == 0
				|| strcmp("--device-path", argv[idx]) == 0) {
			if (validateArgumentCount(++idx, argc) == false) {
				return -1;
			}
			strcpy(devicePath, argv[idx++]);
		}
	}
	return 0;
}

/**
 * Parse arguments for extracting command line parameters
 *
 * @param int argc
 * @param char** argv
 * @return int - 0 when run successfully
 */
static int processArguments(int argc, char **argv) {
	int idx = 1;
	strcpy(devicePath, DEFAULT_SPI_DEV_PATH);
	if (argc < 2) {
		displayUsage();
		return -1;
	}
	while (idx < argc) {
		if (strcmp("-nb", argv[idx]) == 0
				|| strcmp("--number-bytes", argv[idx]) == 0) {
			if (validateArgumentCount(++idx, argc) == false) {
				return -1;
			}
			numGenBytes = atoll(argv[idx++]);
			if (numGenBytes > 200000000000) {
				fprintf(stderr, "Number of bytes cannot exceed 200000000000\n");
				return -1;
			}
		} else if (strcmp("-fn", argv[idx]) == 0
				|| strcmp("--file-name", argv[idx]) == 0) {
			if (validateArgumentCount(++idx, argc) == false) {
				return -1;
			}
			filePathName = argv[idx++];
		} else if (strcmp("-cf", argv[idx]) == 0
				|| strcmp("--clock-frequency", argv[idx]) == 0) {
			if (validateArgumentCount(++idx, argc) == false) {
				return -1;
			}
			maxSpiMasterClock = (uint32_t) atoi(argv[idx++]) * 1000;
		} else if (parseDevicePath(idx, argc, argv) == -1) {
			return -1;
		} else {
			// Could not handle the argument, skip to the next one
			++idx;
		}
	}
	return processDownloadRequest();
}

/**
 * Close file handle
 *
 */
static void closeHandle() {
	if (pOutputFile != nullptr) {
		fclose(pOutputFile);
		pOutputFile = nullptr;
	}
}

/**
 * Write bytes out to the file
 *
 * @param const uint8_t* bytes - pointer to the byte array
 * @param uint32_t numBytes - number of bytes to write
 */
static void writeBytes(const uint8_t *bytes, uint32_t numBytes) {
	FILE *handle = pOutputFile;
	fwrite(bytes, 1, numBytes, handle);
}

/**
 * Handle download request
 *
 * @return int - 0 when run successfully
 */
static int handleDownloadRequest() {

	uint8_t receiveByteBuffer[MCR_BUFF_FILE_SIZE_BYTES];

	if (!spi.connect(devicePath)) {
		fprintf(stderr, " Cannot open SPI device %s, error: %s ... \n",
				devicePath, spi.getLastErrMsg());
		return -1;
	}

	spi.setMaxClockFrequency(maxSpiMasterClock);

	if (!spi.validateDevice()) {
		fprintf(stderr, " Cannot access device, error: %s ... \n",
				spi.getLastErrMsg());
		return -1;
	}

	if (filePathName == nullptr) {
		fprintf(stderr, "No file name defined.\n");
		return -1;
	}

	if (isOutputToStandardOutput == true) {
		pOutputFile = fdopen(dup(fileno(stdout)), "wb");
	} else {
		pOutputFile = fopen(filePathName, "wb");
	}

	if (pOutputFile == nullptr) {
		fprintf(stderr, "Cannot open file: %s in write mode\n", filePathName);
		return -1;
	}

	while (numGenBytes == -1) {
		// Infinite loop for downloading unlimited random bytes
		if (!spi.retrieveRandomBytes(MCR_BUFF_FILE_SIZE_BYTES,
				receiveByteBuffer)) {
			fprintf(stderr,
					"Failed to receive %d bytes for unlimited download, error: %s. \n",
					MCR_BUFF_FILE_SIZE_BYTES, spi.getLastErrMsg());
			return -1;
		}
		writeBytes(receiveByteBuffer, MCR_BUFF_FILE_SIZE_BYTES);
	}

	// Calculate number of complete random byte chunks to download
	int64_t numCompleteChunks = numGenBytes / MCR_BUFF_FILE_SIZE_BYTES;

	// Calculate number of bytes in the last incomplete chunk
	uint32_t chunkRemaindBytes = (uint32_t) (numGenBytes
			% MCR_BUFF_FILE_SIZE_BYTES);

	// Process each chunk
	for (int64_t chunkNum = 0; chunkNum < numCompleteChunks; chunkNum++) {
		if (!spi.retrieveRandomBytes(MCR_BUFF_FILE_SIZE_BYTES,
				receiveByteBuffer)) {
			fprintf(stderr, "Failed to receive %d bytes, error: %s. \n",
			MCR_BUFF_FILE_SIZE_BYTES, spi.getLastErrMsg());
			return -1;
		}
		writeBytes(receiveByteBuffer, MCR_BUFF_FILE_SIZE_BYTES);
	}

	if (chunkRemaindBytes > 0) {
		//Process incomplete chunk
		if (!spi.retrieveRandomBytes(chunkRemaindBytes, receiveByteBuffer)) {
			fprintf(stderr,
					"Failed to receive %d bytes for last chunk, error: code %s. ",
					chunkRemaindBytes, spi.getLastErrMsg());
			return -1;
		}
		writeBytes(receiveByteBuffer, chunkRemaindBytes);
	}

	closeHandle();
	return 0;
}

/**
 * Process Request
 *
 * @return int - 0 when run successfully
 */
static int processDownloadRequest() {
	if (filePathName != nullptr
			&& (!strcmp(filePathName, "STDOUT")
					|| !strcmp(filePathName, "/dev/stdout"))) {
		isOutputToStandardOutput = true;
	} else {
		isOutputToStandardOutput = false;
	}
	int status = handleDownloadRequest();
	return status;
}

/**
 * Main entry
 *
 * @param int argc - number of parameters
 * @param char ** argv - parameters
 *
 */
int main(int argc, char **argv) {
	if (!processArguments(argc, argv)) {
		return -1;
	}
	return 0;
}
