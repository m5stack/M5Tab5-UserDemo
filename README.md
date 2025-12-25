# M5Tab5 User Demo

[中文](README_CN.md) | **English**

User demo source code of [M5Tab5](https://docs.m5stack.com/en/products/sku/k145).

## Project Overview

This is a **M5Tab5 user demo project** based on ESP-IDF and LVGL graphics library, demonstrating various hardware features of the M5Tab5 device. The project uses a modular design where each feature is implemented as an independent panel, providing a complete Hardware Abstraction Layer (HAL) that supports both desktop and ESP32 hardware platform builds.

## Features

### 1. **Real-Time Clock (RTC)**
   - Display current time and date
   - RTC settings support

### 2. **Display Control**
   - LCD backlight brightness control

### 3. **Audio Features**
   - Speaker volume control
   - Dual microphone testing
   - Headphone testing
   - Music playback

### 4. **Power Management**
   - Real-time power monitoring (voltage, current, CPU temperature)
   - Charging status display
   - Multiple switch controls:
     - Charging enable switch
     - QC fast charging switch
     - External 5V output switch
     - USB-A 5V output switch
     - External antenna switch
   - Power management functions:
     - Shutdown function
     - Sleep mode (supports touch/shake/RTC wakeup)

### 5. **Sensor Features**
   - Real-time IMU display
   - 3-axis accelerometer data visualization (X/Y/Z axes)

### 6. **Peripheral Interfaces**
   - Camera function testing
   - SD card scanning and detection
   - I2C device scanning
   - GPIO testing
   - Serial communication monitoring

### 7. **System Features**
   - Startup animation
   - Application launcher interface
   - USB-C/USB-A/Headphone auto-detection

## Technical Features

- **Cross-platform support**: Desktop build (SDL2) and ESP32 hardware build
- **Graphics interface**: Modern UI built with LVGL and Smooth UI Toolkit
- **Modular design**: Each feature implemented as an independent panel, easy to extend
- **Hardware abstraction**: HAL layer design supports hardware abstraction for different platforms

## Build

### Fetch Dependencies

```bash
python ./fetch_repos.py
```

## Desktop Build

### Linux

#### Tool Chains

```bash
sudo apt install build-essential cmake libsdl2-dev
```

#### Build

**Note: The following build steps should be executed in the project root directory.**

```bash
mkdir build && cd build
```

```bash
cmake .. && make -j8
```

#### Run

```bash
./desktop/app_desktop_build
```

### Windows

#### Tool Chains

**Method 1: Using vcpkg (Recommended)**

1. Install [vcpkg](https://github.com/microsoft/vcpkg):
   ```powershell
   git clone https://github.com/microsoft/vcpkg.git
   cd vcpkg
   .\bootstrap-vcpkg.bat
   ```

2. Install SDL2:
   ```powershell
   .\vcpkg install sdl2:x64-windows
   ```

3. Install CMake (if not already installed):
   - Download Windows installer from [CMake website](https://cmake.org/download/) (choose `.msi` package), select "Add CMake to system PATH" during installation
   - Or use package manager (requires package manager installation first):
     - Chocolatey: `choco install cmake` (requires [Chocolatey](https://chocolatey.org/install) first)
     - winget: `winget install Kitware.CMake`
   
   **Note:** After installing CMake, if PowerShell cannot find the `cmake` command:
   - Restart PowerShell window (recommended)
   - Or manually refresh environment variables: `$env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")`
   - Or use CMake's full path (usually `C:\Program Files\CMake\bin\cmake.exe`)

4. Install Visual Studio or Build Tools:
   - Install [Visual Studio](https://visualstudio.microsoft.com/) (with C++ workload)
   - Or install [Visual Studio Build Tools](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022)

**Method 2: Using MSYS2**

1. Install [MSYS2](https://www.msys2.org/)

2. Install dependencies in MSYS2 terminal:
   ```bash
   pacman -S mingw-w64-x86_64-cmake mingw-w64-x86_64-gcc mingw-w64-x86_64-make mingw-w64-x86_64-SDL2
   ```

#### Build

**Note: All build steps below should be executed in the project root directory (M5Tab5-GPS), not in the `platforms/desktop` directory.**

**Using vcpkg:**

Execute in the project root directory:
```powershell
mkdir build
cd build
$vcpkgRoot = (Resolve-Path "../vcpkg").Path.Replace('\', '/')
cmake .. -DCMAKE_TOOLCHAIN_FILE="$vcpkgRoot/scripts/buildsystems/vcpkg.cmake"
cmake --build . --config Release -j
```

Or use absolute path directly (replace with your actual path):
```powershell
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE="C:/Users/vicliu/Projects/M5Tab5-GPS/vcpkg/scripts/buildsystems/vcpkg.cmake"
cmake --build . --config Release -j
```

**If cmake command is not found:**
- Restart PowerShell window (recommended)
- Or refresh environment variables: `$env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")`
- Or use full path: `& "C:\Program Files\CMake\bin\cmake.exe" .. -DCMAKE_TOOLCHAIN_FILE="C:\Users\vicliu\Projects\M5Tab5-GPS\vcpkg\scripts\buildsystems\vcpkg.cmake"`

**If CMake cannot find SDL2:**
- Ensure vcpkg toolchain file path uses forward slashes `/` or escaped backslashes
- You can set VCPKG_ROOT environment variable: `$env:VCPKG_ROOT = (Resolve-Path "../vcpkg").Path`
- Then use: `cmake .. -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"`

**Note:** If vcpkg is installed in a different location, replace the path with your actual vcpkg path.

**Using MSYS2:**

Execute in MSYS2 MinGW64 terminal in the project root directory:
```bash
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
cmake --build . -j8
```

**Using Visual Studio:**

Execute in the project root directory:
```powershell
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release -j
```

#### Run

**Using vcpkg or Visual Studio:**
```powershell
.\desktop\Release\app_desktop_build.exe
```
or
```powershell
.\desktop\app_desktop_build.exe
```

**Using MSYS2:**
```bash
./desktop/app_desktop_build.exe
```

## IDF Build

#### Tool Chains

[ESP-IDF v5.4.2](https://docs.espressif.com/projects/esp-idf/en/v5.4.2/esp32s3/index.html)

**Note:** This project uses **ESP32-P4** chip (ESP32-P4NRW32), not ESP32-S3.

#### Build

```bash
cd platforms/tab5
```

**When building for the first time or switching targets, you need to set the correct target chip:**

```bash
idf.py set-target esp32p4
```

Then build:

```bash
idf.py build
```

**If you encounter a target chip inconsistency error:**

- **Error message:** "Target 'esp32p4' specified on command line is not consistent with target 'esp32' in the environment"
- **Solution:**
  
  **Windows PowerShell:**
  ```powershell
  # Clear environment variable first
  Remove-Item Env:IDF_TARGET -ErrorAction SilentlyContinue
  # Then set target
  idf.py set-target esp32p4
  ```
  
  Or set directly in one command:
  ```powershell
  $env:IDF_TARGET = "esp32p4"
  idf.py set-target esp32p4
  ```
  
  **Linux/Mac:**
  ```bash
  unset IDF_TARGET
  idf.py set-target esp32p4
  ```
  
  Or:
  ```bash
  export IDF_TARGET=esp32p4
  idf.py set-target esp32p4
  ```

**If you encounter encoding errors on Windows:**

When running `idf.py reconfigure`, you may encounter:
```
UnicodeDecodeError: 'gbk' codec can't decode byte 0xa4 in position 1478
```

**Solution:**
Set Python encoding environment variables before running:
```powershell
$env:PYTHONIOENCODING = "utf-8"
$env:PYTHONUTF8 = "1"
cd platforms/tab5
idf.py reconfigure
```

#### Flash

```bash
idf.py flash
```

## ESP-Hosted Configuration

This project uses ESP-Hosted to provide Wi-Fi functionality via ESP32C6 coprocessor connected through SDIO2 interface.

### Configuration

The ESP-Hosted configuration is automatically set in `sdkconfig.defaults`:

- **Slave Target**: ESP32C6
- **Interface**: SDIO2 (Slot 1)
- **GPIO Configuration**:
  - CMD: GPIO 13
  - CLK: GPIO 12
  - D0: GPIO 11
  - D1: GPIO 10
  - D2: GPIO 9
  - D3: GPIO 8
  - Reset: GPIO 54

For detailed configuration information, see:
- [ESP-Hosted Configuration Guide](docs/ESP_HOSTED_CONFIGURATION_GUIDE.md)

## Changelog

For a detailed list of changes, see:
- [Changelog (English)](CHANGELOG_EN.md)
- [更新日志 (中文)](CHANGELOG.md)

## Acknowledgments

This project references the following open-source libraries and resources:

- https://github.com/lvgl/lvgl
- https://www.heroui.com
- https://github.com/Forairaaaaa/smooth_ui_toolkit
- https://github.com/Forairaaaaa/mooncake
- https://github.com/Forairaaaaa/mooncake_log
- https://github.com/alexreinert/piVCCU/blob/master/kernel/rtc-rx8130.c
- https://components.espressif.com/components/espressif/esp_cam_sensor
- https://components.espressif.com/components/espressif/esp_ipa
- https://components.espressif.com/components/espressif/esp_sccb_intf
- https://components.espressif.com/components/espressif/esp_video
- https://components.espressif.com/components/espressif/esp_lvgl_port
- https://github.com/jarzebski/Arduino-INA226
- https://github.com/boschsensortec/BMI270_SensorAPI
