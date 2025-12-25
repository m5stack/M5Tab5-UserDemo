/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "app.h"
#include "hal/hal.h"
#include "apps/app_installer.h"
#include <mooncake.h>
#include <mooncake_log.h>
#include <string>
#include <thread>

#if defined(PLATFORM_BUILD_DESKTOP)
#include <SDL2/SDL.h>
#endif

using namespace mooncake;

static const std::string _tag = "app";

void app::Init(InitCallback_t callback)
{
    mclog::tagInfo(_tag, "init");

    mclog::tagInfo(_tag, "hal injection");
    if (callback.onHalInjection) {
        callback.onHalInjection();
    }

    GetMooncake();

    on_startup_anim();
    on_install_apps();
}

void app::Update()
{
    GetMooncake().update();

#if defined(PLATFORM_BUILD_DESKTOP)
    // Handle SDL events on desktop platform
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        // SDL events are handled by LVGL SDL driver automatically
        // We just need to poll them
        if (event.type == SDL_QUIT) {
            // Handle window close event if needed
        }
    }

#if defined(_WIN32) || defined(WIN32)
    // Windows-specific: Add small delay to prevent high CPU usage
    // LVGL timer handler runs in separate thread on Windows
    SDL_Delay(5);
#endif
#endif

#if defined(__APPLE__) && defined(__MACH__)
    // 'nextEventMatchingMask should only be called from the Main Thread!'
    auto time_till_next = lv_timer_handler();
    std::this_thread::sleep_for(std::chrono::milliseconds(time_till_next));
#endif
}

bool app::IsDone()
{
    return false;
}

void app::Destroy()
{
    DestroyMooncake();
    hal::Destroy();
}
