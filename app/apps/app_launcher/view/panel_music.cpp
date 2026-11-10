/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "lvgl_cpp/obj.h"
#include "view.h"
#include <lvgl.h>
#include <hal/hal.h>
#include <mooncake_log.h>
#include <smooth_ui_toolkit.h>
#include <smooth_lvgl.h>
#include <apps/utils/audio/audio.h>
#include <apps/utils/ui/window.h>
#include <apps/utils/ui/toast.h>

using namespace launcher_view;
using namespace smooth_ui_toolkit;
using namespace smooth_ui_toolkit::lvgl_cpp;

static const std::string _tag = "panel-music";

static const ui::Window::KeyFrame_t _kf_music_test_close = {225, 279, 75, 75, 0};
static const ui::Window::KeyFrame_t _kf_music_test_open  = {236, 116, 413, 204, 255};

class MusicTestWindow : public ui::Window {
public:
    MusicTestWindow()
    {
        config.kfClosed = _kf_music_test_close;
        config.kfOpened = _kf_music_test_open;
    }

    void onOpen() override
    {
        _window->setScrollbarMode(LV_SCROLLBAR_MODE_OFF);

        _panel_aec = std::make_unique<Container>(_window->get());
        _panel_aec->align(LV_ALIGN_CENTER, -60, 0);
        _panel_aec->setSize(243, 153);
        _panel_aec->setBgColor(lv_color_hex(0x5A5A5A));
        _panel_aec->setBorderWidth(0);
        _panel_aec->setRadius(12);

        _label_aec = std::make_unique<Label>(_window->get());
        _label_aec->align(LV_ALIGN_CENTER, -60, 59);
        _label_aec->setText("Speaker Output Capture");
        _label_aec->setTextColor(lv_color_hex(0xD6D6D6));
        _label_aec->setTextFont(&lv_font_montserrat_16);

        _chart_aec = std::make_unique<Chart>(_window->get());
        apply_chart_style(_chart_aec.get(), -60, -17);

        _rec_btn = std::make_unique<Button>(_window->get());
        _rec_btn->align(LV_ALIGN_CENTER, 135, 0);
        _rec_btn->setSize(88, 88);
        _rec_btn->setRadius(233);
        _rec_btn->setBorderWidth(6);
        _rec_btn->setBgColor(lv_color_hex(0x31D584));
        _rec_btn->setBorderColor(lv_color_hex(0x383838));
        _rec_btn->setShadowWidth(0);
        _rec_btn->onClick().connect([&]() {
            if (GetHAL()->getMusicPlayTestState() == hal::HalBase::MUSIC_PLAY_IDLE) {
                GetHAL()->startPlayMusicTest();
            } else {
                GetHAL()->stopPlayMusicTest();
            }
            update_rec_button();
        });

        _rec_btn->label().setTextFont(&lv_font_montserrat_18);
        _rec_btn->label().setTextColor(lv_color_hex(0x0B4D2C));

        update_rec_button();
    }

    void onUpdate() override
    {
        // Record and plot
        if (_chart_aec) {
            GetHAL()->audioRecord(_record_data, 4, 20);  // 48000 * 4 / 1000 = 192
            // [MIC-L, AEC, MIC-R, MIC-HP]
            for (int i = 0; i < _record_data.size(); i += 4) {
                _chart_aec->setNextValue(0, _record_data[i + 1]);
            }
        }

        if (_state != Opened) {
            return;
        }

        if (GetHAL()->millis() - _time_count > 200) {
            update_rec_button();
            _time_count = GetHAL()->millis();
        }
    }

    void onClose() override
    {
        GetHAL()->stopPlayMusicTest();
        _rec_btn.reset();
        _rec_btn_spinner.reset();
        _chart_aec.reset();
    }

private:
    uint32_t _time_count = 0;
    std::vector<int16_t> _record_data;
    std::unique_ptr<Button> _rec_btn;
    std::unique_ptr<Spinner> _rec_btn_spinner;
    std::unique_ptr<Chart> _chart_aec;
    std::unique_ptr<Container> _panel_aec;
    std::unique_ptr<Label> _label_aec;

    void update_rec_button()
    {
        if (GetHAL()->getMusicPlayTestState() == hal::HalBase::MUSIC_PLAY_PLAYING) {
            _rec_btn->label().setText("STOP");
            update_spinner(0x31D584);
        } else {
            _rec_btn->label().setText(" PLAY\nMUSIC");
            _rec_btn_spinner.reset();
        }
    }

    void update_spinner(uint32_t color)
    {
        if (!_rec_btn_spinner) {
            _rec_btn_spinner = std::make_unique<Spinner>(_window->get());
            _rec_btn_spinner->align(LV_ALIGN_CENTER, 135, 0);
            _rec_btn_spinner->setSize(90, 90);
            _rec_btn_spinner->setArcWidth(4, LV_PART_MAIN);
            _rec_btn_spinner->setArcWidth(4, LV_PART_INDICATOR);
            _rec_btn_spinner->setAnimParams(1000, 200);
        }
        _rec_btn_spinner->setArcColor(lv_color_hex(color), LV_PART_INDICATOR);
    }

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
};

void PanelMusic::init()
{
    _btn_music_test = std::make_unique<Container>(lv_screen_active());
    _btn_music_test->align(LV_ALIGN_CENTER, 225, 279);
    _btn_music_test->setSize(100, 100);
    _btn_music_test->setOpa(0);
    _btn_music_test->onClick().connect([&] {
        audio::play_next_tone_progression();

        // Create window
        _window = std::make_unique<MusicTestWindow>();
        _window->init(lv_screen_active());
        _window->open();
    });
}

void PanelMusic::update(bool isStacked)
{
    if (_window) {
        _window->update();
        if (_window->getState() == ui::Window::State_t::Closed) {
            _window.reset();
        }
    }
}
