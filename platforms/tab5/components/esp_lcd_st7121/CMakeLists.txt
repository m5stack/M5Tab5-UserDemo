idf_component_register(SRCS "esp_lcd_st7121.c"
                       INCLUDE_DIRS "include"
                       REQUIRES "esp_lcd" "esp_driver_gpio")

# NOTE: this component deliberately does NOT call cu_pkg_define_version().
# That helper reads the version out of idf_component.yml and injects
# -D<NAME>_VER_{MAJOR,MINOR,PATCH} as target compile options, but
# include/esp_lcd_st7121.h already defines ESP_LCD_ST7121_VER_* itself, and this
# directory has no idf_component.yml, so the injected macros expanded to empty
# and GCC 15 rejected the redefinition under -Werror. The header is the single
# source of truth for the version here.
