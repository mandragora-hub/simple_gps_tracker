#ifndef SERIAL_INTERFACE_H
#define SERIAL_INTERFACE_H

#include "modem.h"

typedef enum {
	SERIAL_INTERFACE_UART_SLEEP_STATUS_OFF = 0,
	SERIAL_INTERFACE_UART_SLEEP_STATUS_DTR_SLEEP = 1,
	SERIAL_INTERFACE_UART_SLEEP_STATUS_RX_SLEEP = 2
} serial_interface_uart_sleep_status_t;

typedef enum {
	SERIAL_INTERFACE_MODEM_RI_OFF = 0,
	SERIAL_INTERFACE_MODEM_RI_ON = 1,
} serial_interface_modem_ring_status_t;


modem_err_t serial_interface_set_control_uart_sleep(modem_ctx_t *modem, serial_interface_uart_sleep_status_t siuss);
modem_err_t serial_interface_configure_ri_pin(modem_ctx_t *modem, serial_interface_modem_ring_status_t status);



#endif // SERIAL_INTERFACE_H
