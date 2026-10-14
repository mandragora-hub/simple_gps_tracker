#ifndef SMS_UTILS_H
#define SMS_UTILS_H

#include "modem.h"
#include "at/sms.h"
#include "esp_event.h"

// declaration of sms events
ESP_EVENT_DECLARE_BASE(SMS_EVENTS);

typedef enum {
	SMS_EVENT_NEW_MESSAGE,
	SMS_EVENT_PROCESS_ALL_MESSAGES
} sms_event_id;
////


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

bool sms_utils_process_sms_command(modem_ctx_t *modem, const char *command, char *new_message, size_t new_message_size);

sms_command_t sms_utils_string_to_sms_command(const char *string);
const char *sms_utils_sms_command_to_string(sms_command_t sms_command);

modem_err_t sms_process_cmti(modem_ctx_t *modem, sms_cmti_t *cmti);
#endif // SMS_UTILS_H
