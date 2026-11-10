
# IDF >= 6.0 no longer exposes esp_driver_* publicly through the legacy `driver`
# component, so each driver used by this BSP must be listed explicitly.
idf_component_register(
    SRCS "m5stack_tab5.c" "esp_lcd_st7123.c"
    INCLUDE_DIRS "include"
    INCLUDE_DIRS "include/bsp"
    PRIV_INCLUDE_DIRS "priv_include"
    REQUIRES
        esp_driver_gpio
        esp_driver_i2c
        esp_driver_i2s
        esp_driver_ledc
        esp_driver_sdmmc
        esp_lcd
        esp_codec_dev
        esp_lvgl_port
        esp_lcd_ili9881c
        esp_lcd_touch_st7123
        esp_lcd_st7121
    PRIV_REQUIRES usb spiffs fatfs
)
