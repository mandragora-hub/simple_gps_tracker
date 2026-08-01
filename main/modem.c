#include "modem.h"

static const char *MODEM_TAG = "modem_debug";

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
	if (modem == NULL || cmd == NULL || res == NULL || res_size == 0) return MODEM_BAD_REQUEST;

	//if (res_size == 0) return MODEM_OVERFLOW;

	// Zero out output buffer. Maybe this is optional
	memset(res, 0, res_size);

	uart_flush(modem->uart_port); // remove unread bytes
	uart_write_bytes(modem->uart_port, cmd, strlen(cmd));
	uart_write_bytes(modem->uart_port, "\r\n", 2);
	ESP_ERROR_CHECK(uart_wait_tx_done(modem->uart_port, pdMS_TO_TICKS(1000)));

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
//
//modem_err_t modem_send_command_and_expect(
//modem_ctx_t *modem,
//const char *cmd,
//const char *expect,
//uint8_t	*res,
//size_t res_size,
//uint32_t timeout_ms) {
//modem_err_t ret = modem_send_command(modem, cmd, res, res_size, timeout_ms);
//if (ret != MODEM_OK) return ret;
//
//if (strstr((char*)res, expect) == NULL) return MODEM_ERR_UNEXPECTED_RESPONSE;
//return MODEM_OK;
//}
//
