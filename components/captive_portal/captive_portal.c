#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

//#include <sys/param.h>

#include "esp_event.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_netif.h"
//#include "lwip/inet.h"
#include "esp_http_server.h"

#include "dns_server.h"
#include "captive_portal.h"
#include "preferences.h"

#define EXAMPLE_MAX_STA_CONN 2

static const char *TAG = "captive_portal";
static esp_netif_t *s_ap_netif = NULL;

extern esp_err_t start_webserver(modem_ctx_t *modem, const char *static_assets_path);

static void handler_on_wifi_ap_statconnect(void *arg, esp_event_base_t event_base,
		int32_t event_id, void *event_data) {
	wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
	ESP_LOGI(TAG, "station " MACSTR " join, AID=%d",
			MAC2STR(event->mac), event->aid);
}

static void handler_on_wifi_ap_statdisconnect(void *arg, esp_event_base_t event_base,
		int32_t event_id, void *event_data) {
	wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
	ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d, reason=%d",
			MAC2STR(event->mac), event->aid, event->reason);
}

static void initialize_networking_stack() {
	//esp_log_level_set("httpd_uri", ESP_LOG_ERROR);
	//esp_log_level_set("httpd_txrx", ESP_LOG_ERROR);
	//esp_log_level_set("httpd_parse", ESP_LOG_ERROR);
	//
	ESP_ERROR_CHECK(esp_netif_init());
	//ESP_ERROR_CHECK(esp_event_loop_create_default());

	// Initialize NVS needed by Wi-Fi
	ESP_ERROR_CHECK(nvs_flash_init());

	// Initialize Wi-Fi including netif with default config
	s_ap_netif = esp_netif_create_default_wifi_ap();
}


static void wifi_init_softap() {
//	const p_app_config_t p_app_config = p_app_config_get();
	//const char *p_wifi_ap_ssid = p_app_config.p_wifi_ap_ssid;
	//const char *p_wifi_ap_password = p_app_config.p_wifi_ap_password;

	char p_wifi_ap_ssid[P_STR_BUF_SIZE] = {0};
	p_app_config_get(P_WIFI_AP_SSID_KEY, p_wifi_ap_ssid);
	char p_wifi_ap_password[P_STR_BUF_SIZE] = {0};
	p_app_config_get(P_WIFI_AP_PASSWORD_KEY, p_wifi_ap_password);

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_AP_STACONNECTED, &handler_on_wifi_ap_statconnect, NULL));
	ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_AP_STADISCONNECTED, &handler_on_wifi_ap_statdisconnect, NULL));

	wifi_config_t wifi_config = {
		.ap = {
			.ssid_len = strlen(p_wifi_ap_ssid),
			.max_connection = EXAMPLE_MAX_STA_CONN,
			.authmode = WIFI_AUTH_WPA_WPA2_PSK
		},
	};
	memcpy(wifi_config.ap.ssid, p_wifi_ap_ssid, strlen(p_wifi_ap_ssid));
	memcpy(wifi_config.ap.password, p_wifi_ap_password, strlen(p_wifi_ap_password));

	if (strlen(p_wifi_ap_password) == 0) {
		wifi_config.ap.authmode = WIFI_AUTH_OPEN;
	}

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
	ESP_ERROR_CHECK(esp_wifi_start());

	esp_netif_ip_info_t ip_info;
	esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("WIFI_AP_DEF"), &ip_info);

	char ip_addr[16];
	inet_ntoa_r(ip_info.ip.addr, ip_addr, 16);
	ESP_LOGI(TAG, "Set up softAP with IP: %s", ip_addr);

	ESP_LOGI(TAG, "wifi_init_softap finished. SSID:'%s' password:'%s'",
			p_wifi_ap_ssid, p_wifi_ap_password);
}

static void dhcp_set_captiveportal_url(void) {
	// get the IP of the access point to redirect to
	esp_netif_ip_info_t ip_info;
	esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("WIFI_AP_DEF"), &ip_info);

	char ip_addr[16];
	inet_ntoa_r(ip_info.ip.addr, ip_addr, 16);
	ESP_LOGI(TAG, "Set up softAP with IP: %s", ip_addr);

	// turn the IP into a URI
	char* captiveportal_uri = (char*) malloc(32 * sizeof(char));
	assert(captiveportal_uri && "Failed to allocate captiveportal_uri");
	strcpy(captiveportal_uri, "http://");
	strcat(captiveportal_uri, ip_addr);

	// get a handle to configure DHCP with
	esp_netif_t* netif = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");

	// set the DHCP option 114
	ESP_ERROR_CHECK_WITHOUT_ABORT(esp_netif_dhcps_stop(netif));
	ESP_ERROR_CHECK(esp_netif_dhcps_option(netif, ESP_NETIF_OP_SET, ESP_NETIF_CAPTIVEPORTAL_URI, captiveportal_uri, strlen(captiveportal_uri)));
	ESP_ERROR_CHECK_WITHOUT_ABORT(esp_netif_dhcps_start(netif));
}


void captive_portal_start(modem_ctx_t *modem, const char *web_page_mount_point_in_fs) {
	initialize_networking_stack();
	wifi_init_softap();

	dhcp_set_captiveportal_url();

	start_webserver(modem, web_page_mount_point_in_fs);

	// Start the DNS server that will redirect all queries to the softAP IP
	dns_server_config_t config = DNS_SERVER_CONFIG_SINGLE("*" /* all A queries */, "WIFI_AP_DEF" /* softAP netif ID */);
	start_dns_server(&config);
}

void captive_portal_stop() {
	ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_AP_STACONNECTED, &handler_on_wifi_ap_statconnect));
	ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_AP_STADISCONNECTED, &handler_on_wifi_ap_statdisconnect));

	esp_err_t err = esp_wifi_stop();
	if (err == ESP_ERR_WIFI_NOT_INIT) {
		return;
	}
	ESP_ERROR_CHECK(err);
	ESP_ERROR_CHECK(esp_wifi_deinit());
	ESP_ERROR_CHECK(esp_wifi_clear_default_wifi_driver_and_handlers(s_ap_netif));
	esp_netif_destroy(s_ap_netif);
	s_ap_netif = NULL;
}


