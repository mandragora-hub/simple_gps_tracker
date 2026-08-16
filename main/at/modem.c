#include "modem.h"
#include "esp_pm.h"

static const char *MODEM_TAG = "modem_debug";

static modem_err_t modem_write_uart_unlocked(modem_ctx_t *modem, const char *cmd);
static modem_err_t modem_read_uart_unlocked(modem_ctx_t *modem, const char *expect, uint8_t	*res, size_t res_size, uint32_t timeout_ms);

static SemaphoreHandle_t uart_mutex = NULL;

static esp_pm_lock_handle_t s_uart_pm_lock = NULL;

void modem_init_pm_locks() {
    // Lock type that prevents CPU frequency scaling
    esp_pm_lock_create(ESP_PM_CPU_FREQ_MAX, 0, "uart_lock", &s_uart_pm_lock);
}

modem_err_t modem_driver_init() {
	if (uart_mutex != NULL) return MODEM_OK; // avoid multiple definition if called multiple

	uart_mutex = xSemaphoreCreateMutex();
	if (uart_mutex == NULL) return MODEM_ERR_NO_MEM;
	return MODEM_OK;
}

void modem_init(modem_ctx_t *modem, uart_port_t uart_port) {
	if (modem == NULL) return;
	modem->uart_port = uart_port;

	ESP_LOGI(MODEM_TAG, "Modem using uart port = %d", modem->uart_port);
}

modem_err_t modem_send_command(
		modem_ctx_t *modem,
		const char *cmd,
		uint8_t	*res,
		size_t res_size,
		uint32_t timeout_ms) {
	return modem_send_command_and_expect(modem, cmd, "OK", res, res_size, timeout_ms);
}

modem_err_t modem_send_command_and_expect(
		modem_ctx_t *modem,
		const char *cmd,
		const char *expect,
		uint8_t	*res,
		size_t res_size,
		uint32_t timeout_ms) {
	modem_err_t timeout_err = MODEM_TIMEOUT;
	if (xSemaphoreTake(uart_mutex, portMAX_DELAY) == pdTRUE) {
		esp_pm_lock_acquire(s_uart_pm_lock); // Block CPU frequency scaling

		modem_err_t ret = modem_write_uart_unlocked(modem, cmd);
		if (ret == MODEM_OK) {
			ret =  modem_read_uart_unlocked(modem, expect, res, res_size, timeout_ms);
		}

		esp_pm_lock_release(s_uart_pm_lock); 
		xSemaphoreGive(uart_mutex);
		return ret;
	}
	return timeout_err;
}

modem_err_t modem_write_uart(modem_ctx_t *modem, const char *cmd) {
	modem_err_t timeout_err = MODEM_TIMEOUT;
	if (xSemaphoreTake(uart_mutex, portMAX_DELAY) == pdTRUE) {
		esp_pm_lock_acquire(s_uart_pm_lock); // Block CPU frequency scaling

		modem_err_t ret = modem_write_uart_unlocked(modem, cmd);

		esp_pm_lock_release(s_uart_pm_lock); 
		xSemaphoreGive(uart_mutex);
		return ret;
	}
	return timeout_err;
}

modem_err_t modem_read_uart(modem_ctx_t *modem, const char *expect, uint8_t	*res, size_t res_size, uint32_t timeout_ms) {
	modem_err_t timeout_err = MODEM_TIMEOUT;
	if (xSemaphoreTake(uart_mutex, portMAX_DELAY) == pdTRUE) {
		esp_pm_lock_acquire(s_uart_pm_lock); // Block CPU frequency scaling

		modem_err_t ret = modem_read_uart_unlocked(modem, expect, res, res_size, timeout_ms);

		esp_pm_lock_release(s_uart_pm_lock); 
		xSemaphoreGive(uart_mutex);
		return ret;
	}
	return timeout_err;
}

static modem_err_t modem_write_uart_unlocked(modem_ctx_t *modem, const char *cmd) {
	if (modem == NULL) return MODEM_BAD_REQUEST;

	uart_flush(modem->uart_port); // remove unread bytes
	uart_write_bytes(modem->uart_port, cmd, strlen(cmd));
	uart_write_bytes(modem->uart_port, "\r\n", 2);
	ESP_ERROR_CHECK(uart_wait_tx_done(modem->uart_port, portMAX_DELAY));
	return MODEM_OK;
}

static modem_err_t modem_read_uart_unlocked(modem_ctx_t *modem, const char *expect, uint8_t	*res, size_t res_size, uint32_t timeout_ms) {
	if (modem == NULL || res == NULL || res_size == 0) return MODEM_BAD_REQUEST;

	// Zero out output buffer. Maybe this is optional
	memset(res, 0, res_size);


	TickType_t start_ticks = xTaskGetTickCount();
	TickType_t timeout_ticks = pdMS_TO_TICKS(timeout_ms);

	size_t total = 0;
	while((xTaskGetTickCount() - start_ticks) < timeout_ticks) {
		size_t to_read = res_size - total - 1;
		size_t len = uart_read_bytes(modem->uart_port, res + total, to_read, pdMS_TO_TICKS(1000));
		if (len > 0) {
			total += len;
			if (total >= res_size - 1) return MODEM_OVERFLOW;

			res[total] = '\0';

			if (strstr((char*)res, "ERROR") || strstr((char*)res, "+CME ERROR") || strstr((char*)res, "+CMS ERROR")) {
				ESP_LOGE(MODEM_TAG, "Modem returned ERROR\n%s", res);
				return MODEM_ERR;
			}

			bool has_expect = (expect == NULL) || (strstr((char*)res, expect) != NULL);
			bool has_ok = (strstr((char*)res, "OK") != NULL);

			if (has_expect && has_ok) {
				ESP_LOGI(MODEM_TAG, "RX Success:\n%s", res);
				return MODEM_OK;
			}
		}
	}

	ESP_LOGW(MODEM_TAG, "Command timed out. Received so far:\n%s", res);

	// If time ran out, check if we at least got the expected string
	if (expect != NULL && strstr((char*)res, expect) == NULL) {
		return MODEM_ERR_UNEXPECTED_RESPONSE;
	}

	return MODEM_TIMEOUT;
}

