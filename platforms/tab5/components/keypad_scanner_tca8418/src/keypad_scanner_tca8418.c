/***************************************************


 Tab5 tca8418 采用 8x9 矩阵(matrix)按键 + 1个通用(GPI)输入

***************************************************/
#include "keypad_scanner_tca8418.h"
#include "keypad_scanner_tca8418_reg.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "bsp/esp-bsp.h"

static const char* TAG = "tca8418";

static i2c_master_dev_handle_t i2c_dev_handle_tca8418;
#define I2C_MASTER_TIMEOUT_MS 100
#define I2C_DEV_ADDR_TCA8418  0x34

QueueHandle_t keypad_evt_queue = NULL;
static void write_reg(uint8_t reg, uint8_t val);
static uint8_t read_reg(uint8_t reg);

void app_keypad_scanner_test(void* pvParam)
{
    uint32_t io_num = 0;

    bsp_ext_i2c_init();
    i2c_master_bus_handle_t ext_i2c_bus_handle = bsp_ext_i2c_get_handle();
    keypad_scanner_tca8418_init(ext_i2c_bus_handle);
    keypad_scanner_tca8418_matrix(8, 9);  // 8x9 + col9 (GPI)
    keypad_scanner_tca8418_flush();
    keypad_scanner_tca8418_irq_init(50);  // TAB5_TCA8418_INT_PIN
    keypad_scanner_tca8418_enable_int();

    // //keypad_scanner_tca8418_init();
    // keypad_scanner_tca8418_matrix(8, 9); // 8x9 + col9 (GPI)
    // keypad_scanner_tca8418_flush();
    // // 配置外部中断
    // //keypad_scanner_tca8418_irq_init();
    // //
    // keypad_scanner_tca8418_enable_int();

    while (1) {
        if (xQueueReceive(keypad_evt_queue, &io_num, portMAX_DELAY)) {
            // ESP_LOGI(TAG, "GPIO[%ld] interrupt detected\n", io_num);

            uint8_t int_stat = read_reg(TCA8418_REG_INT_STAT);
            // printf("int stat: %#x\n", int_stat);

            // GPI interrupt
            if (int_stat & 0x02) {
                // reading the registers is mandatory to clear IRQ flag
                // can also be used to find the GPIO changed
                // as these registers are a bitmap of the gpio pins.
                read_reg(TCA8418_REG_GPIO_INT_STAT_1);
                read_reg(TCA8418_REG_GPIO_INT_STAT_2);
                read_reg(TCA8418_REG_GPIO_INT_STAT_3);

                // clear GPIO IRQ flag
                write_reg(TCA8418_REG_INT_STAT, 2);
            }

            // Key events interrupt
            if (int_stat & 0x01) {
                uint8_t keycode = keypad_scanner_tca8418_get_event();
                if (keycode & 0x80) {
                    printf("Press ");
                    uint8_t key = keycode & 0x7F;
                    if (key <= 81) {  // matrix
                        printf("%d key: %s\n", keycode & 0x7F, key_value_map_str[(keycode & 0x7F) - 1]);
                    } else if (key == 114) {  // GPI
                        printf("%d Fn\n", 114);
                    }
                } else {
                    printf("Release\n");
                }

                // clear the EVENT IRQ flag
                write_reg(TCA8418_REG_INT_STAT, 1);
            }

            // check pending events
            // int intstat = read_reg(TCA8418_REG_INT_STAT);
            // if ((intstat & 0x03) == 0) TCA8418_event = false;
        }
    }
}

esp_err_t keypad_scanner_tca8418_init(i2c_master_bus_handle_t bus_handle)
{
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address  = I2C_DEV_ADDR_TCA8418,
        .scl_speed_hz    = 400000,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &i2c_dev_handle_tca8418));

    // GPIO
    // set default all GIO pins to INPUT
    write_reg(TCA8418_REG_GPIO_DIR_1, 0x00);
    write_reg(TCA8418_REG_GPIO_DIR_2, 0x00);
    write_reg(TCA8418_REG_GPIO_DIR_3, 0x00);

    // add all pins to key events
    write_reg(TCA8418_REG_GPI_EM_1, 0xFF);
    write_reg(TCA8418_REG_GPI_EM_2, 0xFF);
    write_reg(TCA8418_REG_GPI_EM_3, 0xFF);

    // set all pins to FALLING interrupts
    write_reg(TCA8418_REG_GPIO_INT_LVL_1, 0x00);
    write_reg(TCA8418_REG_GPIO_INT_LVL_2, 0x00);
    write_reg(TCA8418_REG_GPIO_INT_LVL_3, 0x00);

    // add all pins to interrupts
    write_reg(TCA8418_REG_GPIO_INT_EN_1, 0xFF);
    write_reg(TCA8418_REG_GPIO_INT_EN_2, 0xFF);
    write_reg(TCA8418_REG_GPIO_INT_EN_3, 0xFF);

    return ESP_OK;
}

static void IRAM_ATTR tca8418_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t)arg;
    xQueueSendFromISR(keypad_evt_queue, &gpio_num, NULL);
}

esp_err_t keypad_scanner_tca8418_irq_init(int int_pin)
{
    gpio_config_t io_conf = {
        .intr_type    = GPIO_INTR_ANYEDGE,  // interrupt of falling edge
        .mode         = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << int_pin),
        .pull_down_en = 0,
        .pull_up_en   = GPIO_PULLUP_ENABLE,
    };
    gpio_config(&io_conf);

    keypad_evt_queue = xQueueCreate(10, sizeof(uint32_t));

    gpio_install_isr_service(ESP_INTR_FLAG_LEVEL1);
    gpio_isr_handler_add(int_pin, tca8418_isr_handler, (void*)int_pin);

    return ESP_OK;
}

/**
 * @brief configures the size of the keypad matrix.
 *
 * @param [in] rows    number of rows, should be <= 8
 * @param [in] columns number of columns, should be <= 10
 * @return true is rows and columns have valid values.
 *
 * @details will always use the lowest pins for rows and columns.
 *          0..rows-1  and  0..columns-1
 */
bool keypad_scanner_tca8418_matrix(uint8_t rows, uint8_t columns)
{
    if ((rows > 8) || (columns > 10)) {
        return false;
    }

    // MATRIX
    // skip zero size matrix
    if ((rows != 0) && (columns != 0)) {
        // setup the keypad matrix.
        uint8_t mask = 0x00;
        for (int r = 0; r < rows; r++) {
            mask <<= 1;
            mask |= 1;
        }
        write_reg(TCA8418_REG_KP_GPIO_1, mask);

        mask = 0x00;
        for (int c = 0; c < columns && c < 8; c++) {
            mask <<= 1;
            mask |= 1;
        }
        write_reg(TCA8418_REG_KP_GPIO_2, mask);

        if (columns > 8) {
            if (columns == 9) {
                mask = 0x01;
            } else {
                mask = 0x03;
            }
            write_reg(TCA8418_REG_KP_GPIO_3, mask);
        }
    }

    return true;
}

/**
 * flushes the internal buffer of key events and cleans the GPIO status registers.
 * @return number of keys flushed.
 */
uint8_t keypad_scanner_tca8418_flush()
{
    //  flush key events
    uint8_t count = 0;

    while (keypad_scanner_tca8418_get_event() != 0) {
        count++;
    }

    // flush gpio events
    read_reg(TCA8418_REG_GPIO_INT_STAT_1);
    read_reg(TCA8418_REG_GPIO_INT_STAT_2);
    read_reg(TCA8418_REG_GPIO_INT_STAT_3);

    // clear INT_STAT register
    write_reg(TCA8418_REG_INT_STAT, 3);

    return count;
}

/**
 * @brief checks if key events are available in the internal buffer
 *
 * @return number of key events in the buffer
 */
uint8_t keypad_scanner_tca8418_available()
{
    uint8_t eventCount = read_reg(TCA8418_REG_KEY_LCK_EC);
    eventCount &= 0x0F;  // lower 4 bits only
    return eventCount;
}

/**
 * gets first event from the internal buffer
 * @return key event or 0 if none available
 * @details
 *     key event 0x00        no event
 *               0x01..0x50  key  press
 *               0x81..0xD0  key  release
 *               0x5B..0x72  GPIO press
 *               0xDB..0xF2  GPIO release
 */
uint8_t keypad_scanner_tca8418_get_event()
{
    uint8_t event = read_reg(TCA8418_REG_KEY_EVENT_A);
    return event;
}

/**
 * @brief enables key event + GPIO interrupts.
 */
void keypad_scanner_tca8418_enable_int()
{
    uint8_t value = read_reg(TCA8418_REG_CFG);
    value |= (TCA8418_REG_CFG_GPI_IEN | TCA8418_REG_CFG_KE_IEN);
    write_reg(TCA8418_REG_CFG, value);
};

/**
 * @brief disables key events + GPIO interrupts.
 */
void keypad_scanner_tca8418_disable_int()
{
    uint8_t value = read_reg(TCA8418_REG_CFG);
    value &= ~(TCA8418_REG_CFG_GPI_IEN | TCA8418_REG_CFG_KE_IEN);
    write_reg(TCA8418_REG_CFG, value);
};

void keypad_scanner_tca8418_clear_irq()
{
    // clear GPIO IRQ flag
    write_reg(TCA8418_REG_INT_STAT, 2);
}

static void write_reg(uint8_t reg, uint8_t val)
{
    uint8_t write_buf[2] = {reg, val};
    i2c_master_transmit(i2c_dev_handle_tca8418, write_buf, 2, I2C_MASTER_TIMEOUT_MS);
}

static uint8_t read_reg(uint8_t reg)
{
    uint8_t ret;
    i2c_master_transmit_receive(i2c_dev_handle_tca8418, &reg, 1, &ret, 1, I2C_MASTER_TIMEOUT_MS);
    return ret;
}

const char key_value_map[81] = {
    '0', '8',  ':', 'c', 'k', 's',
    ' ',  // SHIFT
    ' ',  // NONE
    ' ',  // NONE
    ' ',  // NONE
    '1', '9',  ']', 'd', 'l', 't',
    ' ',  // CTRL
    ' ',  // NONE
    ' ',  // NONE
    ' ',  // NONE
    '2', ' ',  ',', 'e', 'm', 'u',
    ' ',  // CRPH
    ' ',  // ESC
    ' ',  // NONE
    ' ',  // NONE
    '3', '^',  '.', 'f', 'n', 'v',
    ' ',  // CAPS
    ' ',  // TAB
    ' ',  // NONE
    ' ',  // NONE
    // Was the full-width yen sign, whose UTF-8 bytes formed a multi-character
    // constant (GCC 15 rejects that under -Werror). '$' matches the same slot
    // in key_value_map_str below.
    '4', '$', '/', 'g', 'o', 'w',
    ' ',  // KANA
    ' ',  // STOP
    '<',  // <-
    ' ',  // NONE
    '5', '@',  '-', 'h', 'p', 'x',
    ' ',  // NONE
    ' ',  // BS
    ' ',  // UP
    ' ',  // NONE
    '6', '[',  'a', 'i', 'q', 'y',
    ' ',  // NONE
    ' ',  // SEL
    ' ',  // DOWN
    ' ',  // NONE
    '7', ';',  'b', 'j', 'r', 'z',
    ' ',  // NONE
    ' ',  // RET
    '>',  // ->
    ' ',  // NONE
    ' ',  // CTRL
};

const char key_value_map_str[81][10] = {
    "0",       "8", ":", "C", "K", "S",
    " SHIFT ",  // SHIFT
    "NONE",     // NONE
    " ",        // NONE
    "NONE",     // NONE
    "1",       "9", "]", "D", "L", "T",
    " CTRL ",  // CTRL
    "NONE",    // NONE
    "NONE",    // NONE
    "NONE",    // NONE
    "2",       "-", ",", "E", "M", "U",
    " CRPH ",  // CRPH
    " ESC ",   // ESC
    "NONE",    // NONE
    "NONE",    // NONE
    "3",       "^", ".", "F", "N", "V",
    " CAPS ",  // CAPS
    " TAB ",   // TAB
    "NONE",    // NONE
    "NONE",    // NONE
    "4",       "$", "/", "G", "O", "W",
    " KANA ",  // KANA
    " STOP ",  // STOP
    "<-",      // <-
    "NONE",    // NONE
    "5",       "@", "_", "H", "P", "X",
    "NONE",  // NONE
    " BS ",  // BS
    "UP",    // UP
    "NONE",  // NONE
    "6",       "[", "A", "I", "Q", "Y",
    "NONE",    // NONE
    " SEL ",   // SEL
    " DOWN ",  // DOWN
    "NONE",    // NONE
    "7",       ";", "B", "J", "R", "Z",
    "NONE",   // NONE
    " RET ",  // RET
    "->",     // ->
    "NONE",   // NONE
    " FN ",   // CTRL
};

#include "led_strip.h"

#define TAB5_STRIP_LED_PIN   49
#define TAB5_STRIP_LED_COUNT 2

// 10MHz resolution, 1 tick = 0.1us (led strip needs a high resolution)
#define LED_STRIP_RMT_RES_HZ (10 * 1000 * 1000)

static led_strip_handle_t led_strip;

esp_err_t bsp_rgb_led_init(void)
{
    // LED strip general initialization, according to your led board design
    led_strip_config_t strip_config = {
        .strip_gpio_num         = TAB5_STRIP_LED_PIN,    // The GPIO that connected to the LED strip's data line
        .max_leds               = TAB5_STRIP_LED_COUNT,  // The number of LEDs in the strip,
        .led_model              = LED_MODEL_WS2812,      // LED strip model
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB,  // The color order of the strip: GRB
        .flags                  = {
            .invert_out = false,  // don't invert the output signal
        }};

    // LED strip backend configuration: RMT
    led_strip_rmt_config_t rmt_config = {
        .clk_src           = RMT_CLK_SRC_DEFAULT,   // different clock source can lead to different power consumption
        .resolution_hz     = LED_STRIP_RMT_RES_HZ,  // RMT counter clock frequency
        .mem_block_symbols = 64,                    // the memory size of each RMT channel, in words (4 bytes)
        .flags             = {
            .with_dma = false,  // DMA feature is available on chips like ESP32-S3/P4
        }};

    // LED Strip object handle
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    // ESP_LOGI(TAG, "Created LED strip object with RMT backend");

    return ESP_OK;
}

/* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
void bsp_rgb_led_set(uint8_t led_num, uint8_t r, uint8_t g, uint8_t b)
{
    led_strip_set_pixel(led_strip, led_num, r, g, b);
}

/* Refresh the strip to send data */
void bsp_rgb_led_display()
{
    led_strip_refresh(led_strip);
}

/* Set all LED off to clear all pixels */
void bsp_rgb_led_clear()
{
    led_strip_clear(led_strip);
}
