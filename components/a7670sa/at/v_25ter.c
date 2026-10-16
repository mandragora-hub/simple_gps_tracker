#include "at/v_25ter.h"
#include <stdio.h>

static void process_line(char *text, v_25ter_product_id_info_t *v_25ter_product_id_info) {
	const char *delimiters = "\r\n"; 
	char *save_ptr;
	char *line = strtok_r(text, delimiters, &save_ptr);
	while (line != NULL) {
		if (strstr(line, ":") != NULL) {
			char key[32] = {0};
			char val[32] = {0};

			sscanf(line, "%[^:]: %[^\n]", key, val);
			if (strcmp(key, "Manufacturer") == 0) strcpy(v_25ter_product_id_info->manufacturer, val);
			if (strcmp(key, "Model") == 0) strcpy(v_25ter_product_id_info->model, val);
			if (strcmp(key, "Revision") == 0) strcpy(v_25ter_product_id_info->revision, val);
			if (strcmp(key, "IMEI") == 0) strcpy(v_25ter_product_id_info->imei, val);
			if (strcmp(key, "+GCAP") == 0) strcpy(v_25ter_product_id_info->gcap, val);
		}
		line = strtok_r(NULL, delimiters, &save_ptr);
	}
}

modem_err_t v_25ter_check_at(modem_ctx_t *modem) {
	uint8_t data[128] = {0};
	modem_err_t ret	= modem_send_command(modem, "AT", data, sizeof(data), 200);
	return ret;
}

modem_err_t v_25ter_get_product_id_info(modem_ctx_t *modem, v_25ter_product_id_info_t *v_25ter_product_id_info) {
	uint8_t data[256] = {0};
	modem_err_t ret	= modem_send_command(modem, "ATI", data, sizeof(data), 200);
	if (ret == MODEM_OK) {
		process_line((char*)data, v_25ter_product_id_info);
	}
	return ret;
}

