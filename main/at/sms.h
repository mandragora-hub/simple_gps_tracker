#ifndef SMS_H
#define SMS_H

#include "modem.h"
#include "esp_event.h"

// declaration of sms events
ESP_EVENT_DECLARE_BASE(SMS_EVENTS);        

typedef enum {                                      
	SMS_EVENT_NEW_MESSAGE,
	SMS_EVENT_PROCESS_ALL_MESSAGES
} sms_event_id;
////

typedef struct {
	char sca[64];
	uint8_t tosca;
} sms_service_centre_address_t;

typedef struct {
	char mem[4];
	uint8_t used;
	uint8_t total;
} sms_preferred_message_storage_t;

typedef enum {
	SMS_CHARACTER_SET_IRA,
	SMS_CHARACTER_SET_GSM,
	SMS_CHARACTER_SET_UCS2,
	SMS_CHARACTER_SET_HEX 
} sms_character_set;

typedef enum {
	PDU_MODE = 0,
	TEXT_MODE = 1 
} sms_message_format;

typedef enum {
	SMS_STATUS_STORAGE_UNKNOWN = -1,
	SMS_STATUS_STORAGE_REC_UNREAD,  // received unread message (i.e. new message)
	SMS_STATUS_STORAGE_REC_READ, // received read message
	SMS_STATUS_STORAGE_STO_UNSENT, // stored unsent message
	SMS_STATUS_STORAGE_STO_SENT, // stored sent message
	SMS_STATUS_STORAGE_ALL  // all message
} sms_status_storage;

typedef struct {
	uint8_t index;
	sms_status_storage stat;
	char oa_da[32]; // Originating-Address or destination-Address
	char alpha[32];  // see references at manual
	char scts[32]; // TP-Service-Centre-Time-Stamp 
	char data[512];
} sms_message_t;


typedef struct {
	char mem[8];
	int index;
} sms_cmti_t; //Cellular Message Type Indicator.

modem_err_t sms_read_sca(modem_ctx_t *modem, sms_service_centre_address_t *sca);
modem_err_t sms_read_preferred_message_storage(modem_ctx_t *modem, sms_preferred_message_storage_t *pms);
modem_err_t sms_select_te_character_set(modem_ctx_t *modem, sms_character_set cs);
modem_err_t sms_select_message_format(modem_ctx_t *modem, sms_message_format smf);
modem_err_t sms_send_message(modem_ctx_t *modem, const char *dest_addr, const char *msg);

modem_err_t sms_list_messages(modem_ctx_t *modem, sms_message_t *messages, size_t m_size);

modem_err_t sms_read_message(modem_ctx_t *modem, uint8_t index, sms_message_t *message);
modem_err_t sms_delete_message(modem_ctx_t *modem, uint8_t index);
modem_err_t sms_read_and_delete_message(modem_ctx_t *modem, uint8_t index, sms_message_t *message);

bool sms_process_uart_pattern_event(char *line, sms_cmti_t *cmti);

modem_err_t sms_process_cmti(modem_ctx_t *modem, sms_cmti_t *cmti);
//modem_err_t sms_process_all_messages(modem_ctx_t *modem);

#endif //SMS_H
