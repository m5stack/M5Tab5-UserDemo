/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <cstdint>
#include <memory>
#include <lvgl.h>
#include <apps/utils/ui/window.h>
#include <apps/utils/ui/toast.h>
#include <smooth_ui_toolkit.h>
#include <smooth_lvgl.h>
#include <vector>

namespace launcher_view {

/**
 * @brief
 *
 */
class PanelBase {
public:
    virtual ~PanelBase()
    {
    }

    virtual void init()                 = 0;
    virtual void update(bool isStacked) = 0;
};

/**
 * @brief
 *
 */
class PanelRtc : public PanelBase {
public:
    void init() override;
    void update(bool isStacked) override;

private:
    uint32_t _time_count = 0;
    std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Label> _label_time;
    std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Label> _label_date;
    std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Container> _btn_rtc_setting;
    std::unique_ptr<ui::Window> _window;
};

/**
 * @brief
 *
 */
class PanelLcdBacklight : public PanelBase {
public:
    void init() override;
    void update(bool isStacked) override;

private:
    std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Label> _label_brightness;
    std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Label> _label_panel_ic;
    std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Container> _btn_up;
    std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Container> _btn_down;
    smooth_ui_toolkit::AnimateValue _label_y_anim;
};

/**
 * @brief
 *
 */
class PanelSpeakerVolume : public PanelBase {
public:
    void init() override;
    void update(bool isStacked) override;

private:
    std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Label> _label_volume;
    std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Container> _btn_up;
    std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Container> _btn_down;
    smooth_ui_toolkit::AnimateValue _label_y_anim;
};

/**
 * @brief
 *
 */
class PanelPowerMonitor : public PanelBase {
public:
    void init() override;
    void update(bool isStacked) override;

private:
    uint32_t _pm_data_update_time_count  = 0;
    uint32_t _cpu_temp_update_time_count = 0;
    std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Label> _label_voltage;
    std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Label> _label_current;
    std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Label> _label_cpu_temp;
    std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Image> _img_chg_arrow_up;
    std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Image> _img_chg_arrow_down;
};

/**
 * @brief
 *
 */
class PanelImu : public PanelBase {
public:
    void init() override;
    void update(bool isStacked) override;

private:
    uint32_t _time_count = 0;
    std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Label> _label_accel_x;
    std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Label> _label_accel_y;
    std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Label> _label_accel_z;
    std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Container> _accel_dot;
    smooth_ui_toolkit::AnimateValue _anim_x;
    smooth_ui_toolkit::AnimateValue _anim_y;
    smooth_ui_toolkit::AnimateValue _anim_size;
};

/**
 * @brief
 *
 */
class PanelSwitches : public PanelBase {
public:
    void init() override;
    void update(bool isStacked) override;

private:
    uint32_t _time_count = 0;
    std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Image> _img_charge_en_sw;
    std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Image> _img_charge_qc_en_sw;
    std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Image> _img_ext_5v_en_sw;
    std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Image> _img_usba_5v_en_sw;
    std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Image> _img_ext_antenna_en_sw;
    std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Container> _btn_charge_en_sw;
    std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Container> _btn_charge_qc_en_sw;
    std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Container> _btn_ext_5v_en_sw;
    std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Container> _btn_usba_5v_en_sw;
    std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Container> _btn_ext_antenna_en_sw;

    std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Image> _img_usb_c_detect;
    std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Image> _img_usb_a_detect;
    std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Image> _img_hp_detect;

    std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Container> _btn_ap_msg;
    std::unique_ptr<ui::Window> _window;

    bool _last_usb_c_detect = false;
    bool _last_usb_a_detect = false;
    bool _last_hp_detect    = false;

    void update_images();
    void update_detect_images();
    void play_sfx(bool isPlugedIn);
};

/**
 * @brief
 *
 */
class PanelPower : public PanelBase {
public:
    void init() override;
    void update(bool isStacked) override;

private:
    std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Container> _btn_power_off;
    std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Container> _btn_sleep_touch_wakeup;
    std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Container> _btn_sleep_shake_wakeup;
    std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Container> _btn_sleep_rtc_wakeup;
    std::unique_ptr<ui::Window> _window;
};

/**
 * @brief
 *
 */
class PanelCamera : public PanelBase {
public:
    void init() override;
    void update(bool isStacked) override;

private:
    std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Container> _btn_camera;
    std::unique_ptr<ui::Window> _window;
};

/**
 * @brief
 *
 */
class PanelDualMic : public PanelBase {
public:
    void init() override;
    void update(bool isStacked) override;

private:
    std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Container> _btn_mic_test;
    std::unique_ptr<ui::Window> _window;
};

/**
 * @brief
 *
 */
class PanelHeadphone : public PanelBase {
public:
    void init() override;
    void update(bool isStacked) override;

private:
    std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Container> _btn_headphone_test;
    std::unique_ptr<ui::Window> _window;
};

/**
 * @brief
 *
 */
class PanelSdCard : public PanelBase {
public:
    void init() override;
    void update(bool isStacked) override;

private:
    std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Container> _btn_sd_card_scan;
    std::unique_ptr<ui::Window> _window;
};

/**
 * @brief
 *
 */
class PanelI2cScan : public PanelBase {
public:
    void init() override;
    void update(bool isStacked) override;

private:
    std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Container> _btn_i2c_scan;
    std::unique_ptr<ui::Window> _window;
};

/**
 * @brief
 *
 */
class PanelGpioTest : public PanelBase {
public:
    void init() override;
    void update(bool isStacked) override;

private:
    std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Container> _btn_gpio_test;
    std::unique_ptr<ui::Window> _window;
};

/**
 * @brief
 *
 */
class PanelMusic : public PanelBase {
public:
    void init() override;
    void update(bool isStacked) override;

private:
    std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Container> _btn_music_test;
    std::unique_ptr<ui::Window> _window;
};

/**
 * @brief
 *
 */
class PanelComMonitor : public PanelBase {
public:
    void init() override;
    void update(bool isStacked) override;

private:
    std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Container> _btn_com_monitor;
    std::unique_ptr<ui::Window> _window;
};

/**
 * @brief
 *
 */
class PanelKeyboard : public PanelBase {
public:
    void init() override;
    void update(bool isStacked) override;

private:
    std::unique_ptr<ui::Toast> _notification;
    std::unique_ptr<ui::Window> _window;
    uint32_t _last_probe = 0;
    bool _home_active    = false;
    bool _notified       = false;
    bool _open_requested = false;
};

class LauncherView {
public:
    void init();
    void update();

private:
    bool _is_stacked = false;
    std::unique_ptr<smooth_ui_toolkit::lvgl_cpp::Image> _img_bg;
    std::vector<std::unique_ptr<PanelBase>> _panels;

    void update_anim();
};

}  // namespace launcher_view
