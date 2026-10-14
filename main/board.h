#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>
#include "esp_err.h"

#define BATTERY_ADC_PIN	35
#define BATTERY_ADC_CHANNEL ADC_CHANNEL_7 // GPIO 35 corresponds to ADC1 Channel 7
#define ADC_VOLTAGE_DIVIDER_RATIO 2.0f  

typedef enum {
	BATTERY_STATE_NO_BATTERY = 0,
	BATTERY_STATE_DISCHARGING,
	BATTERY_STATE_CHARGING,
	BATTERY_STATE_FULL
} battery_state_t;

esp_err_t board_battery_adc_init();
// When connected to the USB, the battery voltage data read is not the real battery voltage
esp_err_t board_read_battery_voltage_mv(uint32_t *voltage_mv_out);
uint8_t board_battery_voltage_to_percent(uint32_t voltage_mv);
battery_state_t board_evaluate_battery_status(uint32_t voltage_mv_out);
bool board_is_charging();
esp_err_t board_battery_adc_del();

#endif // BOARD_H
