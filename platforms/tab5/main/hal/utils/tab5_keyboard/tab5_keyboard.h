/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <driver/i2c_master.h>
#include <hal/keyboard.h>
#include <array>
#include <vector>

// The test window owns I2C1 while open. No background task or GPIO ISR remains
// after stop(), so Port A and GPIO tests can use their original configuration.
class Tab5Keyboard {
public:
    bool probe();
    bool start();
    void stop();
    hal::KeyboardStatus update(std::vector<hal::KeyboardEvent>& events, uint32_t now);
    bool setMode(hal::KeyboardMode mode);
    bool setLeds(uint32_t left, uint32_t right, uint8_t brightness);

private:
    i2c_master_bus_handle_t _bus    = nullptr;
    i2c_master_dev_handle_t _device = nullptr;
    hal::KeyboardStatus _status;
    hal::KeyboardMode _mode = hal::KeyboardMode::Keys;
    std::array<uint32_t, 2> _colors{};
    uint8_t _brightness = 20;
    uint32_t _lastProbe = 0;
    bool _probePending  = true;
    bool _saved         = false;
    std::array<uint8_t, 4> _settings{};
    std::array<uint8_t, 7> _rgb{};

    bool read(uint8_t reg, uint8_t* data, size_t size);
    bool write(uint8_t reg, const uint8_t* data, size_t size);
    bool writeByte(uint8_t reg, uint8_t value);
    bool connect();
    void disconnect();
    bool applyLeds();
};
