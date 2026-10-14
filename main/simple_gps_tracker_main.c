#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_pm.h"
#include "esp_littlefs.h"
#include "captive_portal.h"
#include "a7670sa.h"
#include "utils.h"
#include "modem.h"
#include "board.h"
#include "sms_utils.h"

#define WEB_PAGE_MOUNT_POINT_IN_FS "/base_path"
#define MODEM_START_WAIT_MS (3000)

static const char *TAG = "simple_gps_tracker_main";

static modem_ctx_t modem = {0};
static TaskHandle_t s_sms_task_handle = NULL;

extern void uart_event_task(void *pvParameters); 
extern void sms_worker_task(void *pvParameters);
extern void gnss_task(void *pvParameters); 
extern void test_task(void *pvParameters);

bool check_respond(modem_ctx_t *modem) {
	uint8_t data[32];
	for (int j = 0; j < 10; j++) {
		modem_err_t ret = modem_send_command(modem, "AT", data, sizeof(data), 1200);
		if (ret == MODEM_OK) return true;
	}
	return false;
}

esp_err_t init_fs(void) {
// #if CONFIG_EXAMPLE_DEPLOY_WEB_PAGES
    esp_vfs_littlefs_conf_t conf = {
        .base_path = WEB_PAGE_MOUNT_POINT_IN_FS,
        .partition_label = "www",
        .format_if_mount_failed = true,
    };
    esp_err_t ret = esp_vfs_littlefs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find LittleFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize LittleFS (%s)", esp_err_to_name(ret));
        }
        return ESP_FAIL;
    }

    size_t total = 0, used = 0;
    ret = esp_littlefs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get LittleFS partition information (%s)", esp_err_to_name(ret));
        esp_littlefs_format(conf.partition_label);
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }
//#endif // CONFIG_EXAMPLE_DEPLOY_WEB_PAGES
    return ESP_OK;
}

ESP_EVENT_DEFINE_BASE(SMS_EVENTS);
static void sms_new_message_handler(void* event_handler_arg, 
		esp_event_base_t event_base,
		int32_t event_id,
		void* event_data) {
	modem_ctx_t modem;
	ESP_ERROR_CHECK(a7670sa_modem_init(&modem));

	sms_cmti_t *cmti = (sms_cmti_t *)event_data;
	ESP_LOGI(TAG, "new sms event process:");
	sms_process_cmti(&modem, cmti);
}

static void sms_process_all_messages_handler(void* event_handler_arg, 
		esp_event_base_t event_base,
		int32_t event_id,
		void* event_data) {
	modem_ctx_t modem;
	ESP_ERROR_CHECK(a7670sa_modem_init(&modem));

	ESP_LOGI(TAG, "new sms event: processing all message");
	if (s_sms_task_handle) {
		xTaskNotifyGive(s_sms_task_handle);
	}
}

void app_main(void) {
	ESP_ERROR_CHECK(a7670sa_init());

	// Configure esp event loop
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	ESP_ERROR_CHECK(esp_event_handler_instance_register(SMS_EVENTS, SMS_EVENT_NEW_MESSAGE, sms_new_message_handler, NULL, NULL));
	ESP_ERROR_CHECK(esp_event_handler_instance_register(SMS_EVENTS, SMS_EVENT_PROCESS_ALL_MESSAGES, sms_process_all_messages_handler, NULL, NULL));

	// TODO: is very unlike to have a error here... maybe
	modem_driver_init();
	modem_init_pm_locks();
	ESP_ERROR_CHECK(a7670sa_modem_init(&modem)); // abort on fail
																							 
	// Init ADC battery
	board_battery_adc_init();

	// Check whether it has been started
	bool started = check_respond(&modem);
	if (!started) {
		ESP_LOGI(TAG, "Wait modem started...");
		// Wait for the modem to finish booting
		vTaskDelay(pdMS_TO_TICKS(MODEM_START_WAIT_MS));
	}

	// enable light sleep mode
	esp_pm_config_t pm_config = {
		.max_freq_mhz = 240,
		.min_freq_mhz = 80,
		.light_sleep_enable = true
	};

	ESP_ERROR_CHECK(esp_pm_configure(&pm_config));

	xTaskCreate(gnss_task, "gnss_task", 8192, (void*)&modem, 10, NULL);
	xTaskCreate(uart_event_task, "uart_event_task", 3072, (void*)&modem, 10, NULL);

	xTaskCreate(sms_worker_task, "sms_worker_task", 8192, (void*)&modem, 12, &s_sms_task_handle);
	a7670sa_set_s_sms_task_handle(s_sms_task_handle);
	if (a7670sa_setup_ri_wakeup() != ESP_OK) {
		ESP_LOGE(TAG, "Cannot init a7570sa wakeup pin. SMS events cannot works properly.");
	}

	//xTaskCreate(test_task, "test_task", 8192, (void*)&modem, 12, NULL);


	//test captive_portal
	//ESP_ERROR_CHECK(init_fs());
	//captive_portal_start(WEB_PAGE_MOUNT_POINT_IN_FS);
}
