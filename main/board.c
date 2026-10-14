#include "board.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"  
#include "esp_sleep.h"
#include "at/sms.h"

static const char *TAG = "board";

static bool adc_battery_initialized = false;
static adc_oneshot_unit_handle_t battery_adc_handle;
static adc_cali_handle_t battery_cali_handle;

uint8_t board_battery_voltage_to_percent(uint32_t voltage_mv) {
	uint16_t min_bat_level = 3000;
	uint16_t max_bat_level = 4200;
	if (voltage_mv <= min_bat_level) return 0;
	if (voltage_mv >= max_bat_level) return 100;
	uint8_t percent = ((voltage_mv - min_bat_level) * 100 / (max_bat_level - min_bat_level));
	return percent;
}

esp_err_t board_read_battery_voltage_mv(uint32_t *voltage_mv_out) {
	if (adc_battery_initialized == false || voltage_mv_out == NULL) return ESP_ERR_INVALID_STATE;

	esp_err_t ret = adc_oneshot_get_calibrated_result(battery_adc_handle, battery_cali_handle, BATTERY_ADC_CHANNEL,(int*)voltage_mv_out);
	if (ret != ESP_OK) return ret;

	*voltage_mv_out *= ADC_VOLTAGE_DIVIDER_RATIO;
	return ret;
}

battery_state_t board_evaluate_battery_status(uint32_t voltage_mv_out) {
	// TODO: test this when i buy the battery
	if (voltage_mv_out < 2800) return BATTERY_STATE_NO_BATTERY;
	if (voltage_mv_out >= 4180) {
		return BATTERY_STATE_CHARGING;
	}
	return BATTERY_STATE_DISCHARGING;
}

bool board_is_charging() { // TODO: unfortunately this is not working
	uint32_t bat_level_mv;
	board_read_battery_voltage_mv(&bat_level_mv);
	battery_state_t state = board_evaluate_battery_status(bat_level_mv);
	return state == BATTERY_STATE_NO_BATTERY ? true : false;
}

esp_err_t board_battery_adc_del() {
	esp_err_t ret =	adc_cali_delete_scheme_line_fitting(battery_cali_handle);
	if (ret != ESP_OK) return ret;
	ret =	adc_oneshot_del_unit(battery_adc_handle);
	return ret;
}

esp_err_t board_battery_adc_init() { 
	if (adc_battery_initialized == true) return ESP_OK;

	adc_oneshot_unit_init_cfg_t init_config1 = {
		.unit_id = ADC_UNIT_1,
		.ulp_mode = ADC_ULP_MODE_DISABLE,
	};

	esp_err_t ret = adc_oneshot_new_unit(&init_config1, &battery_adc_handle);
	if (ret != ESP_OK) return ret;

	adc_oneshot_chan_cfg_t config = {
		.bitwidth = ADC_BITWIDTH_DEFAULT,
		.atten = ADC_ATTEN_DB_12,
	};

	ret = adc_oneshot_config_channel(battery_adc_handle, BATTERY_ADC_CHANNEL, &config);
	if (ret != ESP_OK) return ret;

	adc_cali_line_fitting_config_t cali_config = {
		.unit_id = ADC_UNIT_1,
		.atten = ADC_ATTEN_DB_12,
		.bitwidth = ADC_BITWIDTH_DEFAULT,
	};
	adc_cali_create_scheme_line_fitting(&cali_config, &battery_cali_handle);
	if (ret != ESP_OK) return ret;

	adc_battery_initialized = true;

	return ret;
}
