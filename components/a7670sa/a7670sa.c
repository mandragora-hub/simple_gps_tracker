#include "driver/gpio.h"
#include "esp_sleep.h"
#include "esp_log.h"
#include "a7670sa.h"

static const char *TAG = "a7670sa";
static QueueHandle_t uart_queue;

// TODO: inquire about the advantages of define this utils with #define macro
static uart_config_t init_uart_default_config() {
	uart_config_t uart_config = {
		.baud_rate = MODEM_UART_BAUD_RATE,
		.data_bits = UART_DATA_8_BITS,
		.parity    = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
		.source_clk = UART_SCLK_DEFAULT,
	};
	return uart_config;
}

static TaskHandle_t s_sms_task_handle = NULL;

static void IRAM_ATTR gpio_isr_handler(void* arg) {
	BaseType_t woken = pdFALSE;
	vTaskNotifyGiveFromISR(s_sms_task_handle, &woken);
	if (woken) portYIELD_FROM_ISR();
}

static esp_err_t a7670sa_poweron() {
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

esp_err_t a7670sa_sleep() {
	return gpio_set_level(MODEM_DTR_PIN, 1);
}

esp_err_t a7670sa_wakeup() {
	return gpio_set_level(MODEM_DTR_PIN, 0);
}

esp_err_t a7670sa_setup_ri_wakeup() {
	gpio_config_t io_conf = {
		.pin_bit_mask = (1ULL << MODEM_RING_PIN),
		.mode = GPIO_MODE_INPUT,
		.pull_up_en = GPIO_PULLUP_ENABLE,
		.intr_type = GPIO_INTR_NEGEDGE
	};
	gpio_config(&io_conf);

	esp_err_t err = gpio_wakeup_enable(MODEM_RING_PIN, GPIO_INTR_LOW_LEVEL);
	if (err != ESP_OK) return err;
	// Enable GPIO level wakeup (Wake up when RI drops LOW)
	err = esp_sleep_enable_gpio_wakeup();
	if (err != ESP_OK) return err;	

	// Notify when Ring pin is high
	err = gpio_install_isr_service(0);
	if (err != ESP_OK) return err;
	err = gpio_isr_handler_add(MODEM_RING_PIN, gpio_isr_handler, NULL);
	return err;
}

void a7670sa_set_s_sms_task_handle(TaskHandle_t h) {
	s_sms_task_handle = h;
}

esp_err_t a7670sa_init(void) {
	ESP_ERROR_CHECK(a7670sa_poweron());

	uart_config_t uart_config = init_uart_default_config();

	int intr_alloc_flags = 0;
	ESP_ERROR_CHECK(uart_driver_install(UART_PORT_NUM, UART_RX_BUF_SIZE * 2, 0, 20, &uart_queue, intr_alloc_flags));
	ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_config));
	ESP_ERROR_CHECK(uart_set_pin(UART_PORT_NUM, MODEM_UART_TXD, MODEM_UART_RXD, MODEM_UART_RTS, MODEM_UART_CTS));

	// TODO: REMOVE THOSES LINE
	//Set uart pattern detect function.
	//uart_enable_pattern_det_baud_intr(UART_PORT_NUM, '+', 1, 9, 0, 0);
	//Reset the pattern queue length to record at most 20 pattern positions.
	//uart_pattern_queue_reset(UART_PORT_NUM, 20);

	// TODO: create the uart event here
	return ESP_OK;
}

esp_err_t a7670sa_modem_init(modem_ctx_t *modem) {
	if (modem == NULL) return ESP_ERR_INVALID_ARG;
	modem->uart_port = UART_PORT_NUM;

	ESP_LOGI(TAG, "Modem using uart port = %d", modem->uart_port);
	return ESP_OK;
}

QueueHandle_t a7670sa_get_uart_queue() {
	return uart_queue;
}

