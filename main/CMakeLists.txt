idf_component_register(SRCS "simple_gps_tracker_main.c" 
														"modem.c" 
														"gnss.c" 
														"http.c" 
														"sim_card.c"
														"status_control.c"
														"network.c"
														"packet_domain.c"
														"parse_at.c"
														"string_utils.c"
														"utils.c"
                    REQUIRES esp_driver_uart esp_driver_gpio
                    INCLUDE_DIRS ".")
