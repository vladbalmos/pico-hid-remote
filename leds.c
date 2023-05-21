#include "pico/stdlib.h"
#include "leds.h"
#include "debug.h"

static uint8_t leds[4] = {GREEN_LED_MIN_PIN, GREEN_LED_MED_PIN, GREEN_LED_MAX_PIN, BLUE_LED_PIN};

static uint8_t bt_connecting_indicator_on = 0;
static uint8_t bt_connecting_showing = 0;
static alarm_id_t flash_bt_connecting_alarm = 0;

static uint8_t battery_level_showing = 0;
static uint8_t led_bat_indicator_on = 0;
static alarm_id_t hide_battery_level_alarm = 0;
static alarm_id_t flash_battery_level_alarm = 0;

void leds_init() {
    for (uint8_t i = 0; i < 4; i++) {
        gpio_init(leds[i]);
        gpio_set_dir(leds[i], GPIO_OUT);
        gpio_put(leds[i], 0);
    }
}

void leds_turnoff() {
    cancel_alarm(flash_battery_level_alarm);
    cancel_alarm(hide_battery_level_alarm);
    cancel_alarm(flash_bt_connecting_alarm);
    
    for (uint8_t i = 0; i < 4; i++) {
        gpio_put(leds[i], 0);
    }
    battery_level_showing = 0;
    led_bat_indicator_on = 0;
    
    bt_connecting_indicator_on = 0;
    bt_connecting_showing = 0;
}

static int64_t hide_battery_level(alarm_id_t id, void *user_data) {
    cancel_alarm(flash_battery_level_alarm);
    for (uint8_t i = 0; i < 3; i++) {
        gpio_put(leds[i], 0);
    }
    battery_level_showing = 0;

    return 0;
}

static int64_t flash_battery_level(alarm_id_t id, void *user_data) {
    uint8_t led_state = (led_bat_indicator_on) ? 0 : 1;
    led_bat_indicator_on = led_state;

    gpio_put(GREEN_LED_MIN_PIN, led_state);
    
    flash_battery_level_alarm = add_alarm_in_ms(FLASH_LED_INTERVAL, flash_battery_level, NULL, false);
    
    return 0;
}

static int64_t flash_bt_connecting(alarm_id_t id, void *user_data) {
    uint8_t led_state = (bt_connecting_indicator_on) ? 0 : 1;
    bt_connecting_indicator_on = led_state;

    gpio_put(BLUE_LED_PIN, led_state);
    
    flash_bt_connecting_alarm = add_alarm_in_ms(FLASH_LED_INTERVAL, flash_bt_connecting, NULL, false);
    return 0;
}

void leds_show_battery_level(uint8_t level) {
    if (battery_level_showing) {
        return;
    }

    battery_level_showing = 1;

    uint8_t leds_count = 1;

    if (level >= 70) {
        leds_count = 3;
    } else if (level >= 30 && level < 70) {
        leds_count = 2;
    }

    for (uint8_t i = 0; i < leds_count; i++) {
        gpio_put(leds[i], 1);
    }

    hide_battery_level_alarm = add_alarm_in_ms(HIDE_BAT_LEVEL_TIMEOUT_MS, hide_battery_level, NULL, false);
    
    if (level < 5) {
        led_bat_indicator_on = 1;
        flash_battery_level_alarm = add_alarm_in_ms(FLASH_LED_INTERVAL, flash_battery_level, NULL, false);
    }
}

void leds_show_bt_connecting() {
    if (bt_connecting_showing) {
        return;
    }
    
    bt_connecting_showing = 1;
    bt_connecting_indicator_on = 1;

    gpio_put(BLUE_LED_PIN, 1);
    flash_bt_connecting_alarm = add_alarm_in_ms(FLASH_LED_INTERVAL, flash_bt_connecting, NULL, false);
}

void leds_hide_bt_connecting() {
    cancel_alarm(flash_bt_connecting_alarm);
    gpio_put(BLUE_LED_PIN, 0);
    
    bt_connecting_showing = 0;
    bt_connecting_indicator_on = 0;
}