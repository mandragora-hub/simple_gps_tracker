#include <stdio.h>
#include <stddef.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "utils.h"

const char *build_osmand_traccar_url(char *dest_url, size_t dest_url_size, gnss_info_t *gnss_info) {
	snprintf(dest_url,
			dest_url_size,
			"%s/?id=%s&valid=true&lat=%lf&lon=%lf", 
			TRACCAR_URL,
			DEVICE_ID,
			gnss_info->latitude,
			gnss_info->longitude);

	return dest_url;
}

void remaining_task_stack() {
	UBaseType_t remaining_stack = uxTaskGetStackHighWaterMark(NULL);
	printf("Remaining stack: %u bytes\n", remaining_stack);
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


