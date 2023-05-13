cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)

project(bt-experiments VERSION 0.1 LANGUAGES C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        )

add_executable(bt-experiments
    debug.c
    main.c
)

pico_enable_stdio_usb(bt-experiments 0)
pico_enable_stdio_uart(bt-experiments 1)

# pull in common dependencies
target_link_libraries(bt-experiments
    pico_stdlib
    pico_btstack_ble
    pico_btstack_classic
    pico_btstack_cyw43
    pico_cyw43_arch_none
    )

target_include_directories(bt-experiments PRIVATE
    ${CMAKE_CURRENT_LIST_DIR} # For btstack config
    )

pico_add_extra_outputs(bt-experiments)
