#include <stdio.h>
#include <stddef.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "utils.h"
#include "board.h"
#include "preferences.h"

const char *build_osmand_traccar_url(char *dest_url, size_t dest_url_size, gnss_info_t *gnss_info) {
	char p_osmand_url[P_STR_BUF_SIZE] = {0};
	p_app_config_get(P_OSMAND_URL_KEY, p_osmand_url);
	char p_osmand_device_id[P_STR_BUF_SIZE] = {0};
	p_app_config_get(P_OSMAND_DEVICE_ID_KEY, p_osmand_device_id);


	// example of osmand protocols =>  https://www.traccar.org/osmand/
	// TODO: send all fields left

	// batt info
	uint32_t bat_level_mv;
	board_read_battery_voltage_mv(&bat_level_mv);
	uint8_t bat_level = board_battery_voltage_to_percent(bat_level_mv);
	bool is_charging = board_is_charging();

	int written = snprintf(dest_url,
			dest_url_size,
			"%s/?id=%s&valid=true&batt=%hhu&charge=%s", 
			p_osmand_url,
			p_osmand_device_id,
			bat_level,
			bool_to_string(is_charging));

	if (written < 0 || (size_t)written >= dest_url_size) {
		return dest_url;
	}

	if (gnss_info != NULL) {
		snprintf(dest_url + written,
				dest_url_size,
				"&lat=%lf&lon=%lf", 
				gnss_info->latitude,
				gnss_info->longitude);
	}
	return dest_url;
}

void remaining_task_stack() {
	UBaseType_t remaining_stack = uxTaskGetStackHighWaterMark(NULL);
	printf("Remaining stack: %u bytes\n", remaining_stack);
}

const char *bool_to_string(bool value) {
	return value ? "true" : "false";
}

//append("&timestamp=")
//append(location.time)
//append("&altitude=")
//append(location.altitude)
//append("&speed=")
//append(location.speed)
//append("&accuracy=")
//append(location.accuracy)
//append("&bearing=")
//append(location.bearing)
//append("&batt=${batt}")
//append("&charge=${isCharging}")


