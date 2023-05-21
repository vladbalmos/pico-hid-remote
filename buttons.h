#include "pico/stdlib.h"

#define BTN_MIN_PIN 18

#define FN_BTN_PIN 18
#define PLAY_CTRL_BTN_PIN 19
#define VOL_UP_BTN_PIN 20
#define VOL_DOWN_BTN_PIN 21

#define BTN_PRESS 1
#define BTN_RELEASE 0

#define DEBOUNCE_TIME_MS 25
#define LONG_PRESS_TIME_MS 2000
#define DOUBLE_PRESS_TIME_MS 150

typedef struct {
    int8_t pin;
    absolute_time_t pressed_at;
    absolute_time_t released_at;
} button_t;


button_t *btn_get(uint8_t pin);
void btn_debounce(uint8_t pin, uint32_t event_mask, int8_t *confirm_pin, uint8_t *btn_action);
void btn_create_array();

uint8_t btn_is_pressed(button_t *btn);
uint8_t btn_is_long_press(button_t *btn);
uint8_t btn_is_double_press(button_t *btn);
void btn_handled(button_t *btn, absolute_time_t now);