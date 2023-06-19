#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "battery.h"
#include "debug.h"

#define BATTERY_ADC_CONVERSION_FACTOR 3.3f / (1 << 12)
#define BATTERY_MAX_VOLTAGE 4.15
#define BATTERY_MIN_VOLTAGE 3.2

static const float battery_max_voltage_drop = BATTERY_MAX_VOLTAGE - BATTERY_MIN_VOLTAGE;


void battery_init(uint8_t adc_pin) {
    adc_init();
    adc_gpio_init(adc_pin);
    adc_select_input(0);
}

uint8_t battery_level() {
    uint16_t result = adc_read();
    float voltage = result * BATTERY_ADC_CONVERSION_FACTOR * 2 + .4; // .4 accounts for resistor divider inaccuracy

    DEBUG("Battery voltage: %f\n", voltage);
    if (voltage < BATTERY_MIN_VOLTAGE) {
        voltage = BATTERY_MIN_VOLTAGE;
    } else if (voltage > BATTERY_MAX_VOLTAGE) {
        voltage = BATTERY_MAX_VOLTAGE;
    }
    
    float voltage_drop = BATTERY_MAX_VOLTAGE - voltage;
    uint8_t level = (battery_max_voltage_drop - voltage_drop) * 100 / battery_max_voltage_drop;
    return level;
}
