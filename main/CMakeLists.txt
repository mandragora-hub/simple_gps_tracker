idf_component_register(SRCS "simple_gps_tracker_main.c" 
	                          "modem_board.c"
														"at/modem.c" 
														"at/gnss.c" 
														"at/http.c" 
														"at/sim_card.c"
														"at/status_control.c"
														"at/network.c"
														"at/packet_domain.c"
														"at/sms.c"
														"at/serial_interface.c"
														"geodesic.c"
														"parse_at.c"
														"string_utils.c"
														"sms_utils.c"
														"utils.c"
                    REQUIRES 
											esp_driver_uart 
											esp_driver_gpio 
											esp_event 
											esp_timer 
											esp_adc 
											esp_pm
                    INCLUDE_DIRS ".")

																		##if(CONFIG_EXAMPLE_DEPLOY_WEB_PAGES)
    set(WEB_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../frontend")
    if(EXISTS ${WEB_SRC_DIR}/dist)
        littlefs_create_partition_image(www ${WEB_SRC_DIR}/dist FLASH_IN_PROJECT)
    else()
        message(FATAL_ERROR "'${WEB_SRC_DIR}/dist' doesn't exist. Please run 'pnpm build' under '${WEB_SRC_DIR}'")
    endif()
		##endif()
