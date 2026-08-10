#include <stddef.h>
#include <string.h>
#include "esp_timer.h"

#include "sms_utils.h"
#include "string_utils.h"
#include "at/status_control.h"
#include "at/network.h"
#include "at/gnss.h"

static const char *password = "1234";

static char *rssi_to_signal(char *dest, size_t dest_size, int rssi) {
	if (rssi == 99) {
		strcpy(dest, "Not signal");
	} else {
		snprintf(dest, dest_size, "%d dBm", -113 + (2 * rssi));
	}
	return dest;
}

bool sms_utils_process_sms_command(modem_ctx_t *modem, const char *command, char *new_message, size_t new_message_size) {
	// validate the command has the correct password
	if (strstr(command, password) == NULL) return false; 

	// Clean text
	char command_without_password[16];
	size_t cwp_size = sizeof(command_without_password);
	remove_digits(command_without_password, cwp_size, command);
	to_lowercase(command_without_password, cwp_size, command_without_password);

	sms_command_t sms_command = sms_utils_string_to_sms_command(command_without_password);
	if (sms_command == SMS_COMMAND_UNKNOW_COMMAND) return false;

	if (sms_command == SMS_COMMAND_CHECK) {
		strcpy(new_message, "OK");
		return true;
	}

	if (sms_command == SMS_COMMAND_IMEI) {
		imei_t imei = {0};
		if (status_control_query_imei(modem, &imei) != MODEM_OK) return false;
		strcpy(new_message, imei.imei);
		return true;
	}

	if (sms_command == SMS_COMMAND_SMS_LINK) {
		gnss_info_t info = {0};
		if (gnss_get_fixed_pos_info(modem, &info) != MODEM_OK) return false;
		snprintf(new_message, new_message_size, "https://www.google.com/maps/search/?api=1&query=%lf,%lf", 
				info.latitude,
				info.longitude);
		return true;
	}

	if (sms_command == SMS_COMMAND_GPS) {
		gnss_info_t info = {0};
		if (gnss_get_fixed_pos_info(modem, &info) != MODEM_OK) return false;
		snprintf(new_message, new_message_size, "gnss: on\nfix: %d\nsatellites: %d\nlatitude: %lf\nlongitude: %lf\naltitude: %.1f m\nspeed: %.1f knots\ncourse: %.1f°", 
				info.fix_mode,
				info.num_satellites,
				info.latitude,
				info.longitude,
				info.altitude_m,
				info.speed_knots,
				info.course);
		return true;
	}

	if (sms_command == SMS_COMMAND_SIGNAL) {
		ue_system_information_t ue = {0};
		if (network_query_ue_sys_information(modem, &ue) != MODEM_OK) return false;

		signal_quality_t signal_quality = {0};
		if (status_control_query_signal_quality(modem, &signal_quality) != MODEM_OK) return false;
		char dbm[24] = {0}; 
		rssi_to_signal(dbm, sizeof(dbm), signal_quality.rssi);

		snprintf(new_message, new_message_size, "UE: %s - %s\nCSQ: %s\nRSSI: %d\n", 
				ue.system_mode, ue.operation_mode, dbm, signal_quality.rssi);

		return true;
	}

	if (sms_command == SMS_COMMAND_STATUS) {
		gnss_info_t info = {0};
		if (gnss_get_fixed_pos_info(modem, &info) != MODEM_OK) return false;

		ue_system_information_t ue = {0};
		if (network_query_ue_sys_information(modem, &ue) != MODEM_OK) return false;

		signal_quality_t signal_quality = {0};
		if (status_control_query_signal_quality(modem, &signal_quality) != MODEM_OK) return false;
		char dbm[24] = {0}; 
		rssi_to_signal(dbm, sizeof(dbm), signal_quality.rssi);

		uint64_t ms_sinces_boot = esp_timer_get_time();
		uint32_t total_minutes = ms_sinces_boot / 60000ULL;
		int8_t h = (int8_t)(total_minutes / 60ULL);
		int8_t m = (int8_t)(total_minutes % 60ULL);

		snprintf(new_message, new_message_size, "Tracker: ONLINE\nGNSS: %s\nSatellites: %d\nUE: %s -%s\nSignal: %s\nBattery: 78%%\nUptime: %dh %dm", 
				gnss_is_valid(&info) ? "FIX" : "OFFLINE",
				info.num_satellites, 
				ue.system_mode, 
				ue.operation_mode, 
				dbm, 
				h, 
				m);

		//
		//Tracker: ONLINE
		//GPS: FIX
		//Satellites: 9
		//LTE: REGISTERED
		//Signal: -81 dBm
		//Battery: 78%
		//Uptime: 3h 24m
		return true;
	}
	const char *not = "Every is ok, but command is not implemented...";
	strcpy(new_message, not);
	return true;
}

sms_command_t sms_utils_string_to_sms_command(const char *string) {
	if (strcmp(string, "check") == 0) return SMS_COMMAND_CHECK;
	if (strcmp(string, "smslink") == 0) return SMS_COMMAND_SMS_LINK;
	if (strcmp(string, "imei") == 0) return SMS_COMMAND_IMEI;
	if (strcmp(string, "status") == 0) return SMS_COMMAND_STATUS;
	if (strcmp(string, "gps") == 0) return SMS_COMMAND_GPS;
	if (strcmp(string, "signal") == 0) return SMS_COMMAND_SIGNAL;
	if (strcmp(string, "interval") == 0) return SMS_COMMAND_INTERVAL;
	if (strcmp(string, "reset") == 0) return SMS_COMMAND_RESET;
	if (strcmp(string, "sleep") == 0) return SMS_COMMAND_SLEEP;
	if (strcmp(string, "wake") == 0) return SMS_COMMAND_WAKE;
	if (strcmp(string, "help") == 0) return SMS_COMMAND_HELP;
	return SMS_COMMAND_UNKNOW_COMMAND;
}

const char *sms_utils_sms_command_to_string(sms_command_t sms_command) {
	switch(sms_command) {
		case SMS_COMMAND_CHECK: return "check";
		case SMS_COMMAND_SMS_LINK: return "smslink";
		case SMS_COMMAND_IMEI: return "imei";
		case SMS_COMMAND_STATUS: return "status";
		case SMS_COMMAND_GPS: return "gps";
		case SMS_COMMAND_SIGNAL: return "signal";
		case SMS_COMMAND_INTERVAL: return "interval";
		case SMS_COMMAND_RESET: return "reset";
		case SMS_COMMAND_SLEEP: return "sleep";
		case SMS_COMMAND_WAKE: return "wake";
		case SMS_COMMAND_HELP: return "help";
		default: return "";
	}
}


