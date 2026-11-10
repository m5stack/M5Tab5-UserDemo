/*

The MIT License

Copyright (c) 2014-2023 Korneliusz Jarzębski

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include <math.h>
#include "ina226.hpp"

#define I2C_MASTER_TIMEOUT_MS 50  // 超时时间

bool INA226::begin(i2c_master_bus_handle_t bus_handle, uint8_t address)
{
    inaAddress = address;

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address  = address,
        .scl_speed_hz    = 400000,
        // IDF 6 added scl_wait_us and flags to this struct; 0 keeps the driver
        // default timeout and leaves ACK checking enabled.
        .scl_wait_us     = 0,
        .flags           = {},
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &i2c_dev_handle_ina226));

    if (i2c_dev_handle_ina226 == NULL) {
        return false;
    }

    return true;
}

bool INA226::configure(ina226_averages_t avg, ina226_busConvTime_t busConvTime, ina226_shuntConvTime_t shuntConvTime,
                       ina226_mode_t mode)
{
    uint16_t config = 0;

    config |= (avg << 9 | busConvTime << 6 | shuntConvTime << 3 | mode);

    vBusMax   = 36;
    vShuntMax = 0.08192f;

    writeRegister16(INA226_REG_CONFIG, config);

    return true;
}

bool INA226::calibrate(float rShuntValue, float iMaxExpected)
{
    uint16_t calibrationValue;
    rShunt = rShuntValue;

    // float iMaxPossible;
    float minimumLSB;

    // iMaxPossible = vShuntMax / rShunt;

    minimumLSB = iMaxExpected / 32767;

    currentLSB = (uint32_t)(minimumLSB * 100000000);
    currentLSB /= 100000000;
    currentLSB /= 0.0001;
    currentLSB = ceil(currentLSB);
    currentLSB *= 0.0001;

    powerLSB = currentLSB * 25;

    calibrationValue = (uint16_t)((0.00512) / (currentLSB * rShunt));

    writeRegister16(INA226_REG_CALIBRATION, calibrationValue);

    return true;
}

float INA226::getMaxPossibleCurrent(void)
{
    return (vShuntMax / rShunt);
}

float INA226::getMaxCurrent(void)
{
    float maxCurrent  = (currentLSB * 32767);
    float maxPossible = getMaxPossibleCurrent();

    if (maxCurrent > maxPossible) {
        return maxPossible;
    } else {
        return maxCurrent;
    }
}

float INA226::getMaxShuntVoltage(void)
{
    float maxVoltage = getMaxCurrent() * rShunt;

    if (maxVoltage >= vShuntMax) {
        return vShuntMax;
    } else {
        return maxVoltage;
    }
}

float INA226::getMaxPower(void)
{
    return (getMaxCurrent() * vBusMax);
}

float INA226::readBusPower(void)
{
    return (readRegister16(INA226_REG_POWER) * powerLSB);
}

float INA226::readShuntCurrent(void)
{
    return (readRegister16(INA226_REG_CURRENT) * currentLSB);
}

int16_t INA226::readRawShuntCurrent(void)
{
    return readRegister16(INA226_REG_CURRENT);
}

float INA226::readShuntVoltage(void)
{
    float voltage;

    voltage = readRegister16(INA226_REG_SHUNTVOLTAGE);

    return (voltage * 0.0000025);
}

float INA226::readBusVoltage(void)
{
    int16_t voltage;

    voltage = readRegister16(INA226_REG_BUSVOLTAGE);

    return (voltage * 0.00125);
}

ina226_averages_t INA226::getAverages(void)
{
    uint16_t value;

    value = readRegister16(INA226_REG_CONFIG);
    value &= 0b0000111000000000;
    value >>= 9;

    return (ina226_averages_t)value;
}

ina226_busConvTime_t INA226::getBusConversionTime(void)
{
    uint16_t value;

    value = readRegister16(INA226_REG_CONFIG);
    value &= 0b0000000111000000;
    value >>= 6;

    return (ina226_busConvTime_t)value;
}

ina226_shuntConvTime_t INA226::getShuntConversionTime(void)
{
    uint16_t value;

    value = readRegister16(INA226_REG_CONFIG);
    value &= 0b0000000000111000;
    value >>= 3;

    return (ina226_shuntConvTime_t)value;
}

ina226_mode_t INA226::getMode(void)
{
    uint16_t value;

    value = readRegister16(INA226_REG_CONFIG);
    value &= 0b0000000000000111;

    return (ina226_mode_t)value;
}

void INA226::setMaskEnable(uint16_t mask)
{
    writeRegister16(INA226_REG_MASKENABLE, mask);
}

uint16_t INA226::getMaskEnable(void)
{
    return readRegister16(INA226_REG_MASKENABLE);
}

void INA226::enableShuntOverLimitAlert(void)
{
    writeRegister16(INA226_REG_MASKENABLE, INA226_BIT_SOL);
}

void INA226::enableShuntUnderLimitAlert(void)
{
    writeRegister16(INA226_REG_MASKENABLE, INA226_BIT_SUL);
}

void INA226::enableBusOvertLimitAlert(void)
{
    writeRegister16(INA226_REG_MASKENABLE, INA226_BIT_BOL);
}

void INA226::enableBusUnderLimitAlert(void)
{
    writeRegister16(INA226_REG_MASKENABLE, INA226_BIT_BUL);
}

void INA226::enableOverPowerLimitAlert(void)
{
    writeRegister16(INA226_REG_MASKENABLE, INA226_BIT_POL);
}

void INA226::enableConversionReadyAlert(void)
{
    writeRegister16(INA226_REG_MASKENABLE, INA226_BIT_CNVR);
}

void INA226::disableAlerts(void)
{
    writeRegister16(INA226_REG_MASKENABLE, 0);
}

void INA226::setBusVoltageLimit(float voltage)
{
    uint16_t value = voltage / 0.00125;
    writeRegister16(INA226_REG_ALERTLIMIT, value);
}

void INA226::setShuntVoltageLimit(float voltage)
{
    uint16_t value = voltage / 0.0000025;
    writeRegister16(INA226_REG_ALERTLIMIT, value);
}

void INA226::setPowerLimit(float watts)
{
    uint16_t value = watts / powerLSB;
    writeRegister16(INA226_REG_ALERTLIMIT, value);
}

void INA226::setAlertInvertedPolarity(bool inverted)
{
    uint16_t temp = getMaskEnable();

    if (inverted) {
        temp |= INA226_BIT_APOL;
    } else {
        temp &= ~INA226_BIT_APOL;
    }

    setMaskEnable(temp);
}

void INA226::setAlertLatch(bool latch)
{
    uint16_t temp = getMaskEnable();

    if (latch) {
        temp |= INA226_BIT_LEN;
    } else {
        temp &= ~INA226_BIT_LEN;
    }

    setMaskEnable(temp);
}

bool INA226::isMathOverflow(void)
{
    return ((getMaskEnable() & INA226_BIT_OVF) == INA226_BIT_OVF);
}

bool INA226::isAlert(void)
{
    return ((getMaskEnable() & INA226_BIT_AFF) == INA226_BIT_AFF);
}

int16_t INA226::readRegister16(uint8_t reg)
{
    uint8_t r_buffer[2] = {0};
    i2c_master_transmit_receive(i2c_dev_handle_ina226, &reg, 1, r_buffer, 2, I2C_MASTER_TIMEOUT_MS);
    return r_buffer[0] << 8 | r_buffer[1];
}

void INA226::writeRegister16(uint8_t reg, uint16_t val)
{
    uint8_t w_buffer[3] = {0};
    w_buffer[0]         = reg;
    w_buffer[1]         = (val >> 8) & 0xFF;
    w_buffer[2]         = val & 0xFF;
    i2c_master_transmit(i2c_dev_handle_ina226, w_buffer, 3, I2C_MASTER_TIMEOUT_MS);
}
