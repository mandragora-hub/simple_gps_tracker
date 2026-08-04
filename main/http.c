#include "http.h"
#include "parse_at.h"
#include "string_utils.h"

static void httpaction_field_handler(int field_idx, const char *token, void *user_ctx) {
	http_response_t *http_response = (http_response_t *)user_ctx;
	if (token[0] == '\0') return;

	switch (field_idx) {
		case 1: http_response->statuscode = atoi(token); break;
		case 2: http_response->datalen = atoi(token); break;
		default: break;
	}
}

bool http_perform_action(modem_ctx_t *modem, http_request_t *http_request, http_response_t *http_response) {
	if ((http_start_service(modem)) != MODEM_OK) return false;

	if ((http_set_parameter(modem, http_request)) != MODEM_OK) {
		http_stop_service(modem);
		return false;
	}

	modem_err_t ret = http_action(modem, http_request, http_response);

	if (ret == MODEM_OK && http_response->datalen) {
		ret = http_read(modem, http_response);
	}

	http_stop_service(modem);
	return ret == MODEM_OK ? true: false;
}

modem_err_t http_start_service(modem_ctx_t *modem) {
	uint8_t data[1024] = {0};
	modem_err_t ret	= modem_send_command(modem, "AT+HTTPINIT", data, sizeof(data), 1200);
	return ret;
}

modem_err_t http_stop_service(modem_ctx_t *modem) {
	uint8_t data[1024] = {0};
	modem_err_t ret	= modem_send_command(modem, "AT+HTTPTERM", data, sizeof(data), 1200);
	return ret;
}

modem_err_t http_set_parameter(modem_ctx_t *modem, http_request_t *http_request) {
	// TODO: complete the others http params
	uint8_t data[1024] = {0};
	char command[512] = {0};
	snprintf(command, sizeof(command), "AT+HTTPPARA=\"URL\",\"%s\"", http_request->url);
	modem_err_t ret	= modem_send_command(modem, command, data, sizeof(data), 5000);
	return ret;
}

modem_err_t http_action(modem_ctx_t *modem, http_request_t *http_request, http_response_t *http_response) {
	uint8_t data[1024] = {0};
	char command[32] = {0};
	snprintf(command, sizeof(command), "AT+HTTPACTION=%d", http_request->method);
	modem_err_t ret	= modem_send_command_and_expect(modem, command, "+HTTPACTION:", data, sizeof(data), 5000);
	if (ret == MODEM_OK) {
		parse_at_command_response((char*)data, "+HTTPACTION:", ",", httpaction_field_handler, http_response);
	}
	return ret;
}

modem_err_t http_read(modem_ctx_t *modem, http_response_t *http_response) {
	uint8_t data[1024] = {0};
	char command[32] = {0};
	snprintf(command, sizeof(command), "AT+HTTPREAD=%d", http_response->datalen);
	modem_err_t ret	= modem_send_command(modem, command, data, sizeof(data), 5000);
	if (ret == MODEM_OK) {
		// TODO: if datalen is bigger that modem buffer maybe we need to read it by parts
		char start_str[128] = {0};
		snprintf(start_str, sizeof(start_str), "+HTTPREAD: %d", http_response->datalen);
		char end_str[128] = {0};
		snprintf(end_str, sizeof(end_str), "+HTTPREAD: %d", 0);

		trim_string(extract_between((char*)data, start_str, end_str,  http_response->content, sizeof(http_response->content)));
	}
	return ret;
}


