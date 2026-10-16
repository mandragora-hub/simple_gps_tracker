#include <string.h>

#include "esp_log.h"
#include "esp_http_server.h"
#include "esp_check.h"
#include "esp_chip_info.h"
#include "cJSON.h"
#include "modem.h"
#include "at/v_25ter.h"

//#include "esp_vfs.h"
#define ESP_VFS_PATH_MAX 15 // TODO: use the var in esp_vfs.h

#define FILE_PATH_MAX (ESP_VFS_PATH_MAX + 128)
#define SCRATCH_BUFSIZE (10240)
typedef struct rest_server_context {
	char base_path[ESP_VFS_PATH_MAX + 1];
	char scratch[SCRATCH_BUFSIZE];
	modem_ctx_t *modem;
} rest_server_context_t;

#define CHECK_FILE_EXTENSION(filename, ext) (strcasecmp(&filename[strlen(filename) - strlen(ext)], ext) == 0)

static const char *TAG = "rest_server";

static esp_err_t set_content_type_from_file(httpd_req_t *req, const char *filepath) {
	const char *type = "text/plain";
	if (CHECK_FILE_EXTENSION(filepath, ".html")) {
		type = "text/html";
	} else if (CHECK_FILE_EXTENSION(filepath, ".js")) {
		type = "application/javascript";
	} else if (CHECK_FILE_EXTENSION(filepath, ".css")) {
		type = "text/css";
	} else if (CHECK_FILE_EXTENSION(filepath, ".png")) {
		type = "image/png";
	} else if (CHECK_FILE_EXTENSION(filepath, ".ico")) {
		type = "image/x-icon";
	} else if (CHECK_FILE_EXTENSION(filepath, ".svg")) {
		type = "text/xml";
	}
	return httpd_resp_set_type(req, type);
}

static esp_err_t root_get_handler(httpd_req_t *req) {
	rest_server_context_t *rest_context = (rest_server_context_t *)req->user_ctx;

	char filepath[FILE_PATH_MAX];
	strlcpy(filepath, rest_context->base_path, sizeof(filepath));
	if (req->uri[strlen(req->uri) - 1] == '/') {
		strlcat(filepath, "/index.html", sizeof(filepath));
	} else {
		strlcat(filepath, req->uri, sizeof(filepath));
	}
	int fd = open(filepath, O_RDONLY, 0);
	if (fd == -1) {
		ESP_LOGE(TAG, "Failed to open file : %s", filepath);
		/* Respond with 500 Internal Server Error */
		httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read existing file");
		return ESP_FAIL;
	}

	set_content_type_from_file(req, filepath);

	char *chunk = rest_context->scratch;
	ssize_t read_bytes;
	do {
		/* Read file in chunks into the scratch buffer */
		read_bytes = read(fd, chunk, SCRATCH_BUFSIZE);
		if (read_bytes == -1) {
			ESP_LOGE(TAG, "Failed to read file : %s", filepath);
		} else if (read_bytes > 0) {
			/* Send the buffer contents as HTTP response chunk */
			if (httpd_resp_send_chunk(req, chunk, read_bytes) != ESP_OK) {
				close(fd);
				ESP_LOGE(TAG, "File sending failed!");
				/* Abort sending file */
				httpd_resp_sendstr_chunk(req, NULL);
				/* Respond with 500 Internal Server Error */
				httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
				return ESP_FAIL;
			}
		}
	} while (read_bytes > 0);
	/* Close file after sending complete */
	close(fd);
	ESP_LOGI(TAG, "File sending complete");
	/* Respond with an empty chunk to signal HTTP response completion */
	httpd_resp_send_chunk(req, NULL, 0);
	return ESP_OK;
}

// HTTP Error (404) Handler - Redirects all requests to the root page
static esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err)
{
	// Set status
	httpd_resp_set_status(req, "303 See Other");
	// Redirect to the "/" root directory
	httpd_resp_set_hdr(req, "Location", "/");
	// iOS requires content in the response to detect a captive portal, simply redirecting is not sufficient.
	httpd_resp_send(req, "Redirect to the captive portal", HTTPD_RESP_USE_STRLEN);

	ESP_LOGI(TAG, "Redirecting to root");
	return ESP_OK;
}

static esp_err_t hostpot_detect_get_handler(httpd_req_t *req) {
	httpd_err_code_t err = HTTPD_404_NOT_FOUND;
	return http_404_error_handler(req, err);
}

static esp_err_t system_info_get_handler(httpd_req_t *req) {
	rest_server_context_t *rest_context = (rest_server_context_t *)req->user_ctx;

	httpd_resp_set_type(req, "application/json");
	cJSON *root = cJSON_CreateObject();

	esp_chip_info_t chip_info;
	esp_chip_info(&chip_info);
	cJSON_AddStringToObject(root, "chip", CONFIG_IDF_TARGET);
	cJSON_AddStringToObject(root, "idf_version", IDF_VER);
	cJSON_AddNumberToObject(root, "cores", chip_info.cores);

	bool modem_status_ok = v_25ter_check_at(rest_context->modem) == MODEM_OK ? true : false;
	v_25ter_product_id_info_t v_25ter_product_id_info;
	v_25ter_get_product_id_info(rest_context->modem,  &v_25ter_product_id_info);
	cJSON_AddBoolToObject(root, "modem_status_ok", modem_status_ok);
	cJSON_AddStringToObject(root, "modem_manufacturer", v_25ter_product_id_info.manufacturer);
	cJSON_AddStringToObject(root, "modem_model", v_25ter_product_id_info.model);
	cJSON_AddStringToObject(root, "modem_revision", v_25ter_product_id_info.revision);
	cJSON_AddStringToObject(root, "modem_imei", v_25ter_product_id_info.imei);
	cJSON_AddStringToObject(root, "modem_gcap", v_25ter_product_id_info.gcap);

	const char *sys_info = cJSON_Print(root);
	httpd_resp_sendstr(req, sys_info);
	free((void *)sys_info);
	cJSON_Delete(root);
	return ESP_OK;
}

esp_err_t start_webserver(modem_ctx_t *modem, const char *static_assets_path) {
	ESP_RETURN_ON_FALSE(static_assets_path && strlen(static_assets_path) < ESP_VFS_PATH_MAX, ESP_ERR_INVALID_ARG, TAG, "Invalid static assets path");
	rest_server_context_t *rest_context = calloc(1, sizeof(rest_server_context_t));
	ESP_RETURN_ON_FALSE(rest_context, ESP_ERR_NO_MEM, TAG, "No memory for rest context");

	strlcpy(rest_context->base_path, static_assets_path, sizeof(rest_context->base_path));
	rest_context->modem = modem;

	httpd_handle_t server = NULL;
	httpd_config_t config = HTTPD_DEFAULT_CONFIG();
	//config.max_open_sockets = 5;
	//config.lru_purge_enable = true;
	config.uri_match_fn = httpd_uri_match_wildcard;

	// Start the httpd server
	ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
	if (httpd_start(&server, &config) != ESP_OK) {
		if (rest_context) {
			free(rest_context);
		}
		return ESP_FAIL;
	}

	ESP_LOGI(TAG, "Registering URI handlers");
	const httpd_uri_t hostpot_detect_get_uri = {
		.uri = "/hotspot-detect.html",
		.method = HTTP_GET,
		.handler = hostpot_detect_get_handler,
		.user_ctx = rest_context
	};
	const httpd_uri_t system_info_get_uri = {
		.uri = "/api/v1/system/info",
		.method = HTTP_GET,
		.handler = system_info_get_handler,
		.user_ctx = rest_context
	};
	const httpd_uri_t root_get_uri = {
		.uri = "/*",
		.method = HTTP_GET,
		.handler = root_get_handler,
		.user_ctx = rest_context
	};
	httpd_register_uri_handler(server, &hostpot_detect_get_uri);
	httpd_register_uri_handler(server, &system_info_get_uri);
	httpd_register_uri_handler(server, &root_get_uri);
	httpd_register_err_handler(server, HTTPD_404_NOT_FOUND, http_404_error_handler);

	return ESP_OK;
}
