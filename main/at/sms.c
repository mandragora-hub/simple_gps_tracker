#include "sms.h"
#include "parse_at.h"
#include "string_utils.h"
#include "sms_utils.h"

static const char *SMS_TAG = "sms_tag";

static const char *character_set_to_string(sms_character_set cs) {
	switch (cs) {
		case SMS_CHARACTER_SET_IRA:  return "IRA";
		case SMS_CHARACTER_SET_GSM:  return "GSM";
		case SMS_CHARACTER_SET_UCS2: return "UCS2";
		case SMS_CHARACTER_SET_HEX:  return "HEX";
		default: return NULL;
	}
}

static const char *sms_status_storage_to_string(sms_status_storage sss) {
	switch (sss) {
		case SMS_STATUS_STORAGE_REC_UNREAD: return "REC UNREAD";
		case SMS_STATUS_STORAGE_REC_READ: return "REC READ";
		case SMS_STATUS_STORAGE_STO_UNSENT: return "STO UNSENT";
		case SMS_STATUS_STORAGE_STO_SENT: return "STO SENT";
		case SMS_STATUS_STORAGE_ALL: return "ALL";
		default: return NULL;
	}
}

static sms_status_storage string_to_sms_status_storage(const char *stat) {
	if (stat == NULL) return SMS_STATUS_STORAGE_UNKNOWN;

	if (strcmp(stat, "REC UNREAD") == 0) return SMS_STATUS_STORAGE_REC_UNREAD;
	if (strcmp(stat, "REC READ")   == 0) return SMS_STATUS_STORAGE_REC_READ;
	if (strcmp(stat, "STO UNSENT") == 0) return SMS_STATUS_STORAGE_STO_UNSENT;
	if (strcmp(stat, "STO SENT")   == 0) return SMS_STATUS_STORAGE_STO_SENT;
	if (strcmp(stat, "ALL")        == 0) return SMS_STATUS_STORAGE_ALL;

	return SMS_STATUS_STORAGE_UNKNOWN;
}


static void sms_message_field_handler(int field_idx, const char *token, void *user_ctx) {
	sms_message_t *sm = (sms_message_t *)user_ctx;
	if (token[0] == '\0') return;

	switch (field_idx) {
		case 0: {
							char token_stack[32] = {0};
							strcpy(token_stack, token);

							sm->stat = string_to_sms_status_storage(strip_string(token_stack, '"'));
						} break;
		case 1: {
							strcpy(sm->oa_da, token);
							strip_string(sm->oa_da, '"');
						} break;
		case 2: {
							strcpy(sm->alpha, token);
							strip_string(sm->alpha, '"');
						} break;
		case 3: strcpy(sm->scts, token); break;
		case 4: {
							strcat(sm->scts, ",");
							strcat(sm->scts, token);
							strip_string(sm->scts, '"');
						} break;
		default: break;
	}
}

static void sms_cmti_field_handler(int field_idx, const char *token, void *user_ctx) {
	sms_cmti_t *cmti = (sms_cmti_t *)user_ctx;
	if (token[0] == '\0') return;

	switch (field_idx) {
		case 0: {
							strcpy(cmti->mem, token);
							strip_string(cmti->mem, '"');
						} break;
		case 1: cmti->index = atoi(token); break;
		default: break;
	}
}

char *extract_body_of_sms_response(char *dest, const char *src, size_t dest_size) {
	char *body_start = strstr(src, "+CMGR:");
	if (body_start == NULL) body_start = strstr(src, "+CMGRD:");
	if (body_start == NULL) return NULL;

	body_start = strpbrk(body_start, "\r\n");
	if (body_start == NULL) return NULL;

	body_start += strspn(body_start, "\r\n");

	const char *body_end = strstr(body_start, "\r\nOK");

	size_t body_len = 0;

	if (body_end != NULL) {
		body_len = body_end - body_start;
	} else {
		body_len = strlen(body_start);
	}

	if (body_len >= dest_size) {
		body_len = dest_size - 1; // truncate safely
	}

	memcpy(dest, body_start, body_len);
	dest[body_len] = '\0';

	return dest;
}

static void sca_field_handler(int field_idx, const char *token, void *user_ctx) {
	sms_service_centre_address_t *sca = (sms_service_centre_address_t *)user_ctx;
	if (token[0] == '\0') return;

	switch (field_idx) {
		case 0: {
							strcpy(sca->sca, token);
							strip_string(sca->sca, '"');
						} break;
		case 1: sca->tosca = atoi(token); break;
		default: break;
	}
}

static void pms_field_handler(int field_idx, const char *token, void *user_ctx) {
	sms_preferred_message_storage_t *pms = (sms_preferred_message_storage_t *)user_ctx;
	if (token[0] == '\0') return;

	int field_idx_mod = field_idx % 3;
	int item_index = field_idx / 3;

	if (item_index >= 3) return;

	switch (field_idx_mod) {
		case 0: {
							strcpy(pms[item_index].mem, token);
							strip_string(pms[item_index].mem, '"');
						} break;
		case 1: pms[item_index].used = atoi(token); break;
		case 2: pms[item_index].total = atoi(token); break;
		default: break;
	}
}

modem_err_t sms_read_sca(modem_ctx_t *modem, sms_service_centre_address_t *sca) {
	uint8_t data[128] = {0};
	modem_err_t ret	= modem_send_command_and_expect(modem, "AT+CSCA?", "+CSCA:", data, sizeof(data), 2000);
	if (ret == MODEM_OK) {
		parse_at_command_response((char*)data, "+CSCA:", ",", sca_field_handler, sca);
	}
	return ret;
}

modem_err_t sms_read_preferred_message_storage(modem_ctx_t *modem, sms_preferred_message_storage_t *pms) {
	uint8_t data[128] = {0};
	modem_err_t ret	= modem_send_command_and_expect(modem, "AT+CPMS?", "+CPMS:", data, sizeof(data), 2000);
	if (ret == MODEM_OK) {
		parse_at_command_response((char*)data, "+CPMS:", ",", pms_field_handler, pms);
	}
	return ret;
}

modem_err_t sms_select_te_character_set(modem_ctx_t *modem, sms_character_set cs) {
	uint8_t data[128] = {0};
	char cmd[32] = {0};
	snprintf(cmd, sizeof(cmd), "AT+CSCS=\"%s\"", character_set_to_string(cs));
	modem_err_t ret	= modem_send_command(modem, cmd, data, sizeof(data), 2000);
	return ret;
}

modem_err_t sms_select_message_format(modem_ctx_t *modem, sms_message_format smf) {
	uint8_t data[128] = {0};
	char cmd[32] = {0};
	snprintf(cmd, sizeof(cmd), "AT+CMGF=%d", smf);
	modem_err_t ret	= modem_send_command(modem, cmd, data, sizeof(data), 2000);
	return ret;
}

modem_err_t sms_send_message(modem_ctx_t *modem, const char *dest_addr, const char *msg) {
	uint8_t data[128] = {0};
	char cmd[32] = {0};
	char sms_msg[256] = {0};
	const char ctrl_z = '\x1A'; 

	snprintf(cmd, sizeof(cmd), "AT+CMGS=\"%s\"", dest_addr);
	snprintf(sms_msg, sizeof(sms_msg), "%s%c", msg, ctrl_z);

	modem_send_command(modem, cmd, data, sizeof(data), 600);
	modem_err_t ret = modem_send_command(modem, sms_msg, data, sizeof(data), 10000);

	return ret;
}

modem_err_t sms_read_message(modem_ctx_t *modem, uint8_t index, sms_message_t *message) {
	uint8_t data[128] = {0};
	char cmd[32] = {0};
	snprintf(cmd, sizeof(cmd), "AT+CMGR=%d", index);
	message->index = index;
	modem_err_t ret	= modem_send_command_and_expect(modem, cmd, "+CMGR:", data, sizeof(data), 9000);
	if (ret == MODEM_OK) {
		trim_string(extract_body_of_sms_response(message->data, (const char*)data, sizeof(message->data)));
		parse_at_command_response((char*)data, "+CMGR:", ",", sms_message_field_handler, message);
	}

	return ret;
}

modem_err_t sms_delete_message(modem_ctx_t *modem, uint8_t index) {
	uint8_t data[128] = {0};
	char cmd[32] = {0};
	snprintf(cmd, sizeof(cmd), "AT+CMGD=%d", index);
	modem_err_t ret	= modem_send_command(modem, cmd, data, sizeof(data), 2000);
	return ret;
}

modem_err_t sms_read_and_delete_message(modem_ctx_t *modem, uint8_t index, sms_message_t *message) {
	uint8_t data[128] = {0};
	char cmd[32] = {0};
	snprintf(cmd, sizeof(cmd), "AT+CMGRD=%d", index);
	message->index = index;
	modem_err_t ret	= modem_send_command_and_expect(modem, cmd, "+CMGRD:", data, sizeof(data), 9000);
	if (ret == MODEM_OK) {
		trim_string(extract_body_of_sms_response(message->data, (const char*)data, sizeof(message->data)));
		parse_at_command_response((char*)data, "+CMGRD:", ",", sms_message_field_handler, message);
	}

	return ret;
}

bool sms_process_uart_pattern_event(char *line, sms_cmti_t *cmti) {
	if ((strstr(line, "+CMTI:")) == NULL) return false;	
	parse_at_command_response(line, "+CMTI:", ",", sms_cmti_field_handler, cmti);

	if (cmti->index <= 0) return false;
	return true;
}

modem_err_t sms_process_cmti(modem_ctx_t *modem, sms_cmti_t *cmti) {
	sms_message_t message = {0};
	if ((sms_read_and_delete_message(modem, cmti->index, &message)) != MODEM_OK) return -1;

	ESP_LOGI(SMS_TAG, "message.index = %d", message.index);
	ESP_LOGI(SMS_TAG, "message.stat = %d", message.stat);
	ESP_LOGI(SMS_TAG, "message.oa_da = %s", message.oa_da);
	ESP_LOGI(SMS_TAG, "message.alpha = %s", message.alpha);
	ESP_LOGI(SMS_TAG, "message.scts = %s", message.scts);
	ESP_LOGI(SMS_TAG, "message.data = |%s|", message.data);
	char new_message[128] = {0};
	if (!sms_utils_process_sms_command(modem, message.data, new_message, sizeof(new_message))) return MODEM_UNPROCESSED_REQUEST;
	modem_err_t ret = sms_send_message(modem, message.oa_da, new_message);
	return ret;
}

