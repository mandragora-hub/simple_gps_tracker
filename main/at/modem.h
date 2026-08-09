#ifndef MODEM_H
#define MODEM_H

#include "driver/uart.h"

// TODO Make function to convert err to string. Also how those number are save in memory? 
// I think as a int... in this case use hex, or look for the way to use one byte
#define MODEM_OK 0
#define MODEM_ERR -1
#define MODEM_ERR_UNEXPECTED_RESPONSE -2
#define MODEM_BAD_REQUEST -3
#define MODEM_TIMEOUT -4
#define MODEM_OVERFLOW -5
#define MODEM_ERR_NO_MEM -6
#define MODEM_UNPROCESSED_REQUEST -7

//const char *MODEM_TAG = "modem_debug";

typedef int modem_err_t;

typedef struct {
	uart_port_t	uart_port;
} modem_ctx_t;

modem_err_t modem_driver_init();
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


modem_err_t modem_write_uart(modem_ctx_t *modem, const char *cmd);
modem_err_t modem_read_uart(modem_ctx_t *modem, const char *expect, uint8_t	*res, size_t res_size, uint32_t timeout_ms);

#endif // MODEM_H
