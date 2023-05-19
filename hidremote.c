#include <stdio.h>
#include "pico/cyw43_arch.h"
#include "pico/sleep.h"
#include "hardware/clocks.h"
#include "hardware/pll.h"
#include "hardware/xosc.h"
#include "hardware/rosc.h"
#include "pico/util/queue.h"
#include "bt.h"
#include "buttons.h"
#include "control.h"
#include "battery.h"
#include "debug.h"

#define ADC_PIN 26
#define MICRO_SLEEP_MS 10
#define BATTERY_CHECK_INTERVAL_S 10

static uint8_t pins[] = {FN_BTN_PIN, PLAY_CTRL_BTN_PIN, VOL_UP_BTN_PIN, VOL_DOWN_BTN_PIN};
static queue_t ctrl_ev_w_queue;
static queue_t ctrl_ev_r_queue;
static uint8_t bat_level;

extern uint8_t ctrl_is_connected;
extern absolute_time_t ctrl_disconnected_at;
extern absolute_time_t ctrl_last_command_at;

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
    // Flush stdio
    uart_default_tx_wait_blocking();
    
    // Prepare for sleep
    bt_deinit();
    cyw43_arch_deinit();
    ctrl_deinit();
    
    
    // Sleep until GPIO input
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
    bat_level = battery_level();
    bt_init(&ctrl_ev_r_queue, &ctrl_ev_w_queue, bat_level);
    ctrl_init(&ctrl_ev_w_queue, &ctrl_ev_r_queue);
    return 0;
}


int main() {
    button_t *btn;
    absolute_time_t now = nil_time;
    uint32_t now_s;
    uint32_t time_diff = 0;
    uint8_t battery_level_published = 0;

    stdio_init_all();

    // Init Bluetooth chipset
    if (cyw43_arch_init()) {
        printf("failed to initialise cyw43_arch\n");
        // TODO: check if power goes down when returning early
        return -1;
    }
    
    // Init gpio pins
    for (uint8_t i = 0; i < 4; i++) {
        gpio_init(pins[i]);
        gpio_set_dir(pins[i], GPIO_IN);
        gpio_set_irq_enabled_with_callback(pins[i], GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    }
    
    queue_init(&ctrl_ev_w_queue, sizeof(ctrl_ev_t), 10);
    queue_init(&ctrl_ev_r_queue, sizeof(ctrl_ev_t), 10);

    // Init battery readings
    battery_init(ADC_PIN);
    bat_level = battery_level();
    
    // Init btstack
    bt_init(&ctrl_ev_r_queue, &ctrl_ev_w_queue, bat_level);

    // Init controller
    ctrl_init(&ctrl_ev_w_queue, &ctrl_ev_r_queue);
    
    // Init buttons
    btn_create_array();

    DEBUG("Remote initialized\n");
    while (1) {
        bt_process_queue();
        ctrl_process_queue();
        now = get_absolute_time();
        now_s = to_ms_since_boot(now) / 1000;
        
        if (bat_level <= 2) {
            DEBUG("Battery low! Please recharge! Stopping!\n");
            uart_default_tx_wait_blocking();
            break;
        }

        if (!ctrl_is_connected) {
            time_diff = absolute_time_diff_us(ctrl_disconnected_at, now) / 1000;
            if (time_diff >= CTRL_DEEP_SLEEP_TIMEOUT_MS) {
                DEBUG("Going to sleep due to lack of remote connection\n");
                deep_sleep();
                continue;
            }
        } else {
            // Check battery every 10 seconds
            if ((now_s % BATTERY_CHECK_INTERVAL_S) == 0) {
                if (!battery_level_published) {
                    bat_level = battery_level();
                    bt_set_battery_level(bat_level);
                    battery_level_published = 1;
                    DEBUG("Publishing battery level %d\n", bat_level);
                }
            } else {
                battery_level_published = 0;
            }

            if (ctrl_last_command_at) {
                time_diff = absolute_time_diff_us(ctrl_last_command_at, now) / 1000;
                if (time_diff >= CTRL_CMD_TIMEOUT_MS) {
                    DEBUG("Going to sleep due to inactivity\n");
                    deep_sleep();
                    continue;
                }
            }
        }
        
        btn = btn_get(FN_BTN_PIN);

        if (btn_is_pressed(btn)) {
            if (btn_is_long_press(btn))  {
                btn_handled(btn, now);
                DEBUG("Going to sleep\n");
                if (deep_sleep() == -1) {
                    // Unable to re-initialize hardware
                    panic("Unable to safely wake up from sleep!");
                    break;
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
            DEBUG("Bat: %d\n", battery_level())
            btn_handled(btn, now);
            ctrl_toggle_play_pause();
            goto SLEEP;
        }

        btn = btn_get(VOL_UP_BTN_PIN);
        if (btn_is_pressed(btn)) {
            btn_handled(btn, now);
            ctrl_vol_up();
            goto SLEEP;
        }

        btn = btn_get(VOL_DOWN_BTN_PIN);
        if (btn_is_pressed(btn)) {
            btn_handled(btn, now);
            ctrl_vol_down();
            goto SLEEP;
        }

        SLEEP:
            sleep_ms(MICRO_SLEEP_MS);
    }
    
    // Stop everything
    bt_deinit();
    ctrl_deinit();
    cyw43_arch_deinit();
    
    clock_configure(clk_ref,
                    CLOCKS_CLK_REF_CTRL_SRC_VALUE_ROSC_CLKSRC_PH,
                    0,
                    1 * MHZ,
                    1 * MHZ);
    clock_configure(clk_sys,
                    CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLK_REF,
                    0,
                    1 * MHZ,
                    1 * MHZ);

    xosc_disable();
    rosc_disable();
    clock_stop(clk_peri);
    clock_stop(clk_usb);
    clock_stop(clk_adc);
    clock_stop(clk_rtc);
    pll_deinit(pll_sys);
    pll_deinit(pll_usb);
    clock_stop(clk_sys);
    clock_stop(clk_ref);
    return 0;
}