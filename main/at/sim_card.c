#include "sim_card.h"

bool sim_card_is_ready(modem_ctx_t *modem) {
	uint8_t data[128] = {0};
	modem_err_t ret	= modem_send_command_and_expect(modem, "AT+CPIN?", "+CPIN: READY", data, sizeof(data), 2000);
	return ret == MODEM_OK ? true : false;
}

