#ifndef V_25TER_H
#define V_25TER_H

#include "modem.h"

typedef struct {
	char manufacturer[32]; 
	char model[32]; 
	char revision[32]; 
	char imei[32]; 
	char gcap[32]; // additional capabilities
} v_25ter_product_id_info_t;

modem_err_t v_25ter_check_at(modem_ctx_t *modem);
modem_err_t v_25ter_get_product_id_info(modem_ctx_t *modem, v_25ter_product_id_info_t *v_25ter_product_id_info);

#endif // V_25TER_H
