# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

set(EXTRA_COMPONENT_DIRS 
    "../../dependencies"
) 

project(m5stack_tab5)

# LVGL declares hot functions with LV_ATTRIBUTE_FAST_MEM in both the header and
# the definition. On ESP-IDF the macro is IRAM_ATTR, which expands to
# `section(".iram1.<__COUNTER__>")`, so the declaration and the definition land
# in two differently-numbered sections of the same `.iram1` group. GCC 15 (which
# ships with IDF 6.x) reports that as -Wattributes, and IDF builds with -Werror.
# LVGL lives in the fetched `dependencies/` tree, so relax just this one warning
# for that one component instead of patching it.
idf_build_get_property(__tab5_build_components BUILD_COMPONENTS)
if("lvgl" IN_LIST __tab5_build_components)
    idf_component_get_property(__tab5_lvgl_lib lvgl COMPONENT_LIB)
    target_compile_options(${__tab5_lvgl_lib} PRIVATE "-Wno-error=attributes")
endif()

# IDF 6.x raises the C++ standard from gnu++2b to gnu++26 (GCC 15). In C++20,
# `[=]` no longer implicitly captures `this`; smooth_ui_toolkit's spring.cpp
# still does that, which is now -Wdeprecated caught by -Werror. Same reasoning
# as above: it is fetched into `dependencies/`, so relax the one warning for the
# one component rather than patching third-party sources.
if("smooth_ui_toolkit" IN_LIST __tab5_build_components)
    idf_component_get_property(__tab5_sut_lib smooth_ui_toolkit COMPONENT_LIB)
    target_compile_options(${__tab5_sut_lib} PRIVATE "-Wno-error=deprecated")
endif()
