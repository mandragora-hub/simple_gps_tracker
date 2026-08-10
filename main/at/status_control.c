#include "status_control.h"
#include "parse_at.h"

static void status_control_signal_quality_field_handler(int field_idx, const char *token, void *user_ctx) {
	signal_quality_t *signal_quality = (signal_quality_t *)user_ctx;
	if (token[0] == '\0') return;

	switch (field_idx) {
		case 0: signal_quality->rssi = atoi(token); break;
		case 1: signal_quality->ber = atoi(token); break;
		default: break;
	}
}

static void imei_field_handler(int field_idx, const char *token, void *user_ctx) {
	imei_t *imei = (imei_t *)user_ctx;
	if (token[0] == '\0') return;

	switch (field_idx) {
		case 0: strcpy(imei->imei, token); break;
		default: break;
	}
}

modem_err_t status_control_query_signal_quality(modem_ctx_t *modem, signal_quality_t *signal_quality) {
	uint8_t data[128] = {0};
	modem_err_t ret	= modem_send_command_and_expect(modem, "AT+CSQ", "+CSQ:", data, sizeof(data), 2000);
	if (ret == MODEM_OK) {
		parse_at_command_response((char*)data, "+CSQ:", ",", status_control_signal_quality_field_handler, signal_quality);
	}
	return ret;
}

modem_err_t status_control_read_clock(modem_ctx_t *modem) {
	uint8_t data[128] = {0};
	modem_err_t ret	= modem_send_command_and_expect(modem, "AT+CCLK?", "+CCLK:", data, sizeof(data), 2000);
	return ret;
}

modem_err_t status_control_power_down_module(modem_ctx_t *modem) {
	uint8_t data[128] = {0};
	modem_err_t ret	= modem_send_command(modem, "AT+CPOF", data, sizeof(data), 2000);
	return ret;
}

modem_err_t status_control_reset_module(modem_ctx_t *modem) {
	uint8_t data[128] = {0};
	modem_err_t ret	= modem_send_command(modem, "AT+CRESET", data, sizeof(data), 2000);
	return ret;
}

modem_err_t status_control_query_imei(modem_ctx_t *modem, imei_t *imei) {
	uint8_t data[128] = {0};
	modem_err_t ret	= modem_send_command_and_expect(modem, "AT+SIMEI?", "+SIMEI:", data, sizeof(data), 2000);
	if (ret == MODEM_OK) {
		parse_at_command_response((char*)data, "+SIMEI:", ",", imei_field_handler, imei);
	}
	return ret;
}

