/**
 * Copyright (c) 2022 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "pico/sleep.h"
#include "hardware/clocks.h"
#include "hardware/rosc.h"
#include "hardware/structs/scb.h"
#include "hardware/pll.h"
#include "pico/util/queue.h"
#include "debug.h"
#include "bt.h"
#include "buttons.h"
#include "control.h"

static uint8_t pins[] = {FN_BTN_PIN, PLAY_CTRL_BTN_PIN, VOL_UP_BTN_PIN, VOL_DOWN_BTN_PIN};
static button_t btns[4];
static queue_t ctrl_ev_w_queue;
static queue_t ctrl_ev_r_queue;

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

    // DEBUG("Btn %d. Presed at %llu. Released at %llu\n", btn->pin, btn->pressed_at, btn->released_at);
}


void measure_freqs(void) {
    uint f_pll_sys = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_PLL_SYS_CLKSRC_PRIMARY);
    uint f_pll_usb = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_PLL_USB_CLKSRC_PRIMARY);
    uint f_rosc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_ROSC_CLKSRC);
    uint f_clk_sys = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_SYS);
    uint f_clk_peri = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_PERI);
    uint f_clk_usb = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_USB);
    uint f_clk_adc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_ADC);
    uint f_clk_rtc = frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_RTC);

    printf("------------------- FREQ-------------\n");
    printf("\tll_sys  = %dkHz\n", f_pll_sys);
    printf("\tpll_usb  = %dkHz\n", f_pll_usb);
    printf("\trosc     = %dkHz\n", f_rosc);
    printf("\tclk_sys  = %dkHz\n", f_clk_sys);
    printf("\tclk_peri = %dkHz\n", f_clk_peri);
    printf("\tclk_usb  = %dkHz\n", f_clk_usb);
    printf("\tclk_adc  = %dkHz\n", f_clk_adc);
    printf("\tclk_rtc  = %dkHz\n", f_clk_rtc);
    printf("------------------- END FREQ---------\n");
}

int main() {
    stdio_init_all();
    // Get the current system clock frequency
    // uint current_sys_clk = clock_get_hz(clk_sys);
    // DEBUG("%d\n", current_sys_clk);

    // Set the system clock to a lower frequency, e.g., half of its current speed

    uint scb_orig = scb_hw->scr;
    uint clock0_orig = clocks_hw->sleep_en0;
    uint clock1_orig = clocks_hw->sleep_en1;


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
    
    queue_init(&ctrl_ev_w_queue, sizeof(ctrl_ev_t), 10);
    queue_init(&ctrl_ev_r_queue, sizeof(ctrl_ev_t), 10);
    
    // init btstack
    bt_init(&ctrl_ev_r_queue, &ctrl_ev_w_queue);
    ctrl_init(&ctrl_ev_w_queue, &ctrl_ev_r_queue);
    
    button_t *btn;
    absolute_time_t fn_btn_last_release = nil_time;
    
    while (1) {
        // measure_freqs();
        bt_process_queue();
        ctrl_process_queue();
        btn = get_btn(FN_BTN_PIN);

        if (btn_is_pressed(btn)) {
            if (btn_is_long_press(btn))  {
                ctrl_make_discoverable(1);
            } else if (btn_is_double_press(btn, &fn_btn_last_release)) {
                ctrl_connect();
            }
            goto SLEEP;
        }
        
        if (btn_is_released(btn) && fn_btn_last_release == nil_time) {
            fn_btn_last_release = get_absolute_time();
        }
        
        btn = get_btn(PLAY_CTRL_BTN_PIN);
        if (btn_is_pressed(btn)) {
            // TODO: goto sleep if not paired in one minute
            bt_deinit();
            cyw43_arch_deinit();
            DEBUG("DEBUG_SLEEPING");
            uart_default_tx_wait_blocking();
            sleep_run_from_rosc();
            sleep_goto_dormant_until_edge_high(VOL_DOWN_BTN_PIN);
            

            clocks_init();
            stdio_init_all();
            DEBUG("wokeup\n");
            uart_default_tx_wait_blocking();
            if (cyw43_arch_init()) {
                printf("failed to initialise cyw43_arch\n");
                return -1;
            }
            bt_init(&ctrl_ev_r_queue, &ctrl_ev_w_queue);
            // if (cyw43_arch_init()) {
            //     printf("failed to initialise cyw43_arch\n");
            //     return -1;
            // }

            // bt_init(&ctrl_ev_r_queue, &ctrl_ev_w_queue);

            // ctrl_toggle_play_pause();
            btn_handled(btn, nil_time);
            goto SLEEP;
        }

        btn = get_btn(VOL_UP_BTN_PIN);
        if (btn_is_pressed(btn)) {
            ctrl_vol_up();
            btn_handled(btn, nil_time);
            goto SLEEP;
        }

        btn = get_btn(VOL_DOWN_BTN_PIN);
        if (btn_is_pressed(btn)) {
            ctrl_vol_down();
            btn_handled(btn, nil_time);
            goto SLEEP;
        }
        
        
        SLEEP:
            sleep_ms(10);
    }
    cyw43_arch_deinit();
    return 0;
}