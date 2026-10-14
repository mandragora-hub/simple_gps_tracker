idf_component_register(SRCS "simple_gps_tracker_main.c" 
	                          "board.c"
														"geodesic.c"
														"sms_utils.c"
														"utils.c"
														"tasks.c"
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
