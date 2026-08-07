#include "packet_domain.h"
#include "parse_at.h"
#include "string_utils.h"

static void pdp_field_handler(int field_idx, const char *token, void *user_ctx) {
	packet_data_protocol_t *pdp = (packet_data_protocol_t *)user_ctx;
	if (token[0] == '\0') return;

	switch (field_idx) {
		case 0: pdp->cid = atoi(token); break;
		case 1: {
							strlcpy(pdp->pdp_type, token, sizeof(pdp->pdp_type));
							strip_string(pdp->pdp_type, '"');
						} break;
		case 2: {
							strlcpy(pdp->apn, token, sizeof(pdp->apn));
							strip_string(pdp->apn, '"');
						} break;
		case 3: {
							strlcpy(pdp->apn_addr, token, sizeof(pdp->apn_addr));
							strip_string(pdp->apn_addr, '"');
						} break;
		case 4: pdp->d_comp = atoi(token); break;
		case 5: pdp->h_comp = atoi(token); break;
		case 6: pdp->ipv4_ctrl = atoi(token); break;
		case 7: pdp->pcscf_discovery = atoi(token); break;
		case 8: pdp->im_cn_signalling = atoi(token); break;
		default: break;
	}
}

modem_err_t packet_domain_read_pdp_context(modem_ctx_t *modem, packet_data_protocol_t *pdp) {
	uint8_t data[128] = {0};
	modem_err_t ret	= modem_send_command_and_expect(modem, "AT+CGDCONT?", "+CGDCONT:", data, sizeof(data), 2000);
	if (ret == MODEM_OK) {
		parse_at_command_response((char*)data, "+CGDCONT:", ",", pdp_field_handler, pdp);
	}
	return ret;
}
