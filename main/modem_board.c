#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "modem_board.h"

static const char *TAG = "modem_board";

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



