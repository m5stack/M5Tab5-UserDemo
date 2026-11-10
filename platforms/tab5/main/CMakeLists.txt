file(GLOB_RECURSE APP_LAYER_SRCS
    ../../../app/*.c
    ../../../app/*.cc
    ../../../app/*.cpp
)

set(APP_LAYER_INCS
    ../../../app
)

file(GLOB_RECURSE MY_HAL_SRCS
    ./hal/*.c
    ./hal/*.cc
    ./hal/*.cpp
)

idf_component_register(SRCS "app_main.cpp" ${APP_LAYER_SRCS} ${MY_HAL_SRCS}
                    INCLUDE_DIRS "." ${APP_LAYER_INCS}
                    EMBED_TXTFILES "../audio/canon_in_d.mp3" "../audio/startup_sfx.mp3" "../audio/shutdown_sfx.mp3")

# IDF 6 grew fields on a number of driver structs (i2c_device_config_t,
# audio_player_config_t, v4l2_pix_format, ppa_client_config_t,
# ppa_srm_oper_config_t, ...). This layer initializes them with designated
# initializers that name only the fields it cares about; C++ zero-initializes
# the rest, which is the documented default for every one of those fields. GCC
# 15 warns about the omitted members and IDF turns warnings into errors, so
# relax just that warning here rather than padding every initializer with
# `.field = {}` noise that has to be maintained as IDF grows structs again.
target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-missing-field-initializers")
