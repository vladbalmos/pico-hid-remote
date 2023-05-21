#include "pico/stdlib.h"

#define GREEN_LED_MIN_PIN 2
#define GREEN_LED_MED_PIN 3
#define GREEN_LED_MAX_PIN 4
#define BLUE_LED_PIN 5

#define HIDE_BAT_LEVEL_TIMEOUT_MS 3 * 1000
#define FLASH_LED_INTERVAL 150


void leds_init();
void leds_turnoff();
void leds_show_battery_level(uint8_t level);
void leds_show_bt_connecting();
void leds_hide_bt_connecting();