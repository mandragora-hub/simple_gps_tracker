#include "serial_interface.h"

modem_err_t serial_interface_set_control_uart_sleep(modem_ctx_t *modem, serial_interface_uart_sleep_status_t siuss) {
	uint8_t data[128] = {0};
	char cmd[32] = {0};
	snprintf(cmd, sizeof(cmd), "AT+CSCLK=%d", siuss);
	modem_err_t ret	= modem_send_command(modem, cmd, data, sizeof(data), 2000);
	return ret;
}

