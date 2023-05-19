#include <stdlib.h>
#include "buttons.h"

static button_t btns[4];
static uint8_t btn_state[] = {BTN_RELEASE, BTN_RELEASE, BTN_RELEASE, BTN_RELEASE};
static absolute_time_t debounce_due[] = {0, 0, 0, 0};

static uint8_t btn_get_index(uint8_t pin) {
    return pin - BTN_MIN_PIN;
}

button_t *btn_get(uint8_t pin) {
    button_t *btn;

    uint8_t btn_index = btn_get_index(pin);
    btn = &btns[btn_index];
    return btn;
}


void btn_debounce(uint8_t pin, uint32_t event_mask, int8_t *confirm_pin, uint8_t *btn_action) {
    absolute_time_t _debounce_due;
    absolute_time_t now = get_absolute_time();
    uint8_t btn_index = btn_get_index(pin);
    uint8_t last_state = btn_state[btn_index];
    uint8_t new_state = (event_mask == GPIO_IRQ_EDGE_RISE) ? BTN_PRESS : BTN_RELEASE;
    
    if (last_state == new_state) {
        return;
    }
    
    _debounce_due = debounce_due[btn_index];
    
    if (now < _debounce_due) {
        return;
    }
    
    btn_state[btn_index] = new_state;
    debounce_due[btn_index] = delayed_by_ms(now, DEBOUNCE_TIME_MS);

    *confirm_pin = pin;
    *btn_action = new_state;
}

void btn_create_array() {
    for (uint8_t i = 0; i < 4; i++) {
        button_t btn;
        btn.pin = -1;
        btn.pressed_at = nil_time;
        btn.released_at = nil_time;
        
        btns[i] = btn;
    }
}

void btn_handled(button_t *btn, absolute_time_t now) {
    btn->released_at = now;
}

uint8_t btn_is_pressed(button_t *btn) {
    return btn->pressed_at > btn->released_at;
}

uint8_t btn_is_long_press(button_t *btn) {
    absolute_time_t now = get_absolute_time();
    uint32_t diff = absolute_time_diff_us(btn->pressed_at, now) / 1000;
    
    if (diff >= LONG_PRESS_TIME_MS) {
        return 1;
    }
    
    return 0;
}

uint8_t btn_is_double_press(button_t *btn) {
    absolute_time_t now = get_absolute_time();
    uint32_t diff = absolute_time_diff_us(btn->released_at, btn->pressed_at) / 1000;

    if (diff <= DOUBLE_PRESS_TIME_MS) {
        btn_handled(btn, now);
        return 1;
    }
    
    return 0;
}