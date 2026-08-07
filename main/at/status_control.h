#ifndef STATUS_CONTROL_H
#define STATUS_CONTROL_H

#include "modem.h"

typedef struct {
	uint8_t rssi; // signal strength
	uint8_t ber; // error rate
} signal_quality_t;

// This command is used to return received signal strength indication <rssi> and channel bit error rate <ber> from the ME. 
modem_err_t status_control_query_signal_quality(modem_ctx_t *modem, signal_quality_t *signal_quality);

// TODO: made a type for the time returned by modem
modem_err_t status_control_read_clock(modem_ctx_t *modem);

modem_err_t status_control_power_down_module(modem_ctx_t *modem);
modem_err_t status_control_reset_module(modem_ctx_t *modem);

#endif // STATUS_CONTROL_H
