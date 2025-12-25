# ESP-Hosted 配置指南（基于 esp-hosted-mcu 工程）

根据 `esp-hosted-mcu` 工程的配置，ESP32-P4 和 ESP32C6 的配置有以下几个关键点：

### 1. ESP32-P4 SDIO 架构

- **Slot 0 (SDIO1)**: 固定 GPIO，用于 SD 卡（GPIO 39-44）
- **Slot 1 (SDIO2)**: 灵活 GPIO，用于 ESP32C6 协处理器

### 2. ESP32-P4-Function-EV-Board 默认配置

ESP32-P4-Function-EV-Board 的默认配置（ESP32C6 协处理器）：
- **CLK**: GPIO 18
- **CMD**: GPIO 19
- **D0**: GPIO 14
- **D1**: GPIO 15
- **D2**: GPIO 16
- **D3**: GPIO 17
- **Reset**: GPIO 54

### 3. M5Stack Tab5 实际配置

根据原理图和用户确认，M5Stack Tab5 使用不同的 GPIO：
- **CLK**: GPIO 12
- **CMD**: GPIO 13
- **D0**: GPIO 11
- **D1**: GPIO 10
- **D2**: GPIO 9
- **D3**: GPIO 8
- **Reset**: GPIO 54

## 配置方法

### 方法 1：使用 menuconfig（推荐）

1. **运行 menuconfig**：
   ```bash
   cd platforms/tab5
   idf.py menuconfig
   ```

2. **导航到 ESP-Hosted 配置**：
   ```
   Component config
   └── ESP-Hosted config
       └── Hosted SDIO Configuration
           ├── SDIO Slot To Use: 选择 "Slot 1"
           ├── CMD GPIO number: 设置为 13
           ├── CLK GPIO number: 设置为 12
           ├── D0 GPIO number: 设置为 11
           ├── D1 GPIO number: 设置为 10
           ├── D2 GPIO number: 设置为 9
           ├── D3 GPIO number: 设置为 8
           └── GPIO pin for Reseting slave ESP: 设置为 54
   ```

3. **保存并退出**

### 方法 2：直接编辑 sdkconfig

如果已经知道配置项名称，可以直接编辑 `platforms/tab5/sdkconfig`：

```ini
# 确保使用 SLOT_1
CONFIG_ESP_HOSTED_SDIO_SLOT_1=y
# CONFIG_ESP_HOSTED_SDIO_SLOT_0 is not set

# GPIO 配置（SLOT_1）
CONFIG_ESP_HOSTED_PRIV_SDIO_PIN_CMD_SLOT_1=13
CONFIG_ESP_HOSTED_PRIV_SDIO_PIN_CLK_SLOT_1=12
CONFIG_ESP_HOSTED_PRIV_SDIO_PIN_D0_SLOT_1=11
CONFIG_ESP_HOSTED_PRIV_SDIO_PIN_D1_4BIT_BUS_SLOT_1=10
CONFIG_ESP_HOSTED_PRIV_SDIO_PIN_D2_4BIT_BUS_SLOT_1=9
CONFIG_ESP_HOSTED_PRIV_SDIO_PIN_D3_4BIT_BUS_SLOT_1=8
CONFIG_ESP_HOSTED_SDIO_GPIO_RESET_SLAVE=54
```

**注意**：如果使用新版本的 ESP-Hosted，配置项名称可能是 `CONFIG_ESP_HOSTED_PRIV_SDIO_PIN_*_SLOT_1`，而不是 `CONFIG_ESP_HOSTED_SDIO_PIN_*`。

## 当前配置状态

根据 `platforms/tab5/sdkconfig`，当前使用的是旧版配置项名称：
- `CONFIG_ESP_HOSTED_SDIO_PIN_CMD=13` ✅
- `CONFIG_ESP_HOSTED_SDIO_PIN_CLK=12` ✅
- `CONFIG_ESP_HOSTED_SDIO_PIN_D0=11` ✅
- `CONFIG_ESP_HOSTED_SDIO_PIN_D1=10` ✅
- `CONFIG_ESP_HOSTED_SDIO_PIN_D2=9` ✅
- `CONFIG_ESP_HOSTED_SDIO_PIN_D3=8` ✅
- `CONFIG_ESP_HOSTED_SDIO_GPIO_RESET_SLAVE=54` ✅

这些配置项应该会被 ESP-Hosted 正确识别和使用。

## 验证配置

1. **重新配置**：
   ```bash
   cd platforms/tab5
   idf.py reconfigure
   ```

2. **检查配置**：
   ```bash
   idf.py menuconfig
   ```
   导航到 `Component config` → `ESP-Hosted config` → `Hosted SDIO Configuration`，确认 GPIO 配置正确。

3. **编译并测试**：
   ```bash
   idf.py build
   idf.py flash monitor
   ```

## 参考文档

- ESP-Hosted-MCU 官方文档：`.history/esp-hosted-mcu/README.md`
- ESP32-P4 Function EV Board 指南：`.history/esp-hosted-mcu/docs/esp32_p4_function_ev_board.md`
- SDIO 配置指南：`.history/esp-hosted-mcu/docs/sdio.md`

