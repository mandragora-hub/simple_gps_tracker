#ifndef SMS_UTILS_H
#define SMS_UTILS_H

typedef enum {
	SMS_COMMAND_UNKNOW_COMMAND = -1,
	SMS_COMMAND_CHECK,
	SMS_COMMAND_SMS_LINK,
	SMS_COMMAND_IMEI,
	SMS_COMMAND_STATUS,
	SMS_COMMAND_GPS,
	SMS_COMMAND_SIGNAL,
	SMS_COMMAND_INTERVAL,
	SMS_COMMAND_RESET,
	SMS_COMMAND_SLEEP,
	SMS_COMMAND_WAKE,
	SMS_COMMAND_HELP
} sms_command_t;

bool sms_utils_process_sms_command(const char *command, char *new_message);

sms_command_t sms_utils_string_to_sms_command(const char *string);
const char *sms_utils_sms_command_to_string(sms_command_t sms_command);

#endif // SMS_UTILS_H
