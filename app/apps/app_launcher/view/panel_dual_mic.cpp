/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "view.h"
#include <cstdint>
#include <lvgl.h>
#include <hal/hal.h>
#include <memory>
#include <mooncake_log.h>
#include <smooth_ui_toolkit.h>
#include <smooth_lvgl.h>
#include <apps/utils/audio/audio.h>
#include <apps/utils/ui/window.h>

using namespace launcher_view;
using namespace smooth_ui_toolkit;
using namespace smooth_ui_toolkit::lvgl_cpp;

static const std::string _tag = "panel-dual-mic";

static const ui::Window::KeyFrame_t _kf_mic_test_close = {-154, -318, 75, 75, 0};
static const ui::Window::KeyFrame_t _kf_mic_test_open  = {-215, -217, 670, 171, 255};

class MicTestWindow : public ui::Window {
public:
    MicTestWindow()
    {
        config.kfClosed = _kf_mic_test_close;
        config.kfOpened = _kf_mic_test_open;
    }

    void onOpen() override
    {
        _window->setScrollbarMode(LV_SCROLLBAR_MODE_OFF);

        _chart_mic_left = std::make_unique<Chart>(_window->get());
        apply_chart_style(_chart_mic_left.get(), -189, 0);

        _chart_mic_right = std::make_unique<Chart>(_window->get());
        apply_chart_style(_chart_mic_right.get(), 80, 0);

        _rec_btn = std::make_unique<Button>(_window->get());
        _rec_btn->align(LV_ALIGN_CENTER, 267, 0);
        _rec_btn->setSize(72, 72);
        _rec_btn->setRadius(233);
        _rec_btn->setBorderWidth(4);
        _rec_btn->setBorderColor(lv_color_hex(0x383838));
        _rec_btn->setShadowWidth(0);
        _rec_btn->onClick().connect([&]() {
            GetHAL()->startDualMicRecordTest();
            update_rec_button();
        });

        _rec_btn->label().setTextFont(&lv_font_montserrat_16);
        _rec_btn->label().setTextColor(lv_color_hex(0xF4F3F3));
    }

    void onUpdate() override
    {
        if (GetHAL()->millis() - _time_count > 200) {
            update_rec_button();
            _time_count = GetHAL()->millis();
        }

        // Record and plot
        if (_chart_mic_left && _chart_mic_right) {
            GetHAL()->audioRecord(_record_data, 4);  // 48000 * 4 / 1000 = 192
            // [MIC-L, AEC, MIC-R, MIC-HP]
            for (int i = 0; i < _record_data.size(); i += 4) {
                _chart_mic_left->setNextValue(0, _record_data[i]);
                _chart_mic_right->setNextValue(0, _record_data[i + 2]);
            }
        }
    }

    void onClose() override
    {
        audio::play_next_tone_progression();
        _chart_mic_left.reset();
        _chart_mic_right.reset();
    }

private:
    uint32_t _time_count = 0;
    std::vector<int16_t> _record_data;
    std::unique_ptr<Chart> _chart_mic_left;
    std::unique_ptr<Chart> _chart_mic_right;
    std::unique_ptr<Button> _rec_btn;
    std::unique_ptr<Spinner> _rec_btn_spinner;

    void apply_chart_style(Chart* chart, int16_t x, int16_t y)
    {
        chart->align(LV_ALIGN_CENTER, x, y);
        chart->setBgColor(lv_color_hex(0x383838));
        chart->setRadius(12);
        chart->setSize(243, 120);
        chart->setStyleSize(0, 0, LV_PART_INDICATOR);
        chart->setPointCount(512);
        chart->setRange(LV_CHART_AXIS_PRIMARY_Y, -32768, 32767);
        chart->setUpdateMode(LV_CHART_UPDATE_MODE_SHIFT);
        chart->setBgOpa(LV_OPA_TRANSP, LV_PART_ITEMS);
        chart->setBorderWidth(0, static_cast<lv_style_selector_t>(LV_PART_MAIN) | static_cast<lv_style_selector_t>(LV_STATE_DEFAULT));
        chart->setDivLineCount(0, 0);
        chart->addSeries(lv_color_hex(0x40FFA1), LV_CHART_AXIS_PRIMARY_Y);
    }

    void update_rec_button()
    {
        if (GetHAL()->getDualMicRecordTestState() == hal::HalBase::MIC_TEST_RECORDING) {
            _rec_btn->setBgColor(lv_color_hex(0xE17822));
            _rec_btn->removeFlag(LV_OBJ_FLAG_CLICKABLE);
            _rec_btn->label().setText("REC");
            update_spinner(0xE17822);
        } else if (GetHAL()->getDualMicRecordTestState() == hal::HalBase::MIC_TEST_PLAYING) {
            _rec_btn->setBgColor(lv_color_hex(0x32CC72));
            _rec_btn->removeFlag(LV_OBJ_FLAG_CLICKABLE);
            _rec_btn->label().setText("PLAY");
            update_spinner(0x32CC72);
        } else {
            _rec_btn->setBgColor(lv_color_hex(0xD04848));
            _rec_btn->addFlag(LV_OBJ_FLAG_CLICKABLE);
            _rec_btn->label().setText("REC\n  3S");
            _rec_btn_spinner.reset();
        }
    }

    void update_spinner(uint32_t color)
    {
        if (!_rec_btn_spinner) {
            _rec_btn_spinner = std::make_unique<Spinner>(_window->get());
            _rec_btn_spinner->align(LV_ALIGN_CENTER, 267, 0);
            _rec_btn_spinner->setSize(76, 76);
            _rec_btn_spinner->setArcWidth(3, LV_PART_MAIN);
            _rec_btn_spinner->setArcWidth(3, LV_PART_INDICATOR);
            _rec_btn_spinner->setAnimParams(1000, 200);
        }
        _rec_btn_spinner->setArcColor(lv_color_hex(color), LV_PART_INDICATOR);
    }
};

void PanelDualMic::init()
{
    _btn_mic_test = std::make_unique<Container>(lv_screen_active());
    _btn_mic_test->align(LV_ALIGN_CENTER, -154, -311);
    _btn_mic_test->setSize(103, 95);
    _btn_mic_test->setOpa(0);
    _btn_mic_test->onClick().connect([&] {
        audio::play_next_tone_progression();

        // Create window
        _window = std::make_unique<MicTestWindow>();
        _window->init(lv_screen_active());
        _window->open();
    });
}

void PanelDualMic::update(bool isStacked)
{
    if (_window) {
        _window->update();
        if (_window->getState() == ui::Window::State_t::Closed) {
            _window.reset();
        }
    }
}
