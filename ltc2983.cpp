//
//  ltc2983.cpp
//  xcode_project
//
//  Created by Kristoffer Andersen on 07/04/2016.
//  Copyright © 2016 your name. All rights reserved.
//

#include "ltc2983.h"
#include <mbed.h>

LTC2983::LTC2983()
{
    CyPins_SetPinDriveMode(SpiChipSelectPin, CY_PINS_DM_STRONG);

    LT_SPI_Start();

    measureChannel = 1;
    inited = false;
}

void LTC2983::init()
{
    if (inited)
        return;

    debug("Starting therm...\n\r");
    bool success = waitForStartup();
    if (!success)
        return;

    debug("Setting up channel 1 assignment...\n\r");
    success = assignChannel(LTC2983::REG_CH1, 0x11200000);
    //success = therm.assignChannel(LTC2983::REG_CH13, 0x7A295000);
    if (!success)
    {
        debug("could not assign channel!\n\r");
        return;
    }

    debug("Setting up channel 8 assignment...\n\r");
    success = assignChannel(LTC2983::REG_CH4, 0xE6500C49);
    //success = therm.assignChannel(LTC2983::REG_CH8, 0xE81F4000);
    if (!success)
    {
        debug("could not assign channel!\n\r");
        return;
    }

    inited = true;
}

/// MARK: ITemperature Interface Methods

int LTC2983::Read()
{
    return ReadMilliCelcius() >> 10;
}

int LTC2983::ReadMilliCelcius()
{
    if (!inited)
        init();
    
    bool success = startConversion(measureChannel);
    if (!success)
    {
        debug("could not start conversion!\n\r");
        return 0;
    }
    success = waitForConversionDone();
    if (!success)
    {
        debug("wait failed!\n\r");
        return 0;
    }
    int32_t temp = 0;
    success = readResult(measureChannel, &temp);
    if (!success)
    {
        debug("failed to read result!\n\r");
        return 0;
    }

    return temp;
}

bool LTC2983::waitForStartup()
{
    wait_ms(250);

    uint8_t status;
    if (!readRegister(REG_CMD_STATUS, &status))
    {
        debug("Failed to read CMD stat reg. on startup!\n\r");
        return false;
    }

    if (status != 0x40)
    {
        debug("CMD stat reg. is not 0x40 but 0x%X\n\r",status);
        return false;
    }

    return true;
}

bool LTC2983::assignChannel(Registers channel, uint32_t chData)
{
    return writeRegister(channel, &chData, 4);
}

bool LTC2983::startConversion(uint8_t channelNum)
{
    uint8_t cmd = 0x80 | (channelNum & 0x1F);
    
    return writeRegister(REG_CMD_STATUS, &cmd);
}

bool LTC2983::checkConversionDone()
{
    uint8_t status = 0x0;
    if (!readRegister(REG_CMD_STATUS, &status))
    {
        debug("failed to read register, in check conv. done!\n\r");
        return false;
    }

    return status & 0x40 ? true : false;
}

bool LTC2983::waitForConversionDone(int timeoutSecs)
{
    wait_ms(80);
    timestamp_t timeout = us_ticker_read()+1000000*timeoutSecs;

    while(checkConversionDone() == false && timeout > us_ticker_read())
    {
        wait_ms(10);
    }

    return checkConversionDone();
}

bool LTC2983::readResult(uint8_t channelNum, int32_t *temperature)
{
    int32_t rawResult = 0x00;
    uint16_t addr = 0;
    switch (channelNum) {
        case 1:
            addr = REG_CH1_RESULT;
            break;
        case 8:
            addr = REG_CH8_RESULT;
            break;
        case 13:
            addr = REG_CH13_RESULT;
            break;
        default:
            return false;
            break;
    }

    if (!readRegister(addr, &rawResult, 4))
    {
        debug("failed to read temp result!\n\r");
        return false;
    }

    debug("Raw temp readout: 0x%X\n\r",rawResult);
    *temperature = (rawResult << 8) >> 8;
    return true;
}

/// MARK: BASIC SPI COMM

void LTC2983::setCS(bool active)
{
    if (active)
        CyPins_ClearPin(SpiChipSelectPin);
    else
        CyPins_SetPin(SpiChipSelectPin);
}

bool LTC2983::CS()
{
    return CyPins_ReadPin(SpiChipSelectPin);
}

uint8_t LTC2983::spiTransfer(uint8_t write)
{
    int timeout = 1000;
    LT_SPI_WriteTxData(write);
    int to = 0;
    while((LT_SPI_ReadTxStatus() & LT_SPI_STS_BYTE_COMPLETE) == 0 && timeout > to++)
    {
        CyDelayUs(1);
    }

    int readByte = 0;
    if (LT_SPI_GetRxBufferSize() > 0)
        readByte = LT_SPI_ReadRxData();

    if (to < timeout)
        return readByte;
    else
        return 0;
}


/// MARK: CHIP REGISTER INTERFACE COMM.

bool LTC2983::readRegister(uint16_t addr, void *data, int bytesToRead)
{
    uint8_t *byteData = (uint8_t*) data;
    setCS();

    //send read instruction
    spiTransfer(SPI_READ);

    //send register address
    spiTransfer(addr >> 8);
    spiTransfer(addr & 0xFF);

    // send data bytes
    for (int b=0; b<bytesToRead; b++) {
        byteData[bytesToRead-b-1] = spiTransfer(0x00);
    }

    setCS(false);

    return true;
}

bool LTC2983::writeRegister(uint16_t addr, void *data, int bytesToWrite)
{
    uint8_t *byteData = (uint8_t*) data;
    setCS();

    //send read instruction
    spiTransfer(SPI_WRITE);

    //send register address
    spiTransfer(addr >> 8);
    spiTransfer(addr & 0xFF);

    // write data bytes
    for (int b=0; b<bytesToWrite; b++) {
        spiTransfer( byteData[bytesToWrite-b-1] );
    }

    setCS(false);

    return true;
}
