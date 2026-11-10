/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "../hal_esp32.h"
#include "../utils/tab5_keyboard/tab5_keyboard.h"

namespace {
Tab5Keyboard keyboard;
}

bool HalEsp32::probeKeyboard()
{
    return keyboard.probe();
}

void HalEsp32::startKeyboardTest()
{
    keyboard.start();
}

void HalEsp32::stopKeyboardTest()
{
    keyboard.stop();
}

hal::KeyboardStatus HalEsp32::updateKeyboardTest(std::vector<hal::KeyboardEvent>& events)
{
    return keyboard.update(events, millis());
}

bool HalEsp32::setKeyboardTestMode(hal::KeyboardMode mode)
{
    return keyboard.setMode(mode);
}

bool HalEsp32::setKeyboardTestLeds(uint32_t left, uint32_t right, uint8_t brightness)
{
    return keyboard.setLeds(left, right, brightness);
}
