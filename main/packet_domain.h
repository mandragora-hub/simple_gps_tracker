#ifndef PACKET_DOMAIN_H
#define PACKET_DOMAIN_H

#include "modem.h"

// A PDP Context is essentially a data connection profile that tells the cellular network:
// - Which APN to use
// - What type of IP address you want (IPv4, IPv6, or both)
// - Which network context to activate

typedef struct {
	uint8_t cid;
	char pdp_type[16];
	char apn[128];
	char apn_addr[64];
	uint8_t d_comp; // data compresion
	uint8_t h_comp; // header compresion
	uint8_t ipv4_ctrl;
	uint8_t request_type;
	uint8_t pcscf_discovery;
	uint8_t im_cn_signalling;
} packet_data_protocol_t;

modem_err_t packet_domain_read_pdp_context(modem_ctx_t *modem, packet_data_protocol_t *pdp);

#endif // PACKET_DOMAIN_H
