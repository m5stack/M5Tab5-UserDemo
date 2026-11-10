/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "tab5_keyboard.h"
#include <driver/gpio.h>
#include <algorithm>
#include <cstring>

// Register layout and event encoding follow M5Tab5-Keyboard-Internal-FW:
// code/Keyboard_APP/Core/User/{i2c/user_i2c_callback.c,rgb/user_rgb.c}.
namespace {
constexpr uint8_t ADDRESS      = 0x6D;
constexpr uint8_t INT_CONFIG   = 0x00;
constexpr uint8_t INT_STATUS   = 0x01;
constexpr uint8_t EVENT_COUNT  = 0x02;
constexpr uint8_t BRIGHTNESS   = 0x03;
constexpr uint8_t MODE         = 0x10;
constexpr uint8_t RGB_MODE     = 0x11;
constexpr uint8_t KEY_EVENT    = 0x20;
constexpr uint8_t TEXT_LENGTH  = 0x40;
constexpr uint8_t TEXT_EVENT   = 0x50;
constexpr uint8_t RGB          = 0x60;
constexpr uint8_t VERSION      = 0xFE;
constexpr uint8_t SAVED_REGS[] = {INT_CONFIG, BRIGHTNESS, MODE, RGB_MODE};
constexpr int TIMEOUT_MS       = 10;
}  // namespace

bool Tab5Keyboard::read(uint8_t reg, uint8_t* data, size_t size)
{
    return _device && i2c_master_transmit_receive(_device, &reg, 1, data, size, TIMEOUT_MS) == ESP_OK;
}

bool Tab5Keyboard::write(uint8_t reg, const uint8_t* data, size_t size)
{
    uint8_t packet[8];
    if (!_device || size > sizeof(packet) - 1) {
        return false;
    }
    packet[0] = reg;
    memcpy(packet + 1, data, size);
    return i2c_master_transmit(_device, packet, size + 1, TIMEOUT_MS) == ESP_OK;
}

bool Tab5Keyboard::writeByte(uint8_t reg, uint8_t value)
{
    return write(reg, &value, 1);
}

bool Tab5Keyboard::probe()
{
    bool temporary = _bus == nullptr;
    if (temporary && !start()) {
        return false;
    }
    uint8_t version;
    // A homepage probe only reads the version, leaving modes, RGB and queued
    // keys untouched. Always give I2C1 back before another panel can open.
    bool found = i2c_master_probe(_bus, ADDRESS, TIMEOUT_MS) == ESP_OK && read(VERSION, &version, 1);
    if (temporary) {
        stop();
    }
    return found;
}

bool Tab5Keyboard::start()
{
    stop();
    _mode         = hal::KeyboardMode::Keys;
    _colors       = {};
    _brightness   = 20;
    _probePending = true;
    gpio_reset_pin(GPIO_NUM_0);
    gpio_reset_pin(GPIO_NUM_1);
    gpio_reset_pin(GPIO_NUM_50);

    i2c_master_bus_config_t config{};
    config.i2c_port                     = I2C_NUM_1;
    config.sda_io_num                   = GPIO_NUM_0;
    config.scl_io_num                   = GPIO_NUM_1;
    config.clk_source                   = I2C_CLK_SRC_DEFAULT;
    config.glitch_ignore_cnt            = 7;
    config.flags.enable_internal_pullup = true;
    if (i2c_new_master_bus(&config, &_bus) != ESP_OK) {
        _bus = nullptr;
        return false;
    }

    i2c_device_config_t device{};
    device.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    device.device_address  = ADDRESS;
    device.scl_speed_hz    = 400000;
    if (i2c_master_bus_add_device(_bus, &device, &_device) != ESP_OK) {
        _device = nullptr;
        stop();
        return false;
    }
    _status.available = true;
    return true;
}

void Tab5Keyboard::stop()
{
    if (_saved) {
        // Restore volatile settings, including the original RGB binding mode.
        // Never write the address or firmware-update registers.
        write(RGB, _rgb.data(), _rgb.size());
        for (size_t i = 0; i < _settings.size(); ++i) {
            writeByte(SAVED_REGS[i], _settings[i]);
        }
        writeByte(EVENT_COUNT, 0);
    }
    if (_device) {
        i2c_master_bus_rm_device(_device);
        _device = nullptr;
    }
    if (_bus) {
        i2c_del_master_bus(_bus);
        _bus = nullptr;
        gpio_reset_pin(GPIO_NUM_0);
        gpio_reset_pin(GPIO_NUM_1);
        gpio_reset_pin(GPIO_NUM_50);
    }
    _status = {};
    _saved  = false;
}

bool Tab5Keyboard::connect()
{
    if (!read(VERSION, &_status.version, 1)) {
        return false;
    }
    if (!_saved) {
        for (size_t i = 0; i < _settings.size(); ++i) {
            if (!read(SAVED_REGS[i], &_settings[i], 1)) {
                return false;
            }
        }
        if (!read(RGB, _rgb.data(), _rgb.size())) {
            return false;
        }
        _saved = true;
    }
    // Poll EVENT_COUNT directly; no interrupt task needs to own GPIO50.
    _status.connected = writeByte(INT_CONFIG, 0) && writeByte(MODE, static_cast<uint8_t>(_mode)) &&
                        writeByte(EVENT_COUNT, 0) && applyLeds();
    return _status.connected;
}

void Tab5Keyboard::disconnect()
{
    _status.connected = false;
    // Preserve the initial settings through transient failures and replugging.
}

bool Tab5Keyboard::setMode(hal::KeyboardMode mode)
{
    if (mode != hal::KeyboardMode::Keys && mode != hal::KeyboardMode::Text) {
        return false;
    }
    _mode = mode;
    if (!_status.connected) {
        return true;  // Apply the selected mode when a keyboard is attached.
    }
    if (!writeByte(MODE, static_cast<uint8_t>(mode)) || !writeByte(EVENT_COUNT, 0)) {
        disconnect();
        return false;
    }
    return true;
}

bool Tab5Keyboard::applyLeds()
{
    // Firmware uses a four-byte stride per LED: B,G,R,reserved.
    uint8_t data[7] = {static_cast<uint8_t>(_colors[0]),       static_cast<uint8_t>(_colors[0] >> 8),
                       static_cast<uint8_t>(_colors[0] >> 16), 0,
                       static_cast<uint8_t>(_colors[1]),       static_cast<uint8_t>(_colors[1] >> 8),
                       static_cast<uint8_t>(_colors[1] >> 16)};
    return writeByte(RGB_MODE, 1) && writeByte(BRIGHTNESS, _brightness) && write(RGB, data, sizeof(data));
}

bool Tab5Keyboard::setLeds(uint32_t left, uint32_t right, uint8_t brightness)
{
    _colors     = {left, right};
    _brightness = std::min<uint8_t>(brightness, 100);
    if (_status.connected && !applyLeds()) {
        disconnect();
        return false;
    }
    return true;
}

hal::KeyboardStatus Tab5Keyboard::update(std::vector<hal::KeyboardEvent>& events, uint32_t now)
{
    if (!_status.available) {
        return _status;
    }
    if (_probePending || now - _lastProbe >= 1000) {
        _probePending = false;
        _lastProbe    = now;
        if (!_status.connected) {
            connect();
        } else {
            uint8_t version;
            if (!read(VERSION, &version, 1) || version != _status.version) {
                disconnect();
            }
        }
    }
    if (!_status.connected) {
        return _status;
    }
    uint8_t count;
    if (!read(EVENT_COUNT, &count, 1)) {
        disconnect();
        return _status;
    }
    // Drain at most half the 32-event FIFO per frame to keep the UI responsive.
    count = std::min<uint8_t>(count, 16);
    for (uint8_t i = 0; i < count; ++i) {
        hal::KeyboardEvent event;
        event.mode = _mode;
        if (_mode == hal::KeyboardMode::Keys) {
            uint8_t raw;
            if (!read(KEY_EVENT, &raw, 1)) {
                disconnect();
                break;
            }
            event.pressed = (raw & 0x80) != 0;
            event.row     = (raw >> 4) & 0x07;
            event.column  = raw & 0x0F;
            if (raw == 0xFF || event.row >= 5 || event.column >= 14) {
                continue;
            }
        } else {
            uint8_t length;
            if (!read(TEXT_LENGTH, &length, 1)) {
                disconnect();
                break;
            }
            // The shipping STM32 firmware reports the TOTAL packet length,
            // including its modifier byte (user_i2c_callback.c).
            if (length < 2 || length > 10) {
                if (!writeByte(EVENT_COUNT, 0)) {
                    disconnect();
                }
                break;
            }
            uint8_t packet[10]{};
            if (!read(TEXT_EVENT, packet, length)) {
                disconnect();
                break;
            }
            event.modifiers = packet[0];
            event.text.assign(reinterpret_cast<const char*>(packet + 1), length - 1);
        }
        events.push_back(event);
    }
    if (_status.connected && !writeByte(INT_STATUS, 0)) {
        disconnect();
    }
    return _status;
}
