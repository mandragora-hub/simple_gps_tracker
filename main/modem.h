#ifndef MODEM_H
#define MODEM_H

#include "driver/uart.h"

#define MODEM_OK 0
#define MODEM_ERR -1
#define MODEM_ERR_UNEXPECTED_RESPONSE -2
#define MODEM_BAD_REQUEST -3
#define MODEM_TIMEOUT -4
#define MODEM_OVERFLOW -5


//const char *MODEM_TAG = "modem_debug";

typedef int modem_err_t;

typedef struct {
	uart_port_t	uart_port;
} modem_ctx_t;

void modem_init(modem_ctx_t *modem, uart_port_t uart_port);
//void modem_free(modem_ctx_t *modem);

modem_err_t modem_send_command(
		modem_ctx_t *modem,
		const char *cmd,
		uint8_t	*res,
		size_t res_size,
		uint32_t timeout_ms);

modem_err_t modem_send_command_and_expect(
		modem_ctx_t *modem,
		const char *cmd,
		const char *expect,
		uint8_t	*res,
		size_t res_size,
		uint32_t timeout_ms);


#endif // MODEM_H
