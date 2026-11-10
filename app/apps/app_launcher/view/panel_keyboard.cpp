/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "view.h"
#include <apps/utils/audio/audio.h>
#include <hal/hal.h>
#include <assets/assets.h>
#include <algorithm>
#include <array>
#include <cctype>
#include <cstdio>
#include <cmath>
#include <vector>

using namespace launcher_view;
using namespace smooth_ui_toolkit::lvgl_cpp;

namespace {
// Matrix order follows the official Tab5 Keyboard firmware key_value_map.
constexpr const char* KEY_LABELS[70] = {
    "Esc",  "1",   "2", "3", "4", "5", "6", "7", "8", "9", "0",    "-",    "+",     "Del",
    "`",    "!",   "@", "#", "$", "%", "^", "&", "*", "(", ")",    "[",    "]",     "\\",
    "Tab",  "Q",   "W", "E", "R", "T", "Y", "U", "I", "O", "P",    ";",    "'",     "BS",
    "Sym",  "Aa",  "A", "S", "D", "F", "G", "H", "J", "K", "L",    "Up",   "_",     "Enter",
    "Ctrl", "Alt", "Z", "X", "C", "V", "B", "N", "M", ".", "Left", "Down", "Right", "Space"};
constexpr uint32_t ACCENT     = 0x62D9AE;
constexpr int PICKER_SIZE     = 420;
constexpr int SHADE_SIZE      = 240;
constexpr int PICKER_X        = 48;
constexpr float PICKER_CENTER = (PICKER_SIZE - 1) * 0.5f;
constexpr float HUE_OUTER     = PICKER_SIZE * 0.5f - 4;
constexpr float HUE_INNER     = 174;
constexpr float HUE_RADIUS    = (HUE_OUTER + HUE_INNER) * 0.5f;
constexpr float PI            = 3.14159265f;

void lock_scrolling(lv_obj_t* obj)
{
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_CHAIN);
    lv_obj_set_scroll_dir(obj, LV_DIR_NONE);
    lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
}

lv_obj_t* label(lv_obj_t* parent, int x, int y, const char* text, const lv_font_t* font = &lv_font_montserrat_18)
{
    auto* obj = lv_label_create(parent);
    lv_obj_set_pos(obj, x, y);
    lv_obj_set_style_text_font(obj, font, 0);
    lv_obj_set_style_text_color(obj, lv_color_hex(0xDEDEDE), 0);
    lv_label_set_text(obj, text);
    return obj;
}

lv_obj_t* button(lv_obj_t* parent, int x, int y, int width, const char* text, lv_event_cb_t callback, void* data)
{
    auto* obj = lv_button_create(parent);
    lv_obj_set_pos(obj, x, y);
    lv_obj_set_size(obj, width, 44);
    lv_obj_set_style_radius(obj, 12, 0);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x414141), 0);
    lv_obj_set_style_shadow_width(obj, 0, 0);
    lock_scrolling(obj);
    auto* caption = label(obj, 0, 0, text);
    lv_obj_center(caption);
    lv_obj_add_event_cb(obj, callback, LV_EVENT_CLICKED, data);
    return obj;
}

class KeyboardTestWindow : public ui::Window {
public:
    KeyboardTestWindow()
    {
        config.title    = "Tab5 Keyboard";
        config.closeBtn = true;
        config.kfClosed = {370, 312, 180, 72, 0};
        config.kfOpened = {0, 0, 1120, 600, 255};
        _leds[1].hue    = 265;
    }

    ~KeyboardTestWindow() override
    {
        stop();
    }

    void onOpen() override
    {
        lock_scrolling(_window->get());
    }

    void onUpdate() override
    {
        // Like Camera, animate only the lightweight shell. Build controls and
        // start I2C after the opening animation has completely settled.
        if (_state != Opened) {
            return;
        }
        if (!_body) {
            create_content();
        }
        poll_keyboard();
    }

    void onClose() override
    {
        stop();
        _body.reset();
    }

private:
    struct LedControls {
        KeyboardTestWindow* owner = nullptr;
        size_t index              = 0;
        uint16_t hue              = 160;
        uint8_t saturation        = 65;
        uint8_t value             = 85;
        bool on                   = false;
        lv_obj_t* card            = nullptr;
        lv_obj_t* swatch          = nullptr;
        lv_obj_t* hex             = nullptr;
        lv_obj_t* power           = nullptr;

        uint32_t color() const
        {
            return lv_color_to_u32(lv_color_hsv_to_rgb(hue, saturation, value)) & 0xFFFFFF;
        }
    };

    std::vector<uint32_t> _huePixels;
    std::unique_ptr<Container> _body;
    std::array<lv_obj_t*, 70> _keys{};
    std::array<bool, 70> _pressed{};
    std::array<bool, 70> _tested{};
    std::array<LedControls, 2> _leds{};
    lv_obj_t* _keysButton      = nullptr;
    lv_obj_t* _textButton      = nullptr;
    lv_obj_t* _lightsButton    = nullptr;
    lv_obj_t* _clearButton     = nullptr;
    lv_obj_t* _grid            = nullptr;
    lv_obj_t* _text            = nullptr;
    lv_obj_t* _lighting        = nullptr;
    lv_obj_t* _hueRing         = nullptr;
    lv_obj_t* _hueThumb        = nullptr;
    lv_obj_t* _shade           = nullptr;
    lv_obj_t* _shadeThumb      = nullptr;
    lv_obj_t* _status          = nullptr;
    lv_obj_t* _lastEvent       = nullptr;
    lv_obj_t* _brightness      = nullptr;
    lv_obj_t* _brightnessLabel = nullptr;
    hal::KeyboardMode _mode    = hal::KeyboardMode::Keys;
    uint32_t _lastPoll         = 0;
    uint32_t _lastLedUpdate    = 0;
    size_t _activeLed          = 0;
    uint8_t _version           = 0;
    bool _running              = false;
    bool _connected            = false;
    bool _ledsDirty            = false;
    bool _showLights           = false;
    bool _hueDragging          = false;
    bool _shadeDragging        = false;

    void create_content()
    {
        _body = std::make_unique<Container>(_window->get());
        _body->setSize(1120, 600);
        _body->setPos(0, 0);
        _body->setPadding(0, 0, 0, 0);
        _body->setBorderWidth(0);
        _body->setBgOpa(LV_OPA_TRANSP);
        lock_scrolling(_body->get());
        auto* parent = _body->get();

        _status     = label(parent, 28, 51, "Connecting...");
        _keysButton = button(
            parent, 28, 88, 180, "Key test",
            [](lv_event_t* e) {
                static_cast<KeyboardTestWindow*>(lv_event_get_user_data(e))->selectMode(hal::KeyboardMode::Keys);
            },
            this);
        _textButton = button(
            parent, 220, 88, 180, "Text input",
            [](lv_event_t* e) {
                static_cast<KeyboardTestWindow*>(lv_event_get_user_data(e))->selectMode(hal::KeyboardMode::Text);
            },
            this);
        _lightsButton = button(
            parent, 412, 88, 180, "RGB lights",
            [](lv_event_t* e) {
                auto* self = static_cast<KeyboardTestWindow*>(lv_event_get_user_data(e));
                if (!self->_lighting) {
                    self->create_lighting(self->_body->get());
                }
                self->_showLights = true;
                self->update_tabs();
            },
            this);
        _clearButton = button(
            parent, 930, 88, 162, "Clear",
            [](lv_event_t* e) {
                auto* self = static_cast<KeyboardTestWindow*>(lv_event_get_user_data(e));
                self->clearKeys();
                lv_textarea_set_text(self->_text, "");
                lv_label_set_text(self->_lastEvent, "");
            },
            this);

        _grid = lv_obj_create(parent);
        lv_obj_set_pos(_grid, 28, 150);
        lv_obj_set_size(_grid, 1064, 380);
        lv_obj_set_style_pad_all(_grid, 0, 0);
        lv_obj_set_style_border_width(_grid, 0, 0);
        lv_obj_set_style_bg_opa(_grid, LV_OPA_TRANSP, 0);
        lock_scrolling(_grid);
        for (size_t i = 0; i < _keys.size(); ++i) {
            auto* key = lv_obj_create(_grid);
            _keys[i]  = key;
            lv_obj_set_pos(key, (i % 14) * 76, (i / 14) * 76);
            lv_obj_set_size(key, 70, 68);
            lv_obj_set_style_radius(key, 8, 0);
            lv_obj_set_style_pad_all(key, 0, 0);
            lv_obj_set_style_border_width(key, 1, 0);
            lv_obj_remove_flag(key, LV_OBJ_FLAG_CLICKABLE);
            lock_scrolling(key);
            auto* caption = label(key, 0, 0, KEY_LABELS[i], &lv_font_montserrat_18);
            lv_obj_center(caption);
            paintKey(i);
        }

        _text = lv_textarea_create(parent);
        lv_obj_set_pos(_text, 28, 150);
        lv_obj_set_size(_text, 1064, 380);
        lv_obj_set_style_bg_color(_text, lv_color_hex(0x303030), 0);
        lv_obj_set_style_text_color(_text, lv_color_hex(0xEEEEEE), 0);
        lv_obj_set_style_text_font(_text, &lv_font_montserrat_24, 0);
        lv_obj_set_style_border_color(_text, lv_color_hex(ACCENT), 0);
        lv_obj_set_style_radius(_text, 12, 0);
        lv_textarea_set_max_length(_text, 4096);
        lv_textarea_set_text(_text, "");
        lv_textarea_set_placeholder_text(_text, "Type here");
        lock_scrolling(_text);

        _lastEvent = label(parent, 28, 548, "");
        lv_obj_set_width(_lastEvent, 1060);
        lv_label_set_long_mode(_lastEvent, LV_LABEL_LONG_DOT);

        GetHAL()->startKeyboardTest();
        _running = true;
        selectMode(hal::KeyboardMode::Keys);
    }

    void poll_keyboard()
    {
        if (!_running) {
            return;
        }
        auto now = GetHAL()->millis();
        if (now - _lastPoll < 10) {
            return;
        }
        _lastPoll = now;
        if (_ledsDirty && now - _lastLedUpdate >= 33) {
            GetHAL()->setKeyboardTestLeds(_leds[0].on ? _leds[0].color() : 0, _leds[1].on ? _leds[1].color() : 0,
                                          (_brightness ? lv_slider_get_value(_brightness) : 20));
            _lastLedUpdate = now;
            _ledsDirty     = false;
        }
        std::vector<hal::KeyboardEvent> events;
        auto status = GetHAL()->updateKeyboardTest(events);
        if (!status.connected) {
            if (_connected) {
                clearKeys();
            }
            lv_label_set_text(_status, status.available ? "Disconnected" : "Unavailable");
        } else {
            _version = status.version;
            for (const auto& event : events) {
                if (_showLights) {
                    continue;
                }
                if (event.mode == hal::KeyboardMode::Keys && _mode == event.mode) {
                    size_t index = event.row * 14 + event.column;
                    if (index < _keys.size()) {
                        _pressed[index] = event.pressed;
                        _tested[index]  = _tested[index] || event.pressed;
                        paintKey(index);
                        lv_label_set_text_fmt(_lastEvent, "%s: %s    Row: %u  Column: %u",
                                              event.pressed ? "Pressed" : "Released", KEY_LABELS[index], event.row,
                                              event.column);
                    }
                } else if (event.mode == hal::KeyboardMode::Text && _mode == event.mode) {
                    inputText(event);
                }
            }
            updateStatus();
        }
        _connected = status.connected;
    }

    void stop()
    {
        if (_running) {
            GetHAL()->stopKeyboardTest();
            _running = false;
        }
    }

    void paintKey(size_t i)
    {
        lv_obj_set_style_bg_color(_keys[i], lv_color_hex(_pressed[i] ? 0x28865E : (_tested[i] ? 0x29483D : 0x3B3B3B)),
                                  0);
        lv_obj_set_style_border_color(_keys[i], lv_color_hex(_tested[i] ? ACCENT : 0x555555), 0);
    }

    void clearKeys()
    {
        _pressed.fill(false);
        _tested.fill(false);
        for (size_t i = 0; i < _keys.size(); ++i) {
            paintKey(i);
        }
    }

    void selectMode(hal::KeyboardMode mode)
    {
        GetHAL()->setKeyboardTestMode(mode);
        _mode       = mode;
        _showLights = false;
        clearKeys();
        update_tabs();
    }

    void update_tabs()
    {
        bool keys = !_showLights && _mode == hal::KeyboardMode::Keys;
        bool text = !_showLights && _mode == hal::KeyboardMode::Text;
        lv_obj_set_style_bg_color(_keysButton, lv_color_hex(keys ? 0x28865E : 0x414141), 0);
        lv_obj_set_style_bg_color(_textButton, lv_color_hex(text ? 0x28865E : 0x414141), 0);
        lv_obj_set_style_bg_color(_lightsButton, lv_color_hex(_showLights ? 0x28865E : 0x414141), 0);
        lv_obj_add_flag(_grid, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(_text, LV_OBJ_FLAG_HIDDEN);
        if (_lighting) {
            lv_obj_add_flag(_lighting, LV_OBJ_FLAG_HIDDEN);
        }
        if (keys) {
            lv_obj_remove_flag(_grid, LV_OBJ_FLAG_HIDDEN);
        } else if (text) {
            lv_obj_remove_flag(_text, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_state(_text, LV_STATE_FOCUSED);
        } else {
            lv_obj_remove_flag(_lighting, LV_OBJ_FLAG_HIDDEN);
        }
        if (_showLights) {
            lv_obj_add_flag(_clearButton, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_remove_flag(_clearButton, LV_OBJ_FLAG_HIDDEN);
        }
        lv_label_set_text(_lastEvent, "");
        if (_showLights) {
            lv_obj_add_flag(_lastEvent, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_remove_flag(_lastEvent, LV_OBJ_FLAG_HIDDEN);
        }
        if (_connected) {
            updateStatus();
        }
    }

    void updateStatus()
    {
        if (!_showLights && _mode == hal::KeyboardMode::Keys) {
            lv_label_set_text_fmt(_status, "Connected    Firmware: 0x%02X    Tested: %u / 70    Held: %u", _version,
                                  (unsigned)std::count(_tested.begin(), _tested.end(), true),
                                  (unsigned)std::count(_pressed.begin(), _pressed.end(), true));
        } else {
            lv_label_set_text_fmt(_status, "Connected    Firmware: 0x%02X", _version);
        }
    }

    lv_obj_t* create_thumb(lv_obj_t* parent)
    {
        auto* thumb = lv_obj_create(parent);
        lv_obj_set_size(thumb, 20, 20);
        lv_obj_set_style_radius(thumb, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_border_width(thumb, 3, 0);
        lv_obj_set_style_border_color(thumb, lv_color_white(), 0);
        lv_obj_set_style_shadow_color(thumb, lv_color_black(), 0);
        lv_obj_set_style_shadow_width(thumb, 4, 0);
        lv_obj_set_style_shadow_opa(thumb, LV_OPA_50, 0);
        lv_obj_remove_flag(thumb, LV_OBJ_FLAG_CLICKABLE);
        lock_scrolling(thumb);
        return thumb;
    }

    void create_lighting(lv_obj_t* parent)
    {
        _lighting = lv_obj_create(parent);
        lv_obj_set_pos(_lighting, 28, 150);
        lv_obj_set_size(_lighting, 1064, PICKER_SIZE);
        lv_obj_set_style_pad_all(_lighting, 0, 0);
        lv_obj_set_style_border_width(_lighting, 0, 0);
        lv_obj_set_style_bg_opa(_lighting, LV_OPA_TRANSP, 0);
        lock_scrolling(_lighting);

        // Rasterize the hue ring once, after the window has opened. The shade
        // square uses native gradients, so dragging never rebuilds a bitmap.
        size_t stride = lv_draw_buf_width_to_stride(PICKER_SIZE, LV_COLOR_FORMAT_ARGB8888) / sizeof(uint32_t);
        _huePixels.assign(stride * PICKER_SIZE, 0);
        for (int y = 0; y < PICKER_SIZE; ++y) {
            for (int x = 0; x < PICKER_SIZE; ++x) {
                float dx = x - PICKER_CENTER, dy = y - PICKER_CENTER;
                float radius   = std::sqrt(dx * dx + dy * dy);
                float coverage = std::clamp(std::min(radius - HUE_INNER, HUE_OUTER - radius) + 0.5f, 0.0f, 1.0f);
                if (coverage == 0) {
                    continue;
                }
                float hue = std::atan2(dy, dx) * 180.0f / PI;
                if (hue < 0) {
                    hue += 360;
                }
                uint32_t rgb               = lv_color_to_u32(lv_color_hsv_to_rgb(static_cast<uint16_t>(hue), 100, 100));
                _huePixels[y * stride + x] = (static_cast<uint32_t>(coverage * 255) << 24) | (rgb & 0xFFFFFF);
            }
        }
        _hueRing = lv_canvas_create(_lighting);
        lv_canvas_set_buffer(_hueRing, _huePixels.data(), PICKER_SIZE, PICKER_SIZE, LV_COLOR_FORMAT_ARGB8888);
        lv_obj_set_pos(_hueRing, PICKER_X, 0);
        lv_obj_add_flag(_hueRing, LV_OBJ_FLAG_CLICKABLE);
        lock_scrolling(_hueRing);
        _hueThumb = create_thumb(_hueRing);

        _shade = lv_obj_create(_lighting);
        lv_obj_set_pos(_shade, PICKER_X + (PICKER_SIZE - SHADE_SIZE) / 2, (PICKER_SIZE - SHADE_SIZE) / 2);
        lv_obj_set_size(_shade, SHADE_SIZE, SHADE_SIZE);
        lv_obj_set_style_pad_all(_shade, 0, 0);
        lv_obj_set_style_border_width(_shade, 0, 0);
        lv_obj_set_style_radius(_shade, 8, 0);
        lv_obj_set_style_clip_corner(_shade, true, 0);
        lv_obj_set_style_bg_color(_shade, lv_color_white(), 0);
        lv_obj_set_style_bg_grad_dir(_shade, LV_GRAD_DIR_HOR, 0);
        lock_scrolling(_shade);
        auto* shadow = lv_obj_create(_shade);
        lv_obj_set_pos(shadow, 0, 0);
        lv_obj_set_size(shadow, SHADE_SIZE, SHADE_SIZE);
        lv_obj_set_style_border_width(shadow, 0, 0);
        lv_obj_set_style_radius(shadow, 8, 0);
        lv_obj_set_style_bg_color(shadow, lv_color_black(), 0);
        lv_obj_set_style_bg_grad_color(shadow, lv_color_black(), 0);
        lv_obj_set_style_bg_grad_dir(shadow, LV_GRAD_DIR_VER, 0);
        lv_obj_set_style_bg_main_opa(shadow, LV_OPA_TRANSP, 0);
        lv_obj_set_style_bg_grad_opa(shadow, LV_OPA_COVER, 0);
        lv_obj_remove_flag(shadow, LV_OBJ_FLAG_CLICKABLE);
        lock_scrolling(shadow);
        _shadeThumb = create_thumb(_shade);
        for (auto* surface : {_hueRing, _shade}) {
            lv_obj_add_event_cb(
                surface,
                [](lv_event_t* e) { static_cast<KeyboardTestWindow*>(lv_event_get_user_data(e))->picker_touch(e); },
                LV_EVENT_ALL, this);
        }

        for (size_t i = 0; i < _leds.size(); ++i) {
            auto& led = _leds[i];
            led.owner = this;
            led.index = i;
            led.card  = lv_obj_create(_lighting);
            lv_obj_set_pos(led.card, 576, 72 + i * 112);
            lv_obj_set_size(led.card, 432, 88);
            lv_obj_set_style_pad_all(led.card, 0, 0);
            lv_obj_set_style_bg_color(led.card, lv_color_hex(0x303030), 0);
            lv_obj_set_style_radius(led.card, 18, 0);
            lock_scrolling(led.card);
            lv_obj_add_event_cb(
                led.card,
                [](lv_event_t* e) {
                    auto& led             = *static_cast<LedControls*>(lv_event_get_user_data(e));
                    led.owner->_activeLed = led.index;
                    led.owner->refresh_leds();
                    led.owner->refresh_picker();
                },
                LV_EVENT_CLICKED, &led);
            label(led.card, 84, 12, i == 0 ? "LED 1" : "LED 2", &lv_font_montserrat_16);
            led.swatch = lv_obj_create(led.card);
            lv_obj_set_pos(led.swatch, 20, 22);
            lv_obj_set_size(led.swatch, 44, 44);
            lv_obj_set_style_radius(led.swatch, LV_RADIUS_CIRCLE, 0);
            lv_obj_set_style_border_width(led.swatch, 0, 0);
            lv_obj_remove_flag(led.swatch, LV_OBJ_FLAG_CLICKABLE);
            lock_scrolling(led.swatch);
            led.hex   = label(led.card, 84, 38, "", &lv_font_montserrat_24);
            led.power = button(
                led.card, 334, 22, 76, "Off",
                [](lv_event_t* e) {
                    auto& led             = *static_cast<LedControls*>(lv_event_get_user_data(e));
                    led.on                = !led.on;
                    led.owner->_ledsDirty = true;
                    led.owner->refresh_leds();
                },
                &led);
        }
        label(_lighting, 576, 316, "Brightness");
        _brightnessLabel = label(_lighting, 948, 316, "20%");
        lv_obj_set_width(_brightnessLabel, 60);
        lv_obj_set_style_text_align(_brightnessLabel, LV_TEXT_ALIGN_RIGHT, 0);
        _brightness = lv_slider_create(_lighting);
        lv_obj_set_pos(_brightness, 576, 354);
        lv_obj_set_size(_brightness, 432, 8);
        lv_obj_set_ext_click_area(_brightness, 16);
        lock_scrolling(_brightness);
        lv_slider_set_range(_brightness, 0, 100);
        lv_slider_set_value(_brightness, 20, LV_ANIM_OFF);
        lv_obj_set_style_bg_color(_brightness, lv_color_hex(ACCENT), LV_PART_INDICATOR);
        lv_obj_set_style_bg_color(_brightness, lv_color_hex(ACCENT), LV_PART_KNOB);
        lv_obj_set_style_pad_all(_brightness, 6, LV_PART_KNOB);
        lv_obj_add_event_cb(
            _brightness,
            [](lv_event_t* e) {
                auto* self = static_cast<KeyboardTestWindow*>(lv_event_get_user_data(e));
                lv_label_set_text_fmt(self->_brightnessLabel, "%d%%", (int)lv_slider_get_value(self->_brightness));
                self->_ledsDirty = true;
            },
            LV_EVENT_VALUE_CHANGED, this);
        refresh_leds();
        refresh_picker();
    }

    void refresh_leds()
    {
        for (auto& led : _leds) {
            lv_obj_set_style_border_width(led.card, led.index == _activeLed ? 2 : 1, 0);
            lv_obj_set_style_border_color(led.card, lv_color_hex(led.index == _activeLed ? ACCENT : 0x505050), 0);
            lv_obj_set_style_bg_color(led.swatch, lv_color_hex(led.color()), 0);
            lv_label_set_text_fmt(led.hex, "#%06lX", static_cast<unsigned long>(led.color()));
            lv_label_set_text(lv_obj_get_child(led.power, 0), led.on ? "On" : "Off");
            lv_obj_set_style_bg_color(led.power, lv_color_hex(led.on ? 0x28865E : 0x414141), 0);
        }
    }

    void refresh_picker()
    {
        auto& led = _leds[_activeLed];
        lv_obj_set_style_bg_grad_color(_shade, lv_color_hsv_to_rgb(led.hue, 100, 100), 0);
        float angle = led.hue * PI / 180.0f;
        lv_obj_set_pos(_hueThumb, std::lround(PICKER_CENTER + HUE_RADIUS * std::cos(angle)) - 10,
                       std::lround(PICKER_CENTER + HUE_RADIUS * std::sin(angle)) - 10);
        lv_obj_set_style_bg_color(_hueThumb, lv_color_hsv_to_rgb(led.hue, 100, 100), 0);
        int x = std::clamp<int>(led.saturation * (SHADE_SIZE - 1) / 100, 10, SHADE_SIZE - 11);
        int y = std::clamp<int>((100 - led.value) * (SHADE_SIZE - 1) / 100, 10, SHADE_SIZE - 11);
        lv_obj_set_pos(_shadeThumb, x - 10, y - 10);
        lv_obj_set_style_bg_color(_shadeThumb, lv_color_hex(led.color()), 0);
    }

    void picker_touch(lv_event_t* e)
    {
        auto code      = lv_event_get_code(e);
        auto* surface  = static_cast<lv_obj_t*>(lv_event_get_current_target(e));
        bool hue       = surface == _hueRing;
        bool& dragging = hue ? _hueDragging : _shadeDragging;
        if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
            dragging = false;
            return;
        }
        if (code != LV_EVENT_PRESSED && code != LV_EVENT_PRESSING) {
            return;
        }
        auto* indev = lv_event_get_indev(e);
        if (!indev) {
            return;
        }
        lv_point_t point;
        lv_area_t area;
        lv_indev_get_point(indev, &point);
        lv_obj_get_coords(surface, &area);
        float x = point.x - area.x1, y = point.y - area.y1;
        if (code == LV_EVENT_PRESSED) {
            float dx = x - PICKER_CENTER, dy = y - PICKER_CENTER;
            dragging = !hue || std::abs(std::sqrt(dx * dx + dy * dy) - HUE_RADIUS) <= 25;
        }
        if (!dragging) {
            return;
        }
        auto& led = _leds[_activeLed];
        if (hue) {
            float angle = std::atan2(y - PICKER_CENTER, x - PICKER_CENTER) * 180.0f / PI;
            if (angle < 0) {
                angle += 360;
            }
            led.hue = static_cast<uint16_t>(angle);
        } else {
            led.saturation = std::clamp<int>(x, 0, SHADE_SIZE - 1) * 100 / (SHADE_SIZE - 1);
            led.value      = 100 - std::clamp<int>(y, 0, SHADE_SIZE - 1) * 100 / (SHADE_SIZE - 1);
        }
        led.on     = true;
        _ledsDirty = true;
        refresh_picker();
        refresh_leds();
    }

    void inputText(const hal::KeyboardEvent& event)
    {
        const auto& text = event.text;
        lv_label_set_text_fmt(_lastEvent, "%s%s%s", event.modifiers & 1 ? "Ctrl + " : "",
                              event.modifiers & 4 ? "Alt + " : "", text.c_str());
        if (event.modifiers != 0 || text.empty()) {
            return;
        }
        std::string name = text;
        std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c) { return std::tolower(c); });
        if (name == "backspace" || name == "\b") {
            lv_textarea_delete_char(_text);
        } else if (name == "del" || name == "\x7f") {
            lv_textarea_delete_char_forward(_text);
        } else if (name == "enter" || name == "\r" || name == "\n") {
            lv_textarea_add_text(_text, "\n");
        } else if (name == "tab" || name == "\t") {
            lv_textarea_add_text(_text, "    ");
        } else if (name == "left") {
            lv_textarea_cursor_left(_text);
        } else if (name == "right") {
            lv_textarea_cursor_right(_text);
        } else if (name == "up") {
            lv_textarea_cursor_up(_text);
        } else if (name == "down") {
            lv_textarea_cursor_down(_text);
        } else if (name != "esc" && name != "sym" && name != "aa" && name != "ctrl" && name != "alt") {
            lv_textarea_add_text(_text, text.c_str());
        }
    }
};
}  // namespace

void PanelKeyboard::init()
{
    // The homepage has no permanent keyboard button. Detection runs below only
    // while no test window owns the extension pins / I2C1 controller.
}

void PanelKeyboard::update(bool isStacked)
{
    if (_window) {
        _window->update();
        if (_window->getState() == ui::Window::Closed) {
            _window.reset();
        }
        _home_active = false;
        return;
    }

    if (isStacked) {
        _notification.reset();
        _open_requested = false;
        _home_active    = false;
        return;
    }

    if (_open_requested) {
        _open_requested = false;
        _notification.reset();
        _home_active = false;
        audio::play_next_tone_progression();
        _window = std::make_unique<KeyboardTestWindow>();
        _window->init(lv_screen_active());
        _window->open();
        return;
    }

    auto now = GetHAL()->millis();
    if (!_home_active) {
        _home_active = true;
        _notified    = false;
        _last_probe  = now - 1000;
    }
    if (now - _last_probe >= 1000) {
        _last_probe = now;
        if (!GetHAL()->probeKeyboard()) {
            _notification.reset();
            _notified = false;
        } else if (!_notified) {
            _notified          = true;
            _notification      = std::make_unique<ui::Toast>();
            auto& config       = _notification->config;
            config.msg         = "Keyboard";
            config.detail      = "Tap to test";
            config.icon        = &material_keyboard_filled;
            config.iconColor   = ACCENT;
            config.borderColor = ACCENT;
            config.borderWidth = 2;
            config.durationMs  = 0;
            config.align       = LV_ALIGN_BOTTOM_RIGHT;
            // x=920..1100, y=636..708 on the 1280x720 homepage: between the
            // Music Test and volume buttons, below the IMU display.
            config.x       = -180;
            config.y       = -12;
            config.width   = 180;
            config.height  = 72;
            config.font    = &lv_font_montserrat_18;
            config.onClick = [this] { _open_requested = true; };
            _notification->init(lv_screen_active());
            _notification->open();
        }
    }
    if (_notification) {
        _notification->update();
        if (_notification->getState() == ui::Toast::Closed) {
            _notification.reset();
        }
    }
}
