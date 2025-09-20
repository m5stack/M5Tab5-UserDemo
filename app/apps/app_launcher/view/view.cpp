/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "view.h"

#include <apps/utils/audio/audio.h>
#include <assets/assets.h>
#include <hal/hal.h>
#include <lvgl.h>
#include <mooncake_log.h>
#include <smooth_lvgl.h>
#include <smooth_ui_toolkit.h>

#include "integration/cctv_controller.h"
#include "integration/media_controller.h"
#include "integration/settings_controller.h"
#include "ui/pages/ui_page_settings.h"
#include "ui/ui_root.h"

using namespace launcher_view;
using namespace smooth_ui_toolkit;
using namespace smooth_ui_toolkit::lvgl_cpp;
using custom::integration::CctvController;
using custom::integration::MediaController;
using custom::integration::SettingsController;

static const std::string _tag = "launcher-view";

void LauncherView::init()
{
    mclog::tagInfo(_tag, "init");

    ui::signal_window_opened().clear();
    ui::signal_window_opened().connect([&](bool opened) { _is_stacked = opened; });

    LvglLockGuard lock;

    // Base screen
    lv_obj_remove_flag(lv_screen_active(), LV_OBJ_FLAG_SCROLLABLE);

    // Background image
    _img_bg = std::make_unique<Image>(lv_screen_active());
    _img_bg->setAlign(LV_ALIGN_CENTER);

    // Install panels
    _panels.push_back(std::make_unique<PanelRtc>());
    _panels.push_back(std::make_unique<PanelLcdBacklight>());
    _panels.push_back(std::make_unique<PanelSpeakerVolume>());
    _panels.push_back(std::make_unique<PanelPowerMonitor>());
    _panels.push_back(std::make_unique<PanelImu>());
    _panels.push_back(std::make_unique<PanelSwitches>());
    _panels.push_back(std::make_unique<PanelPower>());
    _panels.push_back(std::make_unique<PanelCamera>());
    _panels.push_back(std::make_unique<PanelDualMic>());
    _panels.push_back(std::make_unique<PanelHeadphone>());
    _panels.push_back(std::make_unique<PanelSdCard>());
    _panels.push_back(std::make_unique<PanelI2cScan>());
    _panels.push_back(std::make_unique<PanelGpioTest>());
    _panels.push_back(std::make_unique<PanelMusic>());
    _panels.push_back(std::make_unique<PanelComMonitor>());

    for (auto& panel : _panels)
    {
        panel->init();
    }

    if (_ui_root != nullptr)
    {
        _media_controller.reset();
        _cctv_controller.reset();
        _settings_controller.reset();
        ui_root_destroy(_ui_root);
        _ui_root = nullptr;
    }
    _ui_root = ui_root_create();
    if (_ui_root != nullptr)
    {
        _settings_controller = std::make_unique<SettingsController>();
        _media_controller    = std::make_unique<MediaController>();
        _cctv_controller     = std::make_unique<CctvController>();

        ui_page_settings_actions_t actions{};
        actions.run_connection_test = [](const char* tester_id, void* user_data)
        {
            auto* controller = static_cast<SettingsController*>(user_data);
            if (controller != nullptr)
            {
                controller->RunConnectionTest(tester_id);
            }
        };
        actions.set_dark_mode = [](bool enabled, void* user_data)
        {
            auto* controller = static_cast<SettingsController*>(user_data);
            if (controller != nullptr)
            {
                controller->SetDarkMode(enabled);
            }
        };
        actions.set_theme_variant = [](const char* variant_id, void* user_data)
        {
            auto* controller = static_cast<SettingsController*>(user_data);
            if (controller != nullptr)
            {
                controller->SetThemeVariant(variant_id);
            }
        };
        actions.set_brightness = [](uint8_t percent, void* user_data)
        {
            auto* controller = static_cast<SettingsController*>(user_data);
            if (controller != nullptr)
            {
                controller->SetBrightness(percent);
            }
        };
        actions.open_display_settings = [](void* user_data)
        {
            auto* controller = static_cast<SettingsController*>(user_data);
            if (controller != nullptr)
            {
                controller->OpenDisplaySettings();
            }
        };
        actions.open_network_settings = [](void* user_data)
        {
            auto* controller = static_cast<SettingsController*>(user_data);
            if (controller != nullptr)
            {
                controller->OpenNetworkSettings();
            }
        };
        actions.sync_time = [](void* user_data)
        {
            auto* controller = static_cast<SettingsController*>(user_data);
            if (controller != nullptr)
            {
                controller->SyncTime();
            }
        };
        actions.check_for_updates = [](void* user_data)
        {
            auto* controller = static_cast<SettingsController*>(user_data);
            if (controller != nullptr)
            {
                controller->CheckForUpdates();
            }
        };
        actions.start_ota_update = [](void* user_data)
        {
            auto* controller = static_cast<SettingsController*>(user_data);
            if (controller != nullptr)
            {
                controller->StartOtaUpdate();
            }
        };
        actions.open_diagnostics = [](void* user_data)
        {
            auto* controller = static_cast<SettingsController*>(user_data);
            if (controller != nullptr)
            {
                controller->OpenDiagnostics();
            }
        };
        actions.export_logs = [](void* user_data)
        {
            auto* controller = static_cast<SettingsController*>(user_data);
            if (controller != nullptr)
            {
                controller->ExportLogs();
            }
        };
        actions.backup_now = [](void* user_data)
        {
            auto* controller = static_cast<SettingsController*>(user_data);
            if (controller != nullptr)
            {
                controller->BackupNow();
            }
        };
        actions.restore_backup = [](void* user_data)
        {
            auto* controller = static_cast<SettingsController*>(user_data);
            if (controller != nullptr)
            {
                controller->RestoreBackup();
            }
        };

        ui_page_settings_set_actions(&actions, _settings_controller.get());
        _settings_controller->PublishInitialState();
        if (_media_controller != nullptr)
        {
            _media_controller->PublishInitialState();
        }
        if (_cctv_controller != nullptr)
        {
            _cctv_controller->PublishInitialState();
        }
    }
}

void LauncherView::update()
{
    LvglLockGuard lock;

    for (auto& panel : _panels)
    {
        panel->update(_is_stacked);
    }
}

LauncherView::~LauncherView()
{
    _media_controller.reset();
    _cctv_controller.reset();
    _settings_controller.reset();
    if (_ui_root != nullptr)
    {
        ui_root_destroy(_ui_root);
        _ui_root = nullptr;
    }
}
