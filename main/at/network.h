#ifndef NETWORK_H
#define NETWORK_H

#include "modem.h"

typedef struct {
	uint8_t n;
	uint8_t stat;
} network_registration_t;

typedef struct {
	uint8_t mode;
	uint8_t format;
	char oper[16];
	uint8_t act; // Access technology selected
} operator_selection_t;

typedef struct {
	char system_mode[16];
	char operation_mode[16];
} ue_system_information_t;

modem_err_t network_read_network_registration(modem_ctx_t *modem, network_registration_t *network_registration);

modem_err_t network_read_operator_selection(modem_ctx_t *modem, operator_selection_t *operator_selection);

//TODO: Implement AT+CPSI Inquiring UE system information
modem_err_t network_query_ue_sys_information(modem_ctx_t *modem, ue_system_information_t *ue_system_information);

#endif // NETWORK_H
