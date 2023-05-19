cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
include(pico_extras_import_optional.cmake)

project(hidremote VERSION 0.1 LANGUAGES C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        )

add_executable(hidremote
    debug.c
    bt.c
    buttons.c
    control.c
    hidremote.c
    battery.c
)

pico_btstack_make_gatt_header(hidremote INTERFACE ${CMAKE_CURRENT_LIST_DIR}/hidremote.gatt)

pico_enable_stdio_usb(hidremote 0)
pico_enable_stdio_uart(hidremote 1)

# pull in common dependencies
target_link_libraries(hidremote
    pico_stdlib
    pico_malloc
    pico_btstack_ble
    pico_btstack_cyw43
    pico_cyw43_arch_none
    hardware_sleep
    hardware_adc
    )

if(DEFINED ENV{DEBUG_MODE})
    target_compile_definitions(hidremote PUBLIC
        DEBUG_MODE
    )
endif()

if(DEFINED ENV{BT_DEBUG_MODE})
    target_compile_definitions(hidremote PUBLIC
        ENABLE_LOG_DEBUG
        ENABLE_LOG_INFO
        ENABLE_LOG_ERROR
        BT_DEBUG_MODE
    )
endif()

target_include_directories(hidremote PRIVATE
    ${CMAKE_CURRENT_LIST_DIR} # For btstack config
    )

pico_add_extra_outputs(hidremote)
