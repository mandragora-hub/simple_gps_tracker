#include "gnss.h"
#include "freertos/task.h"
#include <string.h>

static const char *GNSS_TAG = "gnss_debug";

//TODO: maybe we need to store the last 10 valid position, or save all into a memory;
static gnss_info_t last_valid_position = {0};

// Helper to convert NMEA (ddmm.mmmm or dddmm.mmmm) to decimal degrees
static double nmea_to_decimal(double nmea_val, char hemisphere) {
	int degrees = (int)(nmea_val / 100);
	double minutes = nmea_val - (degrees * 100);
	double decimal = degrees + (minutes / 60.0);

	if (hemisphere == 'S' || hemisphere == 'W') {
		decimal = -decimal;
	}
	return decimal;
}

modem_err_t gnss_power_on(modem_ctx_t *modem) {
	uint8_t data[1024] = {0};
	modem_err_t ret	= modem_send_command_and_expect(modem, "AT+CGNSSPWR=1", "+CGNSSPWR: READY!", data, sizeof(data), 9000);
	if (ret != MODEM_OK) return ret;

	vTaskDelay(pdMS_TO_TICKS(200));
	return ret;
}

void gnss_get_position(gnss_info_t *gnss_info) {
	memcpy(gnss_info, &last_valid_position, sizeof(last_valid_position));
}

bool gnss_has_fixed_position(modem_ctx_t *modem) {
	uint8_t data[1024];
	modem_err_t ret	= modem_send_command_and_expect(modem, "AT+CGNSSINFO", "+CGNSSINFO:", data, sizeof(data), 1200);
	if (ret != MODEM_OK) return false;

	const	char *info_str = strstr((char*)data, "+CGNSSINFO:");
	if (info_str == NULL) return false;

	gnss_info_t gnss_info = {0};
	bool parsed = parse_gnss_info(info_str, &gnss_info);

	if (parsed) {
		//print_gnss_info(&gnss_info);
		memcpy(&last_valid_position, &gnss_info, sizeof(gnss_info));
	}

	return parsed;
}

modem_err_t gnss_sleep(modem_ctx_t *modem) {
	uint8_t data[1024];
	modem_err_t ret	= modem_send_command(modem, "AT+CGNSSPWR=0", data, sizeof(data), 3000);
	return ret;
}

modem_err_t gnss_cold_start(modem_ctx_t *modem) {
	uint8_t data[1024] = {0};
	modem_err_t ret	= modem_send_command(modem, "AT+CGPSCOLD", data, sizeof(data), 1200);
	return ret;
}

modem_err_t gnss_warm_start(modem_ctx_t *modem) {
	uint8_t data[1024] = {0};
	modem_err_t ret	= modem_send_command(modem, "AT+CGPSWARM", data, sizeof(data), 1200);
	return ret;
}

modem_err_t gnss_hot_start(modem_ctx_t *modem) {
	uint8_t data[1024] = {0};
	modem_err_t ret	= modem_send_command(modem, "AT+CGPSHOT", data, sizeof(data), 1200);
	return ret;
}

bool parse_gnss_info(const char *response, gnss_info_t *out_info) {
	if (response == NULL || out_info == NULL) return false;

	// Find the start of the payload
	char *str = strstr(response, "+CGNSSINFO:");
	if (str == NULL) return false;

	char *payload = str + strlen("+CGNSSINFO:");
	char *newline = strpbrk(payload, "\r\n");
	if (newline != NULL) *newline = '\0';

	char *saveptr = NULL;
	char *token = NULL;
	int field_idx = 0;
	const char *separator_str = ",";

	// Buffers for raw values
	int mode = 0, sat_gps = 0, sat_glonass = 0, sat_beidou = 0;
	double raw_lat = 0.0, raw_lon = 0.0;
	char ns = 'N', ew = 'E';
	char date_str[10] = {0}, time_str[12] = {0};
	float alt = 0.0f, speed = 0.0f, course = 0.0f;
	float pdop = 0.0f, hdop = 0.0f, vdop = 0.0f;

	token = strtok_r(payload, separator_str, &saveptr);
	while (token != NULL) {
		// Trim leading spaces if any
		while (*token == ' ') token++;
		switch(field_idx) {
			case 0: mode = atoi(token); break;
			case 1: sat_gps = atoi(token); break;
			case 2: sat_glonass = atoi(token); break;
			case 3: sat_beidou = atoi(token); break;
			case 4: raw_lat = atof(token); break;
			case 5: ns = token[0]; break;
			case 6: raw_lon = atof(token); break;
			case 7: ew = token[0]; break;
			case 8: strlcpy(date_str, token, sizeof(date_str) - 1); break;
			case 9: strlcpy(time_str, token, sizeof(time_str) - 1); break;
			case 10: alt = atof(token); break;
			case 11: speed = atof(token); break;
			case 12: course = atof(token); break;
			case 13: pdop = atof(token); break;
			case 14: hdop = atof(token); break;
			case 15: vdop = atof(token); break;
			default: break;
		}

		token = strtok_r(NULL, separator_str, &saveptr);
		field_idx++;
	}

	if (mode >= 2) { // 2D or 3D fix
		out_info->fix_mode = mode;
		out_info->num_satellites = sat_gps + sat_glonass + sat_beidou;
		out_info->latitude = (ns == 'S') ? -raw_lat : raw_lat;
		out_info->longitude = (ew == 'W') ? -raw_lon : raw_lon;
		strncpy(out_info->raw_date, date_str, sizeof(out_info->raw_date) - 1);
		out_info->raw_date[sizeof(out_info->raw_date) - 1] = '\0';
		strncpy(out_info->raw_time, time_str, sizeof(out_info->raw_time) - 1);
		out_info->raw_time[sizeof(out_info->raw_time) - 1] = '\0';
		out_info->altitude_m = alt;
		out_info->speed_knots = speed;
		out_info->is_valid = true;
		return out_info->is_valid;
	}

	return false;
}

void print_gnss_info(gnss_info_t *gnss_info) {
	if (gnss_info == NULL) return;
	ESP_LOGI(GNSS_TAG, "fix_mode = %d, num_satellites = %d, latitude = %lf, longitude = %lf, raw_date = %s, raw_time = %s, altitude_m = %f, speed_knots = %f, is_valid = %d\n",
		 	gnss_info->fix_mode, 
			gnss_info->num_satellites, 
			gnss_info->latitude, 
			gnss_info->longitude, 
			gnss_info->raw_date, 
			gnss_info->raw_time, 
			gnss_info->altitude_m, 
			gnss_info->speed_knots, 
			gnss_info->is_valid);
}
