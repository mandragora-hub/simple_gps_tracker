#include "sms_utils.h"
#include "string_utils.h"
#include <stddef.h>
#include <string.h>

static const char *password = "1234";


bool sms_utils_process_sms_command(const char *command, char *new_message) {
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


