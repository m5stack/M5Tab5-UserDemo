/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <hal/hal.h>
#include <ina226.hpp>
#include <lvgl.h>
#include "utils/rx8130/rx8130.h"

class HalEsp32 : public hal::HalBase {
public:
    std::string type() override
    {
        return "Tab5";
    }

    void init() override;

    void delay(uint32_t ms) override;
    uint32_t millis() override;
    int getCpuTemp() override;

    INA226 ina226;
    RX8130_Class rx8130;
    lv_disp_t* lvDisp      = nullptr;
    lv_indev_t* lvKeyboard = nullptr;

    void setDisplayBrightness(uint8_t brightness) override;
    uint8_t getDisplayBrightness() override;
    std::string getDisplayPanelIc() override;

    void lvglLock() override;
    void lvglUnlock() override;

    void updatePowerMonitorData() override;
    void updateImuData() override;
    void clearImuIrq() override;

    void clearRtcIrq() override;
    void setRtcTime(tm time) override;

    void setChargeQcEnable(bool enable) override;
    bool getChargeQcEnable() override;
    void setChargeEnable(bool enable) override;
    bool getChargeEnable() override;
    void setUsb5vEnable(bool enable) override;
    bool getUsb5vEnable() override;
    void setExt5vEnable(bool enable) override;
    bool getExt5vEnable() override;
    void powerOff() override;
    void sleepAndTouchWakeup() override;
    void sleepAndShakeWakeup() override;
    void sleepAndRtcWakeup() override;

    void startCameraCapture(lv_obj_t* imgCanvas) override;
    void stopCameraCapture() override;
    bool isCameraCapturing() override;

    void setSpeakerVolume(uint8_t volume) override;
    uint8_t getSpeakerVolume() override;
    void audioRecord(std::vector<int16_t>& data, uint16_t durationMs, float gain = 80.0f) override;
    void audioPlay(std::vector<int16_t>& data, bool async = true) override;
    void startDualMicRecordTest() override;
    MicTestState_t getDualMicRecordTestState() override;
    void startHeadphoneMicRecordTest() override;
    MicTestState_t getHeadphoneMicRecordTestState() override;
    void startPlayMusicTest() override;
    MusicPlayState_t getMusicPlayTestState() override;
    void stopPlayMusicTest() override;
    void playStartupSfx() override;
    void playShutdownSfx() override;

    void setExtAntennaEnable(bool enable) override;
    bool getExtAntennaEnable() override;
    void startWifiAp() override;

    bool isSdCardMounted() override;
    std::vector<FileEntry_t> scanSdCard(const std::string& dirPath) override;

    bool usbCDetect() override;
    bool usbADetect() override;
    bool headPhoneDetect() override;
    std::vector<uint8_t> i2cScan(bool isInternal) override;
    void initPortAI2c() override;
    void deinitPortAI2c() override;
    void gpioInitOutput(uint8_t pin) override;
    void gpioSetLevel(uint8_t pin, bool level) override;
    void gpioReset(uint8_t pin) override;

    bool probeKeyboard() override;
    void startKeyboardTest() override;
    void stopKeyboardTest() override;
    hal::KeyboardStatus updateKeyboardTest(std::vector<hal::KeyboardEvent>& events) override;
    bool setKeyboardTestMode(hal::KeyboardMode mode) override;
    bool setKeyboardTestLeds(uint32_t left, uint32_t right, uint8_t brightness) override;

private:
    void set_gpio_output_capability();
    void hid_init();
    void rs485_init();
    bool wifi_init();
    void imu_init();
    void update_system_time();

    uint8_t _current_lcd_brightness = 100;
    bool _charge_qc_enable          = false;
    bool _charge_enable             = true;
    bool _ext_5v_enable             = true;
    bool _usba_5v_enable            = true;
    bool _ext_antenna_enable        = false;
    bool _sd_card_mounted           = false;
};
