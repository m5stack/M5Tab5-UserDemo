/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <cstdint>
#include <string>

namespace hal {

enum class KeyboardMode : uint8_t { Keys = 0, Text = 2 };

struct KeyboardEvent {
    KeyboardMode mode = KeyboardMode::Keys;
    bool pressed      = false;
    uint8_t row       = 0;
    uint8_t column    = 0;
    uint8_t modifiers = 0;
    std::string text;
};

struct KeyboardStatus {
    bool available  = false;
    bool connected  = false;
    uint8_t version = 0;
};

}  // namespace hal
