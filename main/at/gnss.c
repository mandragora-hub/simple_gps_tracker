#include <string.h>
#include "gnss.h"
#include "parse_at.h"
#include "freertos/task.h"

static const char *GNSS_TAG = "gnss_debug";

//TODO: maybe we need to store the last 10 valid position, or save all into a memory;
static gnss_info_t last_fixed_position = {0};

static void gnss_info_field_handler(int field_idx, const char *token, void *user_ctx) {
	gnss_info_t *info = (gnss_info_t *)user_ctx;
	if (token[0] == '\0') return;

	switch(field_idx) {
		case 0: info->fix_mode = atoi(token); break;
		case 1: info->num_satellites += atoi(token); break; // sat_gps 
		case 2: info->num_satellites += atoi(token); break; // sat_glonass
		case 3: info->num_satellites += atoi(token); break; // sat_beidou
		case 4: info->latitude = atof(token); break;
		case 5: info->latitude *= (token[0]  == 'S') ? -1.0f : 1.0f; break;
		case 6: info->longitude = atof(token); break;
		case 7: info->longitude *= (token[0] == 'W') ? -1.0f : 1.0f; break;
		case 8: strcpy(info->raw_date, token); break;
		case 9: strcpy(info->raw_time, token); break;
		case 10: info->altitude_m = atof(token); break;
		case 11: info->speed_knots = atof(token); break;
		case 12: info->course = atof(token); break;
		//case 13: pdop = atof(token); break;
		//case 14: hdop = atof(token); break;
		//case 15: vdop = atof(token); break;
		default: break;
	}
}

modem_err_t gnss_power_on(modem_ctx_t *modem) {
	uint8_t data[128] = {0}; // give enough spaces, due the big timeout
	modem_err_t ret	= modem_send_command_and_expect(modem, "AT+CGNSSPWR=1", "+CGNSSPWR: READY!", data, sizeof(data), 9000);
	if (ret != MODEM_OK) return ret;
	return ret;
}

void gnss_get_last_pos_info(gnss_info_t *gnss_info) {
	memcpy(gnss_info, &last_fixed_position, sizeof(last_fixed_position));
}

modem_err_t gnss_get_fixed_pos_info(modem_ctx_t *modem, gnss_info_t *info) {
	memset(info, 0, sizeof(gnss_info_t)); // safe guard

	uint8_t data[128];
	modem_err_t ret	= modem_send_command_and_expect(modem, "AT+CGNSSINFO", "+CGNSSINFO:", data, sizeof(data), 1200);
	if (ret == MODEM_OK) {
		bool parsed = parse_at_command_response((char*)data, "+CPSI:", ",", gnss_info_field_handler, info);
		if (parsed) memcpy(&last_fixed_position, &info, sizeof(info));
	}
	return ret;
}

modem_err_t gnss_sleep(modem_ctx_t *modem) {
	uint8_t data[16];
	modem_err_t ret	= modem_send_command(modem, "AT+CGNSSPWR=0", data, sizeof(data), 3000);
	return ret;
}

modem_err_t gnss_cold_start(modem_ctx_t *modem) {
	uint8_t data[16] = {0};
	modem_err_t ret	= modem_send_command(modem, "AT+CGPSCOLD", data, sizeof(data), 1200);
	return ret;
}

modem_err_t gnss_warm_start(modem_ctx_t *modem) {
	uint8_t data[16] = {0};
	modem_err_t ret	= modem_send_command(modem, "AT+CGPSWARM", data, sizeof(data), 1200);
	return ret;
}

modem_err_t gnss_hot_start(modem_ctx_t *modem) {
	uint8_t data[16] = {0};
	modem_err_t ret	= modem_send_command(modem, "AT+CGPSHOT", data, sizeof(data), 1200);
	return ret;
}

bool gnss_is_valid(gnss_info_t *gnss_info) {
	// TODO: made other validation;
	if (gnss_info->fix_mode < 1 || gnss_info->fix_mode > 3) return false;
	return true;
}

