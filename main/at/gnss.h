#ifndef GNSS_H
#define GNSS_H

#include <stdbool.h>
#include "modem.h"

typedef struct {
	int fix_mode;         // 2 = 2D fix, 3 = 3D fix
	int num_satellites;   // Total satellites used
	double latitude;      // Converted Decimal Degrees (e.g., 31.222177)
	double longitude;     // Converted Decimal Degrees (e.g., 121.354376)
	char raw_date[10];    // Date in format ddmmyy
	char raw_time[12];    // UTC Time in format hhmmss.ss
	float altitude_m;     // Altitude in meters
	float speed_knots;    // Speed in knots
} gnss_info_t;

modem_err_t gnss_power_on(modem_ctx_t *modem);
modem_err_t gnss_sleep(modem_ctx_t *modem);

modem_err_t gnss_cold_start(modem_ctx_t *modem);
modem_err_t gnss_warm_start(modem_ctx_t *modem);
modem_err_t gnss_hot_start(modem_ctx_t *modem);

modem_err_t gnss_get_fixed_pos_info(modem_ctx_t *modem, gnss_info_t *info);

bool gnss_is_valid(gnss_info_t *gnss_info);

#endif //GNSS_H
