#include "modem.h"
#include "at/gnss.h"
#include "at/http.h"

#include "utils.h"
#include "geodesic.h"
#include "board.h"
#include "preferences.h"

static const char *TAG = "gnss_task";

static bool gnss_update_server(modem_ctx_t *modem, gnss_info_t *gnss_info) {
	http_request_t request = {0};
	http_response_t response = {0};

	char osmand_traccar_url[150] = {0};
	build_osmand_traccar_url(osmand_traccar_url, sizeof(osmand_traccar_url), gnss_info);

	strcpy(request.url, osmand_traccar_url);
	printf("request.url = %s\n", request.url);

	request.method = HTTP_METHOD_GET;

	if ((http_perform_action(modem, &request, &response) == true)) {
		printf("Http sucessfully operation\n");
		printf("response.statuscode = %d\n", response.statuscode);
		printf("response.datalen = %d\n", response.datalen);
		printf("response.content = %s\n", response.content);
		return true;
	}
	return false;
}

void gnss_task(void *pvParameters) {
	modem_ctx_t *modem = (modem_ctx_t *)pvParameters; 
	gnss_info_t last_sent_gnss_info = {0};
	
	uint32_t p_reporting_interval_ms = 60000;
	p_app_config_get(P_REPORTING_INTERVAL_MS_KEY, &p_reporting_interval_ms);
	
	/* when we don't send the location due distances of new location is short to last location with increment this var by one.
	 * Once the var reaches the value 10 we set the last_sent_gnss_info to {0}.
	 * We just want to keep update the others devices parameters */
	uint8_t retry = 0;

	while (gnss_power_on(modem) != MODEM_OK) {
		ESP_LOGE(TAG, "Failed to power GNSS module");
		gnss_sleep(modem);
		vTaskDelay(pdMS_TO_TICKS(2000));
	}

	// TODO: define criteria to use the different start mode
	gnss_hot_start(modem);

	for (int i = 0; i < 10; i++) {
		ESP_LOGI(TAG, "Enable agnss sever asisst. Attempt: %d", i);
		if (gnss_use_agnss_server_assisted(modem) == MODEM_OK) break;
		vTaskDelay(pdMS_TO_TICKS(2000));
	}

	ESP_LOGI(TAG, "GNSS powered on. Beginning acquisition loop...");
	for (;;) {
		gnss_info_t new_gnss_info = {0};
		if (gnss_get_fixed_pos_info(modem, &new_gnss_info, 30000) == MODEM_OK) {
			if (gnss_is_valid(&new_gnss_info)) {

				ESP_LOGI(TAG, "Lat %.6f, Lon %.6f", new_gnss_info.latitude, new_gnss_info.longitude);

				// TODO: validate the new location is significatly different of last location. 
				// Propose: update devices when location is different or certain time has passed

				double distances_m = 99;
				distances_m = calculate_geodesic_distances_gnss(&new_gnss_info, &last_sent_gnss_info);
				printf("distances_m =  %lf\n", distances_m);
				if (distances_m <= 5) {
					retry++;
					if (retry == 10) {
						memset(&last_sent_gnss_info, 0, sizeof(gnss_info_t));
						retry = 0;
					}	
					continue;
				}
				if ((gnss_update_server(modem, &new_gnss_info) == true)) {
					memcpy(&last_sent_gnss_info, &new_gnss_info, sizeof(new_gnss_info));
				}
			}
		} else {
			ESP_LOGI(TAG, "Waiting for satellite fix...");
			gnss_update_server(modem, NULL);
		}
		vTaskDelay(pdMS_TO_TICKS(p_reporting_interval_ms));
		remaining_task_stack();
	}

	gnss_sleep(modem);
	vTaskDelete(NULL);
} 

