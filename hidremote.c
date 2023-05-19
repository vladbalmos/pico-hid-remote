#include <stdio.h>
#include "pico/cyw43_arch.h"
#include "pico/sleep.h"
#include "hardware/clocks.h"
#include "pico/util/queue.h"
#include "bt.h"
#include "buttons.h"
#include "control.h"
#include "debug.h"

static uint8_t pins[] = {FN_BTN_PIN, PLAY_CTRL_BTN_PIN, VOL_UP_BTN_PIN, VOL_DOWN_BTN_PIN};
static queue_t ctrl_ev_w_queue;
static queue_t ctrl_ev_r_queue;

extern uint8_t ctrl_is_connected;
extern absolute_time_t ctrl_disconnected_at;
extern absolute_time_t last_command_at;

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

    btn = btn_get(btn_pin);
    btn->pin = btn_pin;
    
    if (btn_action == BTN_PRESS) {
        btn->pressed_at = get_absolute_time();
    } else {
        btn->released_at = get_absolute_time();
    }
}

static int8_t deep_sleep() {
    // flush stdio
    uart_default_tx_wait_blocking();
    
    // prepare for sleep
    bt_deinit();
    cyw43_arch_deinit();
    ctrl_deinit();
    
    // sleep until GPIO input
    sleep_run_from_rosc();
    sleep_goto_dormant_until_edge_high(PLAY_CTRL_BTN_PIN);
    
    // At this point we've woken up

    // Reset the clocks
    clocks_init();
    
    // Re-init stdio
    stdio_init_all();
    DEBUG("Woken up\n");
    uart_default_tx_wait_blocking();

    // Initialize BT
    if (cyw43_arch_init()) {
        DEBUG("Failed to initialise cyw43_arch\n");
        return -1;
    }
    bt_init(&ctrl_ev_r_queue, &ctrl_ev_w_queue);
    ctrl_init(&ctrl_ev_w_queue, &ctrl_ev_r_queue);
    return 0;
}


int main() {
    stdio_init_all();

    if (cyw43_arch_init()) {
        printf("failed to initialise cyw43_arch\n");
        return -1;
    }
    
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

    // init controller
    ctrl_init(&ctrl_ev_w_queue, &ctrl_ev_r_queue);
    
    // init buttons
    btn_create_array();

    button_t *btn;

    absolute_time_t now = nil_time;
    uint32_t time_diff = 0;

    DEBUG("Remote initialized\n");
    while (1) {
        bt_process_queue();
        ctrl_process_queue();
        now = get_absolute_time();

        if (!ctrl_is_connected) {
            time_diff = absolute_time_diff_us(ctrl_disconnected_at, now) / 1000;
            if (time_diff >= CTRL_DEEP_SLEEP_TIMEOUT_MS) {
                DEBUG("Going to sleep\n");
                deep_sleep();
                continue;
            }
        } else if (last_command_at) {
            time_diff = absolute_time_diff_us(last_command_at, now) / 1000;
            if (time_diff >= CTRL_CMD_TIMEOUT_MS) {
                DEBUG("Going to sleep\n");
                deep_sleep();
                continue;
            }
        }
        
        btn = btn_get(FN_BTN_PIN);

        if (btn_is_pressed(btn)) {
            if (btn_is_long_press(btn))  {
                DEBUG("Going to sleep\n");
                if (deep_sleep() == -1) {
                    // Unable to re-initialize hardware
                    panic("Unable to wake up from sleep!");
                    return -1;
                }

                DEBUG("Showing BT LED activity\n");
                goto SLEEP;
            } else if (btn_is_double_press(btn)) {
                DEBUG("Showing battery level\n");
            }
            goto SLEEP;
        }

        btn = btn_get(PLAY_CTRL_BTN_PIN);
        if (btn_is_pressed(btn)) {
            btn_handled(btn, get_absolute_time());
            ctrl_toggle_play_pause();
            goto SLEEP;
        }

        btn = btn_get(VOL_UP_BTN_PIN);
        if (btn_is_pressed(btn)) {
            btn_handled(btn, get_absolute_time());
            ctrl_vol_up();
            goto SLEEP;
        }

        btn = btn_get(VOL_DOWN_BTN_PIN);
        if (btn_is_pressed(btn)) {
            btn_handled(btn, get_absolute_time());
            ctrl_vol_down();
            goto SLEEP;
        }

        SLEEP:
            sleep_ms(10);
    }
    cyw43_arch_deinit();
    return 0;
}