# Changelog

**English** | [中文](CHANGELOG.md)

## [V0.3] - Latest Changes

### New Features

#### Documentation
- **Added Chinese README** (`README_CN.md`)
  - Complete Chinese project documentation
  - Includes project overview, feature list, build instructions, etc.

- **Added ESP-Hosted Configuration Guide** (`docs/ESP_HOSTED_CONFIGURATION_GUIDE.md`)
  - Detailed ESP-Hosted configuration instructions
  - Includes GPIO configuration information for M5Stack Tab5
  - Provides two configuration methods: menuconfig and direct editing

#### Windows Platform Support
- **Enhanced Windows Desktop Build Support**
  - Added vcpkg build method support
  - Added MSYS2 build method support
  - Added Visual Studio build method support
  - Improved SDL2 dependency finding and linking
  - Added MSVC compilation options (UTF-8 encoding, math constants support)

### Improvements

#### Documentation Improvements
- **Major README.md Update**
  - Added project overview and feature list
  - Added detailed Windows build instructions (vcpkg/MSYS2/Visual Studio)
  - Added ESP32-P4 target chip description and common issue solutions
  - Added Windows encoding error solutions
  - Added ESP-Hosted configuration section
  - Improved build step instructions with clear execution directory

#### Desktop Platform Optimization
- **Improved Desktop Platform Event Handling** (`app/app.cpp`)
  - Added SDL event polling
  - Added Windows platform CPU usage optimization (added delay)
  - Improved cross-platform compatibility

- **Improved LVGL Initialization** (`platforms/desktop/hal/components/hal_lvgl.cpp`)
  - Added SDL window creation error checking
  - Added initial refresh call to ensure window visibility
  - Improved Windows platform timer handling (handling `LV_NO_TIMER_READY` case)
  - Added minimum sleep time protection

- **Improved Application Launcher View** (`app/apps/app_launcher/view/view.cpp`)
  - Added forced refresh for desktop platform to ensure correct content display

- **Improved CMake Build Configuration** (`platforms/desktop/CMakeLists.txt`)
  - Improved SDL2 finding and linking (supports vcpkg CMake targets)
  - Added MSVC-specific compilation options
  - Improved C/C++ file separation handling
  - Added conditional pthread linking (non-Windows platforms only)
  - Added CMake policy settings

#### Hardware Platform Improvements
- **Updated Audio Codec Dependency** (`platforms/tab5/components/m5stack_tab5/idf_component.yml`)
  - Upgraded `espressif/esp_codec_dev` from `^1.3.0` to `^1.5.2`

- **Fixed Audio Codec Configuration** (`platforms/tab5/components/m5stack_tab5/m5stack_tab5.c`)
  - Fixed ES7210 microphone selection macro error (`ES7120_SEL_MIC*` → `ES7210_SEL_MIC*`)
  - Commented out unused GPIO interface code to avoid compilation warnings
  - Commented out unused hardware gain configuration
  - Commented out deprecated I2C port configuration, using new bus_handle method

- **Added ESP-Hosted Configuration** (`platforms/tab5/sdkconfig.defaults`)
  - Configured ESP32C6 Wi-Fi coprocessor
  - Configured SDIO2 interface (Slot 1)
  - Configured GPIO pins (CMD: 13, CLK: 12, D0-D3: 11-8, Reset: 54)
  - Set 4-bit bus width and 40MHz clock frequency

#### Code Quality Improvements
- **Improved Header File Compatibility** (`app/assets/assets.h`)
  - Added C++ compatibility protection (`extern "C"`)

### Technical Details

#### Build System
- Support for vcpkg toolchain file
- Improved CMake targets usage
- Added platform-specific compilation options

#### Platform-Specific Optimizations
- Windows: Added CPU usage optimization
- Windows: Improved timer handling
- Windows: Added encoding support (UTF-8)

#### Dependency Updates
- `espressif/esp_codec_dev`: `^1.3.0` → `^1.5.2`

### Fixed Issues

1. **Fixed ES7210 Microphone Selection Macro Error**
   - Issue: Used incorrect macro name `ES7120_SEL_MIC*`
   - Fix: Corrected to `ES7210_SEL_MIC*`

2. **Fixed Desktop Platform Rendering Issue**
   - Issue: Desktop platform initial rendering might be incomplete
   - Fix: Added forced refresh call

3. **Fixed Windows Platform High CPU Usage**
   - Issue: Windows platform had high CPU usage
   - Fix: Added appropriate delay handling

4. **Fixed CMake Build Configuration Issue**
   - Issue: Windows platform SDL2 finding and linking problems
   - Fix: Improved SDL2 finding method, supporting vcpkg

### Configuration File Changes

- `platforms/tab5/sdkconfig`: Auto-generated configuration file update
- `platforms/tab5/dependencies.lock`: Dependency lock file update

---

## Modified Files List

### New Files
- `README_CN.md` - Chinese README
- `docs/ESP_HOSTED_CONFIGURATION_GUIDE.md` - ESP-Hosted Configuration Guide

### Modified Files
- `README.md` - Major documentation update
- `app/app.cpp` - Added SDL event handling and Windows optimization
- `app/apps/app_launcher/view/view.cpp` - Added desktop platform refresh
- `app/assets/assets.h` - Added C++ compatibility protection
- `platforms/desktop/CMakeLists.txt` - Improved Windows build support
- `platforms/desktop/hal/components/hal_lvgl.cpp` - Improved error handling and Windows optimization
- `platforms/tab5/components/m5stack_tab5/idf_component.yml` - Updated dependency version
- `platforms/tab5/components/m5stack_tab5/m5stack_tab5.c` - Fixed audio codec configuration
- `platforms/tab5/sdkconfig.defaults` - Added ESP-Hosted configuration
- `platforms/tab5/sdkconfig` - Auto-generated configuration update
- `platforms/tab5/dependencies.lock` - Dependency lock file update

