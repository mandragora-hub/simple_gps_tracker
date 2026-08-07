#ifndef HTTP_H
#define HTTP_H

#include "modem.h"

// For now we only need those field for the http_request. If we need to add more see the AT manual
typedef enum {
	GET = 0,
	POST = 1,
	HEAD = 2,
	DELETE = 3,
	PUT = 4
} HTTP_METHOD;

typedef struct {
	char url[100];
	HTTP_METHOD method;
	uint8_t conn_timeout; 		// Timeout for accessing server, Numeric type, range is 20-120s, default is 120s.
	uint8_t recv_timeout; 		// Timeout for receiving data from server, Numeric type range is 2s-120s, default is 20s.
														//char content_type[256];  	// This is for HTTP "Content-Type" tag, String type, max length is 256, and default is "text/plain".
														//char accept-type[256]; 		// This is for HTTP "Accept-type" tag, String type, max length is 256, and default is "*/*".
} http_request_t;

typedef struct {
	uint16_t statuscode;
	uint16_t datalen;
	char content[1024];
} http_response_t;

bool http_perform_action(modem_ctx_t *modem, http_request_t *http_request, http_response_t *http_response);

modem_err_t http_start_service(modem_ctx_t *modem);
modem_err_t http_stop_service(modem_ctx_t *modem);
modem_err_t http_set_parameter(modem_ctx_t *modem, http_request_t *http_request);
modem_err_t http_action(modem_ctx_t *modem, http_request_t *http_request, http_response_t *http_response);
modem_err_t http_read(modem_ctx_t *modem, http_response_t *http_response);

// TODO: maybe this should be in another fle. follow the content in at references manual
bool enable_network_services();
bool network_services_is_valid();
const char *get_operator();


#endif // HTTP_H
