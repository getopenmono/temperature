//
//  ltc2983.hpp
//  xcode_project
//
//  Created by Kristoffer Andersen on 07/04/2016.
//  Copyright © 2016 your name. All rights reserved.
//

#ifndef ltc2983_h
#define ltc2983_h

extern "C" {
#include <project.h>
}

#include <temperature_interface.h>

class LTC2983 : public mono::sensor::ITemperature {

protected:

    static const int SpiChipSelectPin = CYREG_PRT3_PC4;
    bool inited;
    int measureChannel;

    enum SpiInstructions
    {
        SPI_READ = 0x03,
        SPI_WRITE = 0x02
    };

    /// MARK: BASIC SPI COMM
    void setCS(bool active = true);
    bool CS();

    uint8_t spiTransfer(uint8_t write);


    /// MARK: CHIP REGISTER INTERFACE COMM.

    bool readRegister(uint16_t addr, void *data, int bytesToRead = 1);

    bool writeRegister(uint16_t addr, void *data, int bytesToWrite = 1);

public:

    enum Registers
    {
        REG_CMD_STATUS = 0x00,
        REG_CH1 = 0x200,
        REG_CH1_RESULT = 0x010,
        REG_CH4 = 0x20C,
        REG_CH8 = 0x21C,
        REG_CH8_RESULT = 0x02C,
        REG_CH13 = 0x230,
        REG_CH13_RESULT = 0x040
    };

    LTC2983();

    /**
     * Reads the current temperature from the temperature sensor
     * @return the temperature in Celcius
     */
    int Read();

    /**
     * @brief Reads the temperature in fixed point milli-Celcius
     *
     * To get a higher precision output, this method will return milliCelcius
     * such that: 22,5 Celcius == 22500 mCelcius
     *
     * @return The temperature in mCelcius
     */
    int ReadMilliCelcius();

    /** Assign channels */
    void init();

    /** just wait 200ms, and check CMD STAT REGISTER */
    bool waitForStartup();

    bool assignChannel(Registers channel, uint32_t chData);

    bool startConversion(uint8_t channelNum);

    bool checkConversionDone();

    bool waitForConversionDone(int timeoutSecs = 1);

    bool readResult(uint8_t channelNum, int32_t *temperature);
};

#endif /* ltc2983_h */
