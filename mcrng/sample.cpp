/**
 *   Copyright (C) 2014-2022 TectroLabs LLC, https://tectrolabs.com
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
 *    @file sample.cpp
 *    @author Andrian Belinski
 *    @date 06/07/2022
 *    @version 1.1
 *
 *    @brief a sample C program that demonstrates how to retrieve random bytes from MicroRNG device through SPI interface on Raspberry PI 3+ or other Linux-based single-board computers.
 *
 */
#include "MicroRngSPI.h"

#define BYTE_BUFF_SIZE (10)
#define DEC_BUFF_SIZE (10)

unsigned char randombyte[BYTE_BUFF_SIZE]; // Allocate memory for random bytes
unsigned int randomint[DEC_BUFF_SIZE]; // Allocate memory for random integers

//
// Main entry
//
int main(int argc, char **argv) {
	int i;
	double d;
	unsigned int ui;
	MicroRngSPI spi;

	printf("--------------------------------------------------------------------------\n");
	printf("--- Sample C program for retrieving random bytes from MicroRNG device ----\n");
	printf("---    Use with RPI 3+ or other Linux-based single-board computers     ---\n");
	printf("--------------------------------------------------------------------------\n");

	setbuf(stdout, NULL);

	if (argc < 2)
	{
		printf("Usage: sample <spi device>\n");
		printf("Example: sample /dev/spidev0.0\n");
		return -1;
	}

    	char *devicePath = argv[1];

	if (!spi.connect(devicePath)) {
		printf("%s\n", spi.getLastErrMsg());
		return -1;
	}

	if (!spi.validateDevice()) {
		printf("%s\n", spi.getLastErrMsg());
		return -1;
	}

	printf("\nMicroRNG device open successfully, SPI clock frequency: %8ld Hz\n\n", (long)spi.getMaxClockFrequency());

	// Retrieve random bytes from device
	if (!spi.retrieveRandomBytes(BYTE_BUFF_SIZE, randombyte)) {
		printf("%s\n", spi.getLastErrMsg());
		return -1;
	}

	printf("*** Generating %d random bytes ***\n", BYTE_BUFF_SIZE);
	// Print random bytes
	for (i = 0; i < BYTE_BUFF_SIZE; i++) {
		printf("random byte %d -> %d\n", i, (int)randombyte[i]);
	}

	// Retrieve random integers from device
	if (!spi.retrieveRandomBytes(DEC_BUFF_SIZE * sizeof(unsigned int), (unsigned char*)randomint)) {
		printf("%s\n", spi.getLastErrMsg());
		return -1;
	}

	printf("\n*** Generating %d random numbers between 0 and 1 with 5 decimals  ***\n", DEC_BUFF_SIZE);
	// Print random bytes
	for (i = 0; i < DEC_BUFF_SIZE; i++) {
		ui = randomint[i] % 99999;
		d = (double)ui / 100000.0;
		printf("random number -> %lf\n", d);
	}

	printf("\n");
	return (0);

}
