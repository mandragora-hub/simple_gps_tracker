#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "modem_board.h"
#include "esp_adc/adc_oneshot.h"  

static const char *TAG = "modem_board";

static bool adc_battery_initialized = false;
static adc_oneshot_unit_handle_t battery_adc_handle;
static adc_cali_handle_t battery_cali_handle;

esp_err_t modem_board_init_and_poweron() {
	gpio_config_t io_conf = {
		.mode = GPIO_MODE_OUTPUT,
		.pin_bit_mask =
			(1ULL << BOARD_PWRKEY_PIN) |
			(1ULL << BOARD_POWERON_PIN) |
			(1ULL << MODEM_DTR_PIN) |
			(1ULL << MODEM_RESET_PIN),
	};

	esp_err_t err = gpio_config(&io_conf);
	if (err != ESP_OK) return err;

	ESP_LOGI(TAG, "Set power control pin %d HIGH\n", BOARD_POWERON_PIN);
	gpio_set_level(BOARD_POWERON_PIN, 1);

	// Reset
	ESP_LOGI(TAG, "Reset modem via pin %d\n", MODEM_RESET_PIN);
	gpio_set_level(MODEM_RESET_PIN, 0);
	vTaskDelay(pdMS_TO_TICKS(100));
	gpio_set_level(MODEM_RESET_PIN, 1);
	vTaskDelay(pdMS_TO_TICKS(2600));
	gpio_set_level(MODEM_RESET_PIN, 0);

	// Pull down DTR to ensure the modem is not in sleep state
	ESP_LOGI(TAG, "Set DTR pin %d LOW\n", MODEM_DTR_PIN);
	gpio_set_level(MODEM_DTR_PIN, 0);

	// Turn on the modem
	ESP_LOGI(TAG, "Power on modem via pin %d\n", BOARD_PWRKEY_PIN);
	gpio_set_level(BOARD_PWRKEY_PIN, 0);
	vTaskDelay(pdMS_TO_TICKS(100));
	gpio_set_level(BOARD_PWRKEY_PIN, 1);
	vTaskDelay(pdMS_TO_TICKS(MODEM_POWERON_PULSE_WIDTH_MS));
	gpio_set_level(BOARD_PWRKEY_PIN, 0);

	ESP_LOGI(TAG, "Product model name: %s", PRODUCT_MODEL_NAME);
	return ESP_OK;
}

esp_err_t battery_adc_init() { 
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

esp_err_t read_battery_voltage_mv(uint32_t *voltage_mv_out) {
	if (adc_battery_initialized == false || voltage_mv_out == NULL) return ESP_ERR_INVALID_STATE;

	esp_err_t ret = adc_oneshot_get_calibrated_result(battery_adc_handle, battery_cali_handle, BATTERY_ADC_CHANNEL,(int*)voltage_mv_out);
	if (ret != ESP_OK) return ret;

	*voltage_mv_out *= ADC_VOLTAGE_DIVIDER_RATIO;
	return ret;
}

battery_state_t evaluate_battery_status(uint32_t *voltage_mv_out) {
	// TODO: test this when i buy the battery
	if (voltage_mv_out < 2800) return BATTERY_STATE_NO_BATTERY;
	if (voltage_mv_out >= 4180) {
		return BATTERY_STATE_CHARGING;
	}
	return BATTERY_STATE_DISCHARGING
}

esp_err_t battery_adc_del() {
	esp_err_t ret =	adc_cali_delete_scheme_line_fitting(battery_cali_handle);
	if (ret != ESP_OK) return ret;
	ret =	adc_oneshot_del_unit(battery_adc_handle);
	return ret;
}

