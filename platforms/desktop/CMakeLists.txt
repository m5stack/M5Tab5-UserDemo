cmake_minimum_required (VERSION 3.10)
project(DesktopBuild)

cmake_policy(SET CMP0077 NEW)
cmake_policy(SET CMP0079 NEW)  # Allow linking to targets from other directories
set(CMAKE_CXX_STANDARD 17)

# Set UTF-8 encoding for MSVC to support Unicode
if(MSVC)
    add_compile_options(/utf-8)
    # Enable math constants like M_PI
    add_definitions(-D_USE_MATH_DEFINES)
endif()

add_definitions(-DPLATFORM_BUILD_DESKTOP)

# Mooncake 
set(MOONCAKE_BUILD_EXAMPLE OFF)
add_subdirectory(dependencies/mooncake)

# Mooncake log
set(MOONCAKE_EVENT_BUILD_EXAMPLE OFF)
add_subdirectory(dependencies/mooncake_log)

# Smooth ui shit
set(SMOOTH_UI_TOOLKIT_BUILD_EXAMPLE OFF)
add_subdirectory(dependencies/smooth_ui_toolkit)
# Apply MSVC-specific definitions to smooth_ui_toolkit
if(MSVC)
    target_compile_definitions(smooth_ui_toolkit PRIVATE _USE_MATH_DEFINES)
endif()

# App layer
file(GLOB_RECURSE APP_LAYER_SRCS
    app/*.c
    app/*.cc
    app/*.cpp
)
set(APP_LAYER_INCS
    app/
)

# SDL - find before lvgl so we can link it to lvgl
find_package(SDL2 CONFIG REQUIRED)
# vcpkg provides SDL2 as CMake targets, not variables
# SDL2::SDL2 or SDL2::SDL2-static will be available

# Lvgl
set(LV_CONF_INCLUDE_SIMPLE OFF)
set(LV_CONF_PATH ../../lv_conf.h)
add_subdirectory(dependencies/lvgl)

# Link SDL2 to lvgl so SDL driver can find SDL2 headers
target_link_libraries(lvgl PUBLIC 
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)

# App layer
file(GLOB_RECURSE APP_LAYER_SRCS
    app/*.c
    app/*.cc
    app/*.cpp
)
# Ensure .c files are compiled as C, not C++
file(GLOB_RECURSE APP_LAYER_C_SRCS app/*.c)
set_source_files_properties(${APP_LAYER_C_SRCS} PROPERTIES LANGUAGE C)
set(APP_LAYER_INCS
    app/
)

# 桌面端源文件
file(GLOB_RECURSE APP_DESKTOP_BUILD_SRCS
    platforms/desktop/*.cpp
    platforms/desktop/*.cc
    platforms/desktop/*.c
)
add_executable(app_desktop_build ${APP_DESKTOP_BUILD_SRCS} ${APP_LAYER_SRCS})
target_include_directories(app_desktop_build PUBLIC ${APP_LAYER_INCS})
target_link_libraries(app_desktop_build PUBLIC 
    mooncake 
    mooncake_log
    lvgl 
    lvgl_examples 
    lvgl_demos 
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
    smooth_ui_toolkit
)

# Link pthread only on non-Windows platforms
if(NOT WIN32)
    target_link_libraries(app_desktop_build PUBLIC pthread)
endif()

# 设置构建路径
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build/desktop)

# 确保构建路径存在
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR})

# 设置生成的可执行文件输出路径
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

# # 设置生成的库文件输出路径
# set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})
