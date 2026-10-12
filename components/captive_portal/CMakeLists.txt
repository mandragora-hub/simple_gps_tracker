idf_component_register(SRCS "captive_portal.c" "rest_server.c"
										PRIV_REQUIRES esp_event nvs_flash esp_wifi esp_http_server
                    INCLUDE_DIRS "include")


