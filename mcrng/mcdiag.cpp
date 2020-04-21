/**
 *   Copyright (c) 2018 TectroLabs L.L.C.
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
 *    @file MicroRngSPI.h
 *    @author Andrian Belinski
 *    @date 04/21/2020
 *    @version 1.0
 *
 *    @brief MicroRNG diagnostics utility through SPI interface, used on Raspberry PI 3+ or other Linux-based single-board computers.
 *
 */

#include "MicroRngSPI.h"
#include <math.h>

#define BLOCK_SIZE_TEST_BYTES (32000)
#define TEST_RETRIEVE_BLOCKS (20)


int main(int argc, char **argv)
{
    MicroRngSPI spi;
    bool status;
    uint8_t testBuff[BLOCK_SIZE_TEST_BYTES];
    uint8_t rngStatus;
    clock_t t;

    printf("-------------------------------------------------------------------\n");
    printf("--- TectroLabs - mcdiag - MicroRNG diagnostics utility Ver 1.0  ---\n");
    printf("--- Use with RPI 3+ or other Linux-based single-board computers ---\n");
    printf("-------------------------------------------------------------------\n");

    setbuf(stdout, NULL);

    if (argc < 2)
    {
        printf("Usage: mcdiag <spi device>\n");
        printf("Example: mcdiag /dev/spidev0.0\n");
        return -1;
    }

    char *devicePath = argv[1];

    printf("Opening device %s ----------------------------- ", devicePath);
    status = spi.connect(devicePath);
    if (!status)
    {
        printf("*FAILED*, error: %s\n", spi.getLastErrMsg());
        return -1;
    }
    printf("Success\n");

    // Make shure the RNG is turned on
    spi.startUpNoiseSources(testBuff);

    printf("Identifying device %s -------------- ", devicePath);
    status = spi.validateCommunication();
    if (!status)
    {
        printf("MicroRNG not found\n");
        return -1;
    }
    printf(" MicroRNG detected\n");

    printf("Identifying maximum SPI clock frequency --------------- ");
    status = spi.autodetectMaxFrequency();
    if (!status)
    {
        printf("*FAILED*, error: %s\n", spi.getLastErrMsg());
        return -1;
    }
    printf("%8ld Hz\n", (long)spi.getMaxClockFrequency());


    printf("Retrieving %d random bytes ----------------------------- ", BLOCK_SIZE_TEST_BYTES);
    status = spi.retrieveRandomBytes(BLOCK_SIZE_TEST_BYTES, testBuff);
    if (!status)
    {
        printf("*FAILED*, error: %s\n", spi.getLastErrMsg());
        return -1;
    }
    printf("Success\n");

    printf("Retrieving %d RAW random bytes ------------------------- ", BLOCK_SIZE_TEST_BYTES);
    status = spi.retrieveRawRandomBytes(BLOCK_SIZE_TEST_BYTES, testBuff);
    if (!status)
    {
        printf("*FAILED*, error: %s\n", spi.getLastErrMsg());
        return -1;
    }
    printf("Success\n");

    printf("Retrieving %d random bytes ----------------------------- ", BLOCK_SIZE_TEST_BYTES);
    status = spi.retrieveRandomBytes(BLOCK_SIZE_TEST_BYTES, testBuff);
    if (!status)
    {
        printf("*FAILED*, error: %s\n", spi.getLastErrMsg());
        return -1;
    }
    printf("Success\n");

    printf("Shutting down RNG ----------------------------------------- ");
    status = spi.shutDownNoiseSources(testBuff);
    if (!status)
    {
        printf("*FAILED*, error: %s\n", spi.getLastErrMsg());
        return -1;
    }
    if (testBuff[0] == 200)
    {
        printf("Success\n");
    }
    else
    {
        printf("  Error\n");
        return -1;
    }

    printf("Starting RNG up ------------------------------------------- ");
    status = spi.startUpNoiseSources(testBuff);
    if (!status)
    {
        printf("*FAILED*, error: %s\n", spi.getLastErrMsg());
        return -1;
    }
    if (testBuff[0] == 0)
    {
        printf("Success\n");
    }
    else
    {
        printf("  Error\n");
        return -1;
    }

    printf("Computing transfer speed speed --------------------------");
    t = clock();
    for (int i = 0; i < TEST_RETRIEVE_BLOCKS; i++)
    {
        status = spi.retrieveRandomBytes(BLOCK_SIZE_TEST_BYTES, testBuff);
        if (!status)
        {
            printf("*FAILED*, error: %s\n", spi.getLastErrMsg());
            return -1;
        }
    }
    t = clock() - t;
    double durationSecs = ((double)t)/CLOCKS_PER_SEC;
    double kbitsPerSecond =  (double)(BLOCK_SIZE_TEST_BYTES * TEST_RETRIEVE_BLOCKS * 8) / durationSecs / 1024;
    printf("%5.0f kbps\n", kbitsPerSecond);

    printf("Validating MicroRNG internal status  ---------------------- ");
    status = spi.retrieveDeviceStatusByte(&rngStatus);
    if (!status)
    {
        printf("*FAILED*, error: %s\n", spi.getLastErrMsg());
        return -1;
    }
    if (rngStatus == 0)
    {
        printf("Healthy\n");
    }
    else
    {
        printf("RNG failed with status code: %d\n", (int)rngStatus);
        return -1;
    }

    return 0;
}
