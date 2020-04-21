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
 *    @brief communicates with MicroRNG device through SPI interface on Raspberry PI 3+ or other Linux-based single-board computers.
 *
 */

#ifndef MICRORNGSPI_H
#define MICRORNGSPI_H

#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <sys/stat.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

class MicroRngSPI
{
public:
    MicroRngSPI();
    virtual ~MicroRngSPI();
    bool isConnected();
    bool connect(const char *devicePath);
    bool disconnect();
    bool executeCommand(char cmd, uint8_t *rx);
    const char* getLastErrMsg();
    void setMaxClockFrequency(uint32_t clockHz);
    uint32_t getMaxClockFrequency();
    bool retrieveRandomByte(uint8_t *rx);
    bool retrieveRandomBytes(int len, uint8_t *rx);
    bool retrieveRawRandomByte(uint8_t *rx);
    bool retrieveRawRandomBytes(int len, uint8_t *rx);
    bool retrieveTestByte(uint8_t *rx);
    bool retrieveTestBytes(int len, uint8_t *rx);
    bool retrieveDeviceStatusByte(uint8_t *rx);
    bool shutDownNoiseSources(uint8_t *rx);
    bool startUpNoiseSources(uint8_t *rx);
    bool resetUART(uint8_t *rx);
    bool validateCommunication();
    bool autodetectMaxFrequency();

protected:

private:
    int fd;
    uint32_t clockHz;
    uint32_t maxClockHz;
    uint32_t minClockHz;
    char lastSentCommand;
    uint32_t spiMode;
    uint8_t spiBits;
    bool deviceConnected;
    char lastError[512];
    char testCommand;
    char randomByteCommand;
    char statusByteCommand;
    char rawRandomByteCommand;
    char shutDownCommand;
    char startUpCommand;
    char resetUartSpeedCommand;
    void setErrMsg(const char *errMessage);
    void clearErrMsg();
    void initialize();
    bool exhangeByte(char cmd, uint8_t *rx);

};

#endif // MICRORNGSPI_H
