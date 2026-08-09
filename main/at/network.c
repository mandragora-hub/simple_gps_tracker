#include "network.h"
#include "parse_at.h"
#include "string_utils.h"

static void network_registration_field_handler(int field_idx, const char *token, void *user_ctx) {
	network_registration_t *network_registration = (network_registration_t *)user_ctx;
	if (token[0] == '\0') return;

	switch (field_idx) {
		case 0: network_registration->n = atoi(token); break;
		case 1: network_registration->stat = atoi(token); break;
		default: break;
	}
}

static void operator_selection_field_handler(int field_idx, const char *token, void *user_ctx) {
	operator_selection_t *operator_selection = (operator_selection_t *)user_ctx;
	if (token[0] == '\0') return;

	switch (field_idx) {
		case 0: operator_selection->mode = atoi(token); break;
		case 1: operator_selection->format = atoi(token); break;
		case 2: {
							strlcpy(operator_selection->oper, token, sizeof(operator_selection->oper));
							strip_string(operator_selection->oper, '"');
						}	break;
		case 3: operator_selection->act = atoi(token); break;
		default: break;
	}
}

static void ue_system_information_field_handler(int field_idx, const char *token, void *user_ctx) {
	ue_system_information_t *ue = (ue_system_information_t *)user_ctx;
	if (token[0] == '\0') return;

	switch (field_idx) {
		case 0: {
							strcpy(ue->system_mode, token); 
							strip_string(ue->system_mode, '"');
						} break;
		case 1: {
							strcpy(ue->operation_mode, token);
							strip_string(ue->operation_mode, '"');
						}
					 	break;
		default: break;
	}
}

modem_err_t network_read_network_registration(modem_ctx_t *modem, network_registration_t *network_registration) {
	uint8_t data[128] = {0};
	modem_err_t ret	= modem_send_command_and_expect(modem, "AT+CREG?", "+CREG:", data, sizeof(data), 2000);
	if (ret == MODEM_OK) {
		parse_at_command_response((char*)data, "+CREG:", ",", network_registration_field_handler, network_registration);
	}
	return ret;
}

modem_err_t network_read_operator_selection(modem_ctx_t *modem, operator_selection_t *operator_selection) {
	uint8_t data[128] = {0};
	modem_err_t ret	= modem_send_command_and_expect(modem, "AT+COPS?", "+COPS:", data, sizeof(data), 2000);
	if (ret == MODEM_OK) {
		parse_at_command_response((char*)data, "+COPS:", ",", operator_selection_field_handler, operator_selection);
	}
	return ret;
}

modem_err_t network_query_ue_sys_information(modem_ctx_t *modem, ue_system_information_t *ue_system_information) {
	uint8_t data[128] = {0};
	modem_err_t ret	= modem_send_command_and_expect(modem, "AT+CPSI?", "+CPSI:", data, sizeof(data), 2000);
	if (ret == MODEM_OK) {
		parse_at_command_response((char*)data, "+CPSI:", ",", ue_system_information_field_handler, ue_system_information);
	}
	return ret;
}

