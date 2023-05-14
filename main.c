/**
 * Copyright (c) 2022 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "debug.h"
#include "bt.h"
#include "buttons.h"

static uint8_t pins[] = {FN_BTN_PIN, PLAY_CTRL_BTN_PIN, VOL_UP_BTN_PIN, VOL_DOWN_BTN_PIN};
static button_t btns[4];

static button_t *get_btn(uint8_t pin) {
    button_t *btn;

    uint8_t btn_index = btn_get_index(pin);
    btn = &btns[btn_index];
    return btn;
}

static void gpio_irq_handler(uint gpio, uint32_t event) {
    if (event == (GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL)) {
        return;
    }

    button_t *btn;
    uint8_t btn_action = 0;
    int8_t btn_pin = -1;
    
    btn_debounce(gpio, event, &btn_pin, &btn_action);
    
    if (btn_pin == -1) {
        return;
    }

    btn = get_btn(btn_pin);
    btn->pin = btn_pin;
    
    if (btn_action == BTN_PRESS) {
        btn->pressed_at = get_absolute_time();
        btn->released_at = nil_time;
    } else {
        btn->released_at = get_absolute_time();
    }

    printf("Presed_at %llu. Released at %llu\n", btn->pressed_at, btn->released_at);
}

int main() {
    stdio_init_all();

    if (cyw43_arch_init()) {
        printf("failed to initialise cyw43_arch\n");
        return -1;
    }
    
    btn_create_array(btns, 4);
    
    // init gpio pins
    for (uint8_t i = 0; i < 4; i++) {
        gpio_init(pins[i]);
        gpio_set_dir(pins[i], GPIO_IN);
        gpio_set_irq_enabled_with_callback(pins[i], GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    }
    
    // init btstack
    bt_init();
    
    while (1) {
        sleep_ms(100);
    }
    cyw43_arch_deinit();
    return 0;
}