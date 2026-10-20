#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "preferences.h"

#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"

static const char *TAG = "preferences";
static nvs_handle_t p_nvs_handler;

static SemaphoreHandle_t preferences_mutex;

static bool initialized = false;

static const p_pair_t p_pair[] = {
	P_KEY_TBL_IT(P_WIFI_AP_SSID_KEY, "pwask", "p_wifi_ap_ssid", P_STR_TYPE),
	P_KEY_TBL_IT(P_WIFI_AP_PASSWORD_KEY, "pwapk", "p_wifi_ap_password", P_STR_TYPE),
	P_KEY_TBL_IT(P_OSMAND_URL_KEY, "pouk", "p_osmand_url", P_STR_TYPE),
	P_KEY_TBL_IT(P_OSMAND_DEVICE_ID_KEY, "podik", "p_osmand_device_id", P_STR_TYPE),
	P_KEY_TBL_IT(P_REPORTING_INTERVAL_MS_KEY, "pru32k", "p_reporting_interval_ms", P_U32_TYPE),
};

static const size_t P_PAIR_SIZE = sizeof(p_pair) / sizeof(p_pair[0]);

static p_app_config_t p_app_config = { // default values
	.p_osmand_url = "https://osmand.traccar.redania.online",
	.p_osmand_device_id = "TEST_D01",
	.p_reporting_interval_ms = 30000,
	.p_wifi_ap_ssid = "simplegps-ap",
	.p_wifi_ap_password ="12345678a",
};

static const p_pair_t *p_key_to_pair(p_key_t p_key) {
	for (int i = 0; i < P_PAIR_SIZE; i++) {
		const p_pair_t *pair = &p_pair[i];
		if (pair->p_key == p_key) return pair;
	}
	return NULL;
}

static bool p_key_exist(p_key_t p_key) {
	const p_pair_t *pair = p_key_to_pair(p_key);
	if (pair == NULL) return false;
	return true;
}

static const p_pair_t *app_config_attr_to_p_pair(const char *app_config_attr) {
	for (int i = 0; i < P_PAIR_SIZE; i++) {
		const p_pair_t *pair = &p_pair[i];
		if (strcmp(pair->app_config_attr, app_config_attr) == 0) return pair;
	}
	return NULL;
}

static esp_err_t p_set_str_value(const char *p_nvs_key, const char *input_str) {
	ESP_LOGI(TAG, "Writing string to NVS...");
	esp_err_t err = nvs_set_str(p_nvs_handler, p_nvs_key, input_str);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "Failed to write string!, %s", esp_err_to_name(err));
	}
	return err;
}

static esp_err_t p_set_u32_value(const char *p_nvs_key, uint32_t *input) {
	ESP_LOGI(TAG, "Writing uint32 to NVS...");
	esp_err_t err = nvs_set_u32(p_nvs_handler, p_nvs_key, *input);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "Failed to write uint32!, %s", esp_err_to_name(err));
	}
	return err;
}

static esp_err_t p_get_u32_value(const char *p_nvs_key, uint32_t *out) {
	ESP_LOGI(TAG, "Reading uint32 from NVS...");
	return nvs_get_u32(p_nvs_handler, p_nvs_key, out);
}

static esp_err_t p_get_str_value(const char *p_nvs_key, char *out_str) {
	size_t required_size = 0;
	ESP_LOGI(TAG, "Reading string from NVS...");
	esp_err_t err = nvs_get_str(p_nvs_handler, p_nvs_key, NULL, &required_size);
	if (err == ESP_OK) {
		char* text = malloc(required_size);
		err = nvs_get_str(p_nvs_handler, p_nvs_key, text, &required_size);
		if (err == ESP_OK) {
			strcpy(out_str, text);
			//out_str[required_size] = '\0';
			//printf("Read string: %s\n", out_str);
		}
		free(text);
	}
	return err;
}

static esp_err_t p_get_value(p_key_t p_key, void *out_v) {
	const p_pair_t *pair = p_key_to_pair(p_key);
	if (pair == NULL) { 
		ESP_LOGE(TAG, "Cannot find pair from p_key = %d", p_key);
		return ESP_FAIL; 
	}

	if (strcmp(pair->p_datatype, P_STR_TYPE) == 0) {
		char *out_str = (char*)out_v;
		return p_get_str_value(pair->p_nvs_key, out_str);
	}

	if (strcmp(pair->p_datatype, P_U32_TYPE) == 0) {
		uint32_t *u32 = (uint32_t*)out_v;
		return p_get_u32_value(pair->p_nvs_key, u32);
	}

	return ESP_FAIL;
}

static void p_app_config_update() {
	for (int i = 0; i < P_PAIR_SIZE; i++) {
		const p_pair_t *pair = &p_pair[i];
		const p_key_t p_key = pair->p_key;

		if (strcmp(pair->p_datatype, P_U32_TYPE) == 0) {
			uint32_t out_v;
			esp_err_t err = p_get_value(p_key, (void*)&out_v);
			if (err == ESP_OK) { 
				if (p_key == P_REPORTING_INTERVAL_MS_KEY) memcpy(&p_app_config.p_reporting_interval_ms, &out_v, sizeof(out_v));
			} 
		}

		if (strcmp(pair->p_datatype, P_STR_TYPE) == 0) {
			char out_v[P_STR_BUF_SIZE] = {0};
			esp_err_t err = p_get_value(p_key, (void*)&out_v);
			if (err == ESP_OK) { 
				if (p_key == P_WIFI_AP_SSID_KEY) strcpy(p_app_config.p_wifi_ap_ssid, out_v);
				if (p_key == P_WIFI_AP_PASSWORD_KEY) strcpy(p_app_config.p_wifi_ap_password, out_v);
				if (p_key == P_OSMAND_URL_KEY) strcpy(p_app_config.p_osmand_url, out_v);
				if (p_key == P_OSMAND_DEVICE_ID_KEY) strcpy(p_app_config.p_osmand_device_id, out_v);
			} 
		}
	}
}

static void p_init() {
	if (initialized) return;

	preferences_mutex = xSemaphoreCreateMutex();

	esp_err_t err = nvs_flash_init();
	if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		err = nvs_flash_init();
	}
	ESP_ERROR_CHECK(err);

	ESP_LOGI(TAG, "Opening Non-Volatile Storage (NVS) handle...");
	err = nvs_open(P_NVS_NAMESPACE, NVS_READWRITE, &p_nvs_handler);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
		return;
	}

	p_app_config_update();

	initialized = true;
}

esp_err_t p_app_config_get(p_key_t p_key, void *out_v) {
	p_init();

	if (xSemaphoreTake(preferences_mutex, portMAX_DELAY) == pdFALSE) return ESP_FAIL;

	if (!p_key_exist(p_key)) {
		xSemaphoreGive(preferences_mutex);
		return ESP_FAIL;
	}

	if (p_key == P_WIFI_AP_SSID_KEY) {
		strcpy(out_v, p_app_config.p_wifi_ap_ssid);
	}
	if (p_key == P_WIFI_AP_PASSWORD_KEY) {
		strcpy(out_v, p_app_config.p_wifi_ap_password);
	}
	if (p_key == P_OSMAND_URL_KEY) {
		strcpy(out_v, p_app_config.p_osmand_url);
	}
	if (p_key == P_OSMAND_DEVICE_ID_KEY) {
		strcpy(out_v, p_app_config.p_osmand_device_id);
	}
	if (p_key == P_REPORTING_INTERVAL_MS_KEY) {
		uint32_t src = p_app_config.p_reporting_interval_ms;
		memcpy(out_v, &src, sizeof(src)); // cpy value not references
	}

	xSemaphoreGive(preferences_mutex);
	return ESP_OK;
}

esp_err_t p_app_config_set_value(const char *app_config_atrr, void *value) {
	p_init(); // nvs handler should be initialized after use the function

	if (xSemaphoreTake(preferences_mutex, portMAX_DELAY) == pdFALSE) return ESP_FAIL;

	const p_pair_t *pair = app_config_attr_to_p_pair(app_config_atrr);
	if (pair == NULL) {
		ESP_LOGE(TAG, "Cannot find pair from app_config_atrr = %s", app_config_atrr);
		xSemaphoreGive(preferences_mutex);
		return ESP_FAIL; 
	}

	const p_key_t p_key = pair->p_key;

	if (strcmp(pair->p_datatype, P_STR_TYPE) == 0) {
		esp_err_t ret = p_set_str_value(pair->p_nvs_key, value);
		if (ret == ESP_OK) {
			if (p_key == P_WIFI_AP_SSID_KEY) strcpy(p_app_config.p_wifi_ap_ssid, value);
			if (p_key == P_WIFI_AP_PASSWORD_KEY) strcpy(p_app_config.p_wifi_ap_password, value);
			if (p_key == P_OSMAND_URL_KEY) strcpy(p_app_config.p_osmand_url, value);
			if (p_key == P_OSMAND_DEVICE_ID_KEY) strcpy(p_app_config.p_osmand_device_id, value);
		}
	}

	if (strcmp(pair->p_datatype, P_U32_TYPE) == 0) {
		uint32_t *new_value = (uint32_t*)&value;
		esp_err_t ret = p_set_u32_value(pair->p_nvs_key, new_value);
		if (ret == ESP_OK) {
			if (p_key == P_REPORTING_INTERVAL_MS_KEY) memcpy(&p_app_config.p_reporting_interval_ms, new_value, sizeof(new_value));
		}
	}

	xSemaphoreGive(preferences_mutex);
	return ESP_FAIL;
}
