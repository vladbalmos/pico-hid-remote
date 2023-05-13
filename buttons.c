#include <stdio.h>
#include "pico/stdlib.h"
#include "buttons.h"


static uint8_t btn_state[] = {BTN_RELEASE, BTN_RELEASE, BTN_RELEASE, BTN_RELEASE};
static absolute_time_t debounce_due_press[] = {0, 0, 0, 0};
static absolute_time_t debounce_due_release[] = {0, 0, 0, 0};

void btn_debounce(uint8_t pin, uint32_t event_mask, int8_t *confirm_pin, uint8_t *btn_action) {
    absolute_time_t *debounce_due = NULL;
    absolute_time_t _debounce_due;
    absolute_time_t now = get_absolute_time();
    uint8_t btn_index = pin - BTN_MIN_PIN;
    uint8_t last_state = btn_state[btn_index];
    uint8_t new_state = (event_mask == GPIO_IRQ_EDGE_RISE) ? BTN_PRESS : BTN_RELEASE;
    
    if (last_state == new_state) {
        return;
    }
    
    if (new_state == BTN_PRESS) {
        debounce_due = debounce_due_press;
    } else {
        debounce_due = debounce_due_release;
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

void btn_create_array(button_t *btns, uint8_t btn_count) {
    for (uint8_t i = 0; i < btn_count; i++) {
        button_t btn;
        btn.pin = -1;
        btn.pressed_at = nil_time;
        btn.released_at = nil_time;
        
        btns[i] = btn;
    }
}