#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <stdint.h>
#include "esp_err.h"

#define P_NVS_NAMESPACE						"preferences"

#define P_WIFI_AP_SSID_KEY 					0x1
#define P_WIFI_AP_PASSWORD_KEY 			0x2
#define P_OSMAND_URL_KEY			 			0x3
#define P_REPORTING_INTERVAL_MS_KEY 0x4
#define P_OSMAND_DEVICE_ID_KEY 			0x5

#define P_STR_TYPE									"str"
#define P_U32_TYPE									"u32"

#define P_STR_BUF_SIZE							(256)				

#define P_KEY_TBL_IT(key, nvs_key, app_config_attr, type)    {key, nvs_key, app_config_attr, type}

typedef uint8_t p_key_t;

typedef struct {
	p_key_t p_key;
	const char *p_nvs_key;
	const char *app_config_attr;
	const char *p_datatype;
} p_pair_t;

typedef struct {
	char p_osmand_url[P_STR_BUF_SIZE];
	char p_osmand_device_id[P_STR_BUF_SIZE];
	uint32_t p_reporting_interval_ms;
	char p_wifi_ap_ssid[P_STR_BUF_SIZE];
	char p_wifi_ap_password[P_STR_BUF_SIZE];
} p_app_config_t;

esp_err_t p_app_config_get(p_key_t p_key, void *out_v);
esp_err_t p_app_config_set_value(const char *app_config_attr, void *value);

#endif // PREFERENCES_H

