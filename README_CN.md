# M5Tab5 用户演示项目

**中文** | [English](README.md)

[M5Tab5](https://docs.m5stack.com/en/products/sku/k145) 的用户演示源代码。

## 项目简介

这是一个 **M5Tab5 用户演示项目**，基于 ESP-IDF 和 LVGL 图形库开发，展示了 M5Tab5 设备的各项硬件功能。项目采用模块化设计，每个功能以独立面板的形式实现，提供了完整的硬件抽象层（HAL），支持桌面平台和 ESP32 硬件平台构建。

## 主要功能

### 1. **实时时钟 (RTC)**
   - 显示当前时间和日期
   - 支持 RTC 设置功能

### 2. **显示控制**
   - LCD 背光亮度调节

### 3. **音频功能**
   - 扬声器音量控制
   - 双麦克风测试
   - 耳机测试
   - 音乐播放功能

### 4. **电源管理**
   - 实时电源监控（电压、电流、CPU 温度）
   - 充电状态显示（充电/放电指示）
   - 多种开关控制：
     - 充电使能开关
     - QC 快充开关
     - 外部 5V 输出开关
     - USB-A 5V 输出开关
     - 外部天线开关
   - 电源管理功能：
     - 关机功能
     - 睡眠模式（支持触摸/摇动/RTC 唤醒）

### 5. **传感器功能**
   - IMU（惯性测量单元）实时显示
   - 三轴加速度计数据可视化（X/Y/Z 轴）

### 6. **外设接口**
   - 摄像头功能测试
   - SD 卡扫描和检测
   - I2C 设备扫描
   - GPIO 测试
   - 串口通信监控

### 7. **系统功能**
   - 启动动画
   - 应用启动器界面
   - USB-C/USB-A/耳机自动检测

## 技术特点

- **跨平台支持**：支持桌面构建（SDL2）和 ESP32 硬件构建
- **图形界面**：基于 LVGL 和 Smooth UI Toolkit 构建现代化 UI
- **模块化设计**：每个功能以独立面板形式实现，易于扩展
- **硬件抽象**：HAL 层设计支持不同平台的硬件抽象

## 构建

### 获取依赖

```bash
python ./fetch_repos.py
```

## 桌面构建

### Linux

#### 工具链

```bash
sudo apt install build-essential cmake libsdl2-dev
```

#### 构建

**注意：以下构建步骤在项目根目录执行。**

```bash
mkdir build && cd build
```

```bash
cmake .. && make -j8
```

#### 运行

```bash
./desktop/app_desktop_build
```

### Windows

#### 工具链

**方式一：使用 vcpkg（推荐）**

1. 安装 [vcpkg](https://github.com/microsoft/vcpkg)：
   ```powershell
   git clone https://github.com/microsoft/vcpkg.git
   cd vcpkg
   .\bootstrap-vcpkg.bat
   ```

2. 安装 SDL2：
   ```powershell
   .\vcpkg install sdl2:x64-windows
   ```

3. 安装 CMake（如果尚未安装）：
   - 从 [CMake官网](https://cmake.org/download/) 下载 Windows 安装程序（选择 `.msi` 安装包），安装时选择"Add CMake to system PATH"
   - 或使用包管理器（需要先安装对应包管理器）：
     - Chocolatey：`choco install cmake`（需要先安装 [Chocolatey](https://chocolatey.org/install)）
     - winget：`winget install Kitware.CMake`
   
   **注意：** 安装 CMake 后，如果 PowerShell 提示找不到 `cmake` 命令，请：
   - 重新启动 PowerShell 窗口（推荐）
   - 或手动刷新环境变量：`$env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")`
   - 或使用 CMake 的完整路径（通常在 `C:\Program Files\CMake\bin\cmake.exe`）

4. 安装 Visual Studio 或 Build Tools：
   - 安装 [Visual Studio](https://visualstudio.microsoft.com/)（包含 C++ 工作负载）
   - 或安装 [Visual Studio Build Tools](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022)

**方式二：使用 MSYS2**

1. 安装 [MSYS2](https://www.msys2.org/)

2. 在 MSYS2 终端中安装依赖：
   ```bash
   pacman -S mingw-w64-x86_64-cmake mingw-w64-x86_64-gcc mingw-w64-x86_64-make mingw-w64-x86_64-SDL2
   ```

#### 构建

**注意：以下所有构建步骤都在项目根目录（M5Tab5-GPS）执行，而不是在 `platforms/desktop` 目录中。**

**使用 vcpkg 构建：**

在项目根目录执行：
```powershell
mkdir build
cd build
$vcpkgRoot = (Resolve-Path "../vcpkg").Path.Replace('\', '/')
cmake .. -DCMAKE_TOOLCHAIN_FILE="$vcpkgRoot/scripts/buildsystems/vcpkg.cmake"
cmake --build . --config Release -j
```

或者直接使用绝对路径（将路径替换为你的实际路径）：
```powershell
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE="C:/Users/vicliu/Projects/M5Tab5-GPS/vcpkg/scripts/buildsystems/vcpkg.cmake"
cmake --build . --config Release -j
```

**如果提示找不到 cmake 命令：**
- 重新启动 PowerShell 窗口（推荐）
- 或刷新环境变量：`$env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")`
- 或使用完整路径：`& "C:\Program Files\CMake\bin\cmake.exe" .. -DCMAKE_TOOLCHAIN_FILE="C:\Users\vicliu\Projects\M5Tab5-GPS\vcpkg\scripts\buildsystems\vcpkg.cmake"`

**如果 CMake 找不到 SDL2：**
- 确保 vcpkg toolchain 文件路径使用正斜杠 `/` 或转义的反斜杠
- 可以设置 VCPKG_ROOT 环境变量：`$env:VCPKG_ROOT = (Resolve-Path "../vcpkg").Path`
- 然后使用：`cmake .. -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"`

**注意：** 如果 vcpkg 安装在其他位置，请将路径替换为实际的 vcpkg 路径。

**使用 MSYS2 构建：**

在项目根目录的 MSYS2 MinGW64 终端中执行：
```bash
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
cmake --build . -j8
```

**使用 Visual Studio 构建：**

在项目根目录执行：
```powershell
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release -j
```

#### 运行

**使用 vcpkg 或 Visual Studio：**
```powershell
.\desktop\Release\app_desktop_build.exe
```
或
```powershell
.\desktop\app_desktop_build.exe
```

**使用 MSYS2：**
```bash
./desktop/app_desktop_build.exe
```

## IDF 构建

#### 工具链

[ESP-IDF v5.4.2](https://docs.espressif.com/projects/esp-idf/en/v5.4.2/esp32s3/index.html)

**注意：** 本项目使用 **ESP32-P4** 芯片（ESP32-P4NRW32），不是 ESP32-S3。

#### 构建

```bash
cd platforms/tab5
```

**首次构建或切换目标时，需要设置正确的目标芯片：**

```bash
idf.py set-target esp32p4
```

然后构建：

```bash
idf.py build
```

**如果遇到目标芯片不一致的错误：**

- **错误信息：** "Target 'esp32p4' specified on command line is not consistent with target 'esp32' in the environment"
- **解决方法：**
  
  **Windows PowerShell：**
  ```powershell
  # 先清除环境变量
  Remove-Item Env:IDF_TARGET -ErrorAction SilentlyContinue
  # 然后设置目标
  idf.py set-target esp32p4
  ```
  
  或者直接在一个命令中设置：
  ```powershell
  $env:IDF_TARGET = "esp32p4"
  idf.py set-target esp32p4
  ```
  
  **Linux/Mac：**
  ```bash
  unset IDF_TARGET
  idf.py set-target esp32p4
  ```
  
  或者：
  ```bash
  export IDF_TARGET=esp32p4
  idf.py set-target esp32p4
  ```

**如果遇到编码错误（Windows）：**

运行 `idf.py reconfigure` 时，可能会遇到：
```
UnicodeDecodeError: 'gbk' codec can't decode byte 0xa4 in position 1478
```

**解决方法：**
在运行前设置 Python 编码环境变量：
```powershell
$env:PYTHONIOENCODING = "utf-8"
$env:PYTHONUTF8 = "1"
cd platforms/tab5
idf.py reconfigure
```

#### 烧录

```bash
idf.py flash
```

## ESP-Hosted 配置

本项目使用 ESP-Hosted 通过 ESP32C6 协处理器提供 Wi-Fi 功能，协处理器通过 SDIO2 接口连接。

### 配置说明

ESP-Hosted 配置已自动设置在 `sdkconfig.defaults` 中：

- **从设备目标**：ESP32C6
- **接口**：SDIO2 (Slot 1)
- **GPIO 配置**：
  - CMD: GPIO 13
  - CLK: GPIO 12
  - D0: GPIO 11
  - D1: GPIO 10
  - D2: GPIO 9
  - D3: GPIO 8
  - Reset: GPIO 54

详细配置信息请参考：
- [ESP-Hosted 配置指南](docs/ESP_HOSTED_CONFIGURATION_GUIDE.md)

## 更新日志

详细的更改列表请参考：
- [更新日志 (中文)](CHANGELOG.md)
- [Changelog (English)](CHANGELOG_EN.md)

## 致谢

本项目参考了以下开源库和资源：

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

