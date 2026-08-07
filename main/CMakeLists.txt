idf_component_register(SRCS "simple_gps_tracker_main.c" 
														"at/modem.c" 
														"at/gnss.c" 
														"at/http.c" 
														"at/sim_card.c"
														"at/status_control.c"
														"at/network.c"
														"at/packet_domain.c"
														"at/sms.c"
														"parse_at.c"
														"string_utils.c"
														"utils.c"
                    REQUIRES esp_driver_uart esp_driver_gpio esp_event
                    INCLUDE_DIRS ".")
