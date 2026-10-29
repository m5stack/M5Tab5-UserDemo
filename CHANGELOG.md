# Changelog

[English](CHANGELOG_EN.md) | **中文**

## [V0.3] - 最新修改

### 新增功能

#### 文档
- **新增中文版 README** (`README_CN.md`)
  - 完整的中文项目文档
  - 包含项目概述、功能列表、构建说明等

- **新增 ESP-Hosted 配置指南** (`docs/ESP_HOSTED_CONFIGURATION_GUIDE.md`)
  - 详细的 ESP-Hosted 配置说明
  - 包含 M5Stack Tab5 的 GPIO 配置信息
  - 提供 menuconfig 和直接编辑两种配置方法

#### Windows 平台支持
- **增强 Windows 桌面构建支持**
  - 添加 vcpkg 构建方式支持
  - 添加 MSYS2 构建方式支持
  - 添加 Visual Studio 构建方式支持
  - 改进 SDL2 依赖查找和链接方式
  - 添加 MSVC 编译选项（UTF-8 编码、数学常量支持）

### 改进

#### 文档改进
- **大幅更新 README.md**
  - 添加项目概述和功能列表
  - 添加详细的 Windows 构建说明（vcpkg/MSYS2/Visual Studio）
  - 添加 ESP32-P4 目标芯片说明和常见问题解决方案
  - 添加 Windows 编码错误解决方案
  - 添加 ESP-Hosted 配置说明章节
  - 改进构建步骤说明，明确执行目录

#### 桌面平台优化
- **改进桌面平台事件处理** (`app/app.cpp`)
  - 添加 SDL 事件轮询处理
  - 添加 Windows 平台 CPU 使用率优化（添加延迟）
  - 改进跨平台兼容性

- **改进 LVGL 初始化** (`platforms/desktop/hal/components/hal_lvgl.cpp`)
  - 添加 SDL 窗口创建错误检查
  - 添加初始刷新调用，确保窗口可见
  - 改进 Windows 平台定时器处理（处理 `LV_NO_TIMER_READY` 情况）
  - 添加最小睡眠时间保护

- **改进应用启动器视图** (`app/apps/app_launcher/view/view.cpp`)
  - 添加桌面平台的强制刷新，确保内容正确显示

- **改进 CMake 构建配置** (`platforms/desktop/CMakeLists.txt`)
  - 改进 SDL2 查找和链接方式（支持 vcpkg 的 CMake targets）
  - 添加 MSVC 特定编译选项
  - 改进 C/C++ 文件分离处理
  - 添加条件链接 pthread（仅非 Windows 平台）
  - 添加 CMake 策略设置

#### 硬件平台改进
- **更新音频编解码器依赖** (`platforms/tab5/components/m5stack_tab5/idf_component.yml`)
  - 升级 `espressif/esp_codec_dev` 从 `^1.3.0` 到 `^1.5.2`

- **修复音频编解码器配置** (`platforms/tab5/components/m5stack_tab5/m5stack_tab5.c`)
  - 修复 ES7210 麦克风选择宏错误（`ES7120_SEL_MIC*` → `ES7210_SEL_MIC*`）
  - 注释掉未使用的 GPIO 接口代码，避免编译警告
  - 注释掉未使用的硬件增益配置
  - 注释掉已弃用的 I2C 端口配置，使用新的 bus_handle 方式

- **添加 ESP-Hosted 配置** (`platforms/tab5/sdkconfig.defaults`)
  - 配置 ESP32C6 Wi-Fi 协处理器
  - 配置 SDIO2 接口（Slot 1）
  - 配置 GPIO 引脚（CMD: 13, CLK: 12, D0-D3: 11-8, Reset: 54）
  - 设置 4 位总线宽度和 40MHz 时钟频率

#### 代码质量改进
- **改进头文件兼容性** (`app/assets/assets.h`)
  - 添加 C++ 兼容性保护（`extern "C"`）

### 技术细节

#### 构建系统
- 支持 vcpkg 工具链文件
- 改进 CMake targets 使用方式
- 添加平台特定的编译选项

#### 平台特定优化
- Windows: 添加 CPU 使用率优化
- Windows: 改进定时器处理
- Windows: 添加编码支持（UTF-8）

#### 依赖更新
- `espressif/esp_codec_dev`: `^1.3.0` → `^1.5.2`

### 修复的问题

1. **修复 ES7210 麦克风选择宏错误**
   - 问题：使用了错误的宏名称 `ES7120_SEL_MIC*`
   - 修复：更正为 `ES7210_SEL_MIC*`

2. **修复桌面平台渲染问题**
   - 问题：桌面平台初始渲染可能不完整
   - 修复：添加强制刷新调用

3. **修复 Windows 平台高 CPU 使用率**
   - 问题：Windows 平台 CPU 使用率过高
   - 修复：添加适当的延迟处理

4. **修复 CMake 构建配置问题**
   - 问题：Windows 平台 SDL2 查找和链接问题
   - 修复：改进 SDL2 查找方式，支持 vcpkg

### 配置文件变更

- `platforms/tab5/sdkconfig`: 自动生成的配置文件更新
- `platforms/tab5/dependencies.lock`: 依赖锁定文件更新

---

## 修改文件列表

### 新增文件
- `README_CN.md` - 中文版 README
- `docs/ESP_HOSTED_CONFIGURATION_GUIDE.md` - ESP-Hosted 配置指南

### 修改文件
- `README.md` - 大幅更新文档
- `app/app.cpp` - 添加 SDL 事件处理和 Windows 优化
- `app/apps/app_launcher/view/view.cpp` - 添加桌面平台刷新
- `app/assets/assets.h` - 添加 C++ 兼容性保护
- `platforms/desktop/CMakeLists.txt` - 改进 Windows 构建支持
- `platforms/desktop/hal/components/hal_lvgl.cpp` - 改进错误处理和 Windows 优化
- `platforms/tab5/components/m5stack_tab5/idf_component.yml` - 更新依赖版本
- `platforms/tab5/components/m5stack_tab5/m5stack_tab5.c` - 修复音频编解码器配置
- `platforms/tab5/sdkconfig.defaults` - 添加 ESP-Hosted 配置
- `platforms/tab5/sdkconfig` - 自动生成的配置更新
- `platforms/tab5/dependencies.lock` - 依赖锁定文件更新

