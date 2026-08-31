#include <stdio.h>
#include <stddef.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "utils.h"
#include "modem_board.h"

const char *build_osmand_traccar_url(char *dest_url, size_t dest_url_size, gnss_info_t *gnss_info) {
	// example of osmand protocols =>  https://www.traccar.org/osmand/
	// TODO: send all fields left
	
	// batt info
	uint32_t bat_level_mv;
	modem_board_read_battery_voltage_mv(&bat_level_mv);
	uint8_t bat_level = modem_board_battery_voltage_to_percent(bat_level_mv);
	bool is_charging = modem_board_is_charging();

	snprintf(dest_url,
			dest_url_size,
			"%s/?id=%s&valid=true&lat=%lf&lon=%lf&batt=%hhu&charge=%s", 
			TRACCAR_URL,
			DEVICE_ID,
			gnss_info->latitude,
			gnss_info->longitude,
			bat_level,
			bool_to_string(is_charging));

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


