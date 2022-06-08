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
 *    @file MicroRngSPI.cpp
 *    @author Andrian Belinski
 *    @date 06/07/2022
 *    @version 1.1
 *
 *    @brief communicates with MicroRNG device through SPI interface on Raspberry PI 3+ or other Linux-based single-board computers.
 *
 */
#include "MicroRngSPI.h"

MicroRngSPI::MicroRngSPI()
{
    initialize();
}

/**
 * Initialize all variables
 *
 */
void MicroRngSPI::initialize()
{
    deviceConnected = false;
    fd = -1;
    setErrMsg("Not Connected");
    spiMode = SPI_CPHA;
    spiBits = 8;
    minClockHz = 250000;
    clockHz = minClockHz;
    lastSentCommand = '\0';
    testCommand = 't';
    randomByteCommand = 'l';
    statusByteCommand = 's';
    rawRandomByteCommand = 'r';
    shutDownCommand = 'D';
    startUpCommand = 'U';
    resetUartSpeedCommand = 'R';
    maxClockHz = 60000000;
}

/**
 * Clear error message
 */
void MicroRngSPI::clearErrMsg()
{
    setErrMsg("");
}

/**
 * Check if connection to SPI device is established
 *
 * @return true if already connected
 */
bool MicroRngSPI::isConnected()
{
    return deviceConnected;
}

/**
 * Connect to MicroRNG using SPI device
 *
 * @param devicePath complete path to SPI device
 *
 * @return true if connected successfully
 */
bool MicroRngSPI::connect(const char *devicePath)
{
    int retCode;

    if (isConnected())
    {
        return false;
    }

    clearErrMsg();

    fd = open(devicePath, O_RDWR);
    if (fd < 0)
    {
        sprintf(lastError, "Could not open SPI device: %s", devicePath);
        return false;
    }

    // Set SPI mode
    retCode = ioctl(fd, SPI_IOC_WR_MODE, &spiMode);
    if (retCode == -1)
    {
        close(fd);
        sprintf(lastError, "Could not set SPI write mode");
        return false;
    }

    retCode = ioctl(fd, SPI_IOC_RD_MODE, &spiMode);
    if (retCode == -1)
    {
        close(fd);
        sprintf(lastError, "Could not set SPI read mode");
        return false;
    }

    // Set 8 bits for data exchange word
    retCode = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &spiBits);
    if (retCode == -1)
    {
        close(fd);
        sprintf(lastError, "Could not set SPI transmission word bits");
        return false;
    }

    retCode = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &spiBits);
    if (retCode == -1)
    {
        close(fd);
        sprintf(lastError, "Could not set SPI word bits");
        return false;
    }

    // Set clock frequency
    retCode = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &clockHz);
    if (retCode == -1)
    {
        close(fd);
        sprintf(lastError, "Could not set SPI transmission clock frequency");
        return false;
    }

    retCode = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &clockHz);
    if (retCode == -1)
    {
        close(fd);
        sprintf(lastError, "Could not set SPI clock frequency");
        return false;
    }

    deviceConnected = true;
    return true;
}

/**
 * Exchange bytes with connected MicroRNG device by sending a command 
 * while receiving response for the previous command.
 *
 * @param cmd one byte command that will be sent to connected device through SPI interface
 * @param rx pointer to receiving byte from connected device through SPI interface
 *
 * @return true when data exchanged successfully
 */
bool MicroRngSPI::exhangeByte(char cmd, uint8_t *rx)
{
    int retCode;
    if (!isConnected())
    {
        return false;
    }
    *rx = 0;    // Set it initially to zero to avoid 'valgrind' complains
    lastSentCommand = cmd;
    struct spi_ioc_transfer tr = {};
    tr.tx_buf = (unsigned long)&cmd;
    tr.rx_buf = (unsigned long)rx;
    tr.len = 1;
    tr.delay_usecs = 0;
    tr.speed_hz = clockHz;
    tr.bits_per_word = spiBits;

    retCode = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    if (retCode < 1)
    {
        sprintf(lastError, "Could not exchange SPI bytes");
        return false;
    }
    return true;
}

/**
 * Exchange bytes with connected MicroRNG device by sending a command while receiving response for the previous command.
 * It may need an additional transfer to execute the command if the last command was different.
 *
 * @param cmd one byte command that will be sent to connected device through SPI interface
 * @param rx pointer to receiving byte from connected device through SPI interface
 *
 * @return true when command executed successfully
 */
bool MicroRngSPI::executeCommand(char cmd, uint8_t *rx)
{
    if (!isConnected())
    {
        return false;
    }

    if (cmd != lastSentCommand)
    {
        // Will need two data transfers because the current command is different from the one that was sent last time.
        if (!exhangeByte(cmd, rx))
        {
            return false;
        }
    }
    return exhangeByte(cmd, rx);
}

/**
 * Check to see if the MicroRNG device is actually responding to requests.
 *
 * @return true when validated successfully
 */
bool MicroRngSPI::validateDevice()
{
    if (!isConnected())
    {
        return false;
    }

    uint8_t beginTransactionID;
    for (int i = 1; i <= 16; ++i)
    {
    	uint8_t transactionID;
        if (!executeCommand('t', &transactionID))
        {
        	return false;
        }

    	if (i == 1)
    	{
    		beginTransactionID = transactionID;
    	}

        if ( i != 1 && transactionID != ++beginTransactionID)
        {
        	sprintf(lastError, "MicroRNG device not found");
        	return false;
        }
    }
    return true;
}

/**
 * Retrieve MicroRNG internal status.
 *
 * @param rx pointer to receiving status byte of the RNG. A zero value indicates a healthy status.
 *
 * @return true when status retieved successfully
 */
bool MicroRngSPI::retrieveDeviceStatusByte(uint8_t *rx)
{
    if (!isConnected())
    {
        return false;
    }
    return executeCommand(statusByteCommand, rx);
}

/**
 * Shuts down both random noise sources of the MicroRNG device.
 * This command is used to enable the sleep mode when the device is not in use to preserve energy.
 *
 * @param rx pointer to RNG status byte. Expected value is decimal 200
 *
 * @return true when command exchanged successfully
 */
bool MicroRngSPI::shutDownNoiseSources(uint8_t *rx)
{
    if (!isConnected())
    {
        return false;
    }
    return executeCommand(shutDownCommand, rx);
}

/**
 * Starts up both random noise sources of the MicroRNG device. This command is used to leave the sleep mode to
 * restore normal functionality of the device.
 *
 * @param rx pointer to receiving status byte of the RNG. A zero value indicates that noise sources are turned on.
 *
 * @return true when command exchanged successfully
 */
bool MicroRngSPI::startUpNoiseSources(uint8_t *rx)
{
    if (!isConnected())
    {
        return false;
    }
    return executeCommand(startUpCommand, rx);
}

/**
 * This call doesn't have to do anything with SPI communication. It resets the UART configuration baud rate to the factory default
 * value 19200 — it will take effect after the device is powered off-and-on or after RST signal assertion.
 * This command is used to restore the factory default MicroRNG device UART baud rate when
 * it has been miss-configured through the 2-wire UART API.
 *
 * @param rx pointer to receiving status byte of the RNG
 *
 * @return true when command exchanged successfully
 */
bool MicroRngSPI::resetUART(uint8_t *rx)
{
    if (!isConnected())
    {
        return false;
    }
    return executeCommand(resetUartSpeedCommand, rx);
}

/**
 * Validate SPI communication with MicroRNG device by sending a series of test commands and inspecting results.
 *
 *
 * @return true when communication to MicroRNG is validated
 */
bool MicroRngSPI::validateCommunication()
{
    if (!isConnected())
    {
        return false;
    }

    uint8_t testBuffer[2048];
    if (!retrieveTestBytes(sizeof(testBuffer), testBuffer))
    {
        return false;
    }

    uint8_t expectedTestByte = 0;
    for (unsigned int i = 0; i < sizeof(testBuffer); i++)
    {
        if (i == 0)
        {
            expectedTestByte = testBuffer[i];
        }
        else
        {
            if (++expectedTestByte != testBuffer[i])
            {
                setErrMsg("Could not validate SPI communication");
                return false;
            }
        }
    }
    return true;
}

/**
 * Identify the maximum SPI master clock frequency while still maintaining a valid communication with MicroRNG device.
 *
 * @return true when maximum clock frequency is successfully determined
 */
bool MicroRngSPI::autodetectMaxFrequency()
{
    bool success = false;
    if (!isConnected())
    {
        return false;
    }
    for (uint32_t freqHz = minClockHz; freqHz < maxClockHz; freqHz += minClockHz)
    {
        uint32_t prevClockHz = getMaxClockFrequency();
        setMaxClockFrequency(freqHz);
        bool status = validateCommunication();
        if (status)
        {
            success = true;
        }
        else
        {
            setMaxClockFrequency(prevClockHz);
            break;
        }
    }
    return success;
}

/**
 * Retrieves a random byte value processed internally with an embedded Linear Corrector (P. Lacharme)
 *
 * @param rx pointer to receiving random byte
 *
 * @return true when random data successfully retrieved
 */
bool MicroRngSPI::retrieveRandomByte(uint8_t *rx)
{
    if (!isConnected())
    {
        return false;
    }
    return executeCommand(randomByteCommand, rx);
}

/**
 * Retrieves random bytes processed internally with an embedded Linear Corrector (P. Lacharme)
 *
 * @param len how many random bytes to retrieve
 * @param rx pointer to receiving random bytes
 *
 * @return true when random data successfully retrieved
 */
bool MicroRngSPI::retrieveRandomBytes(int len, uint8_t *rx)
{
    bool status = true;
    if (!isConnected())
    {
        return false;
    }
    if (len <= 0)
    {
        setErrMsg("Invalid ammount of random bytes requested");
        return false;
    }

    for (int i = 0; i < len; i++)
    {
        bool status = retrieveRandomByte(rx + i);
        if (!status)
        {
            break;
        }
    }

    return status;
}

/**
 * Retrieves the internal SPI transfer ID which is incremented with each transfer.
 * Used for validation of the SPI communication between the master device and the MicroRNG device.
 * Primarily used in the development phase to detect SPI miss-configurations of the master device.
 *
 * @param rx response byte represents the latest SPI transfer ID
 *
 * @return true when transfer ID successfully retrieved
 */
bool MicroRngSPI::retrieveTestByte(uint8_t *rx)
{
    if (!isConnected())
    {
        return false;
    }
    return executeCommand(testCommand, rx);
}

/**
 * Retrieves a series of SPI transfer IDs.
 * Used for validation of the SPI communication between the master device and the MicroRNG device.
 * Primarily used in the development phase to detect SPI miss-configurations of the master device.
 *
 * @param len how many transfer IDs to retrieve
 * @param rx pointer to receiving transfer IDs
 *
 * @return true when transfer IDs successfully retrieved
 */
bool MicroRngSPI::retrieveTestBytes(int len, uint8_t *rx)
{
    bool status = true;
    if (!isConnected())
    {
        return false;
    }
    if (len <= 0)
    {
        setErrMsg("Invalid amount of test bytes requested");
        return false;
    }

    for (int i = 0; i < len; i++)
    {
        bool status = retrieveTestByte(rx + i);
        if (!status)
        {
            break;
        }
    }

    return status;
}


/**
 * Retrieves a raw (unprocessed) random byte value.
 * It should only be used for verification or when used with external post-processing implementations.
 *
 * @param rx pointer to receiving random byte
 *
 * @return true when random data successfully retrieved
 */
bool MicroRngSPI::retrieveRawRandomByte(uint8_t *rx)
{
    if (!isConnected())
    {
        return false;
    }
    return executeCommand(rawRandomByteCommand, rx);
}

/**
 * Retrieves a raw (unprocessed) random bytes.
 * It should only be used for verification or when used with external post-processing implementations.
 *
 * @param len how many random bytes to retrieve
 * @param rx pointer to receiving random bytes
 *
 * @return true when random data successfully retrieved
 */
bool MicroRngSPI::retrieveRawRandomBytes(int len, uint8_t *rx)
{
    bool status = true;
    if (!isConnected())
    {
        return false;
    }
    if (len <= 0)
    {
        setErrMsg("Invalid amount of raw random bytes requested");
        return false;
    }

    for (int i = 0; i < len; i++)
    {
        bool status = retrieveRawRandomByte(rx + i);
        if (!status)
        {
            break;
        }
    }

    return status;
}

/**
 * Set new SPI master clock speed. Setting this value too high may result in miss communication over SPI interface.
 *
 * @param clockHz new SPI master clock frequency in Hz
 *
 */
void MicroRngSPI::setMaxClockFrequency(uint32_t clockHz)
{
    this->clockHz = clockHz;
}

/**
 * Get current SPI master clock frequency
 *
 * @return current SPI master clock frequency in Hz
 *
 */
uint32_t MicroRngSPI::getMaxClockFrequency()
{
    return clockHz;
}

/**
 * Set error message
 *
 * @param errMessage new error message
 */
void MicroRngSPI::setErrMsg(const char *errMessage)
{
    strcpy(this->lastError, errMessage);
}

/**
 * Disconnect from SPI interface device
 *
 * @return true when executed successfully
 *
 */
bool MicroRngSPI::disconnect()
{
    if (!isConnected())
    {
        return false;
    }
    close(fd);
    initialize();
    return true;
}

/**
 * Retrieves a pointer to the internally saved error message
 */
const char* MicroRngSPI::getLastErrMsg()
{
    return this->lastError;
}

/**
 * De-allocate resources
 */
MicroRngSPI::~MicroRngSPI()
{
    if (isConnected())
    {
        disconnect();
    }
}
