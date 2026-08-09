#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "sdkconfig.h"
#include "modem_board.h"
#include "esp_log.h"
#include "esp_event.h"
#include "at/modem.h"
#include "at/gnss.h"
#include "at/http.h"
#include "at/status_control.h"
#include "at/sim_card.h"
#include "at/network.h"
#include "at/packet_domain.h"
#include "at/sms.h"
#include "utils.h"
#include "geodesic.h"

#define MODEM_UART_TXD 26
#define MODEM_UART_RXD 27
#define MODEM_UART_RTS (UART_PIN_NO_CHANGE)
#define MODEM_UART_CTS (UART_PIN_NO_CHANGE)

#define UART_PORT_NUM 2
#define MODEM_UART_BAUD_RATE 115200

#define UART_RX_BUF_SIZE (1024)

static const char *TAG = "simple_gps_tracker_debug";

static QueueHandle_t uart_queue;

uart_config_t init_uart_config() {
	uart_config_t uart_config = {
		.baud_rate = MODEM_UART_BAUD_RATE,
		.data_bits = UART_DATA_8_BITS,
		.parity    = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
		.source_clk = UART_SCLK_DEFAULT,
	};
	return uart_config;
}

bool check_respond() {
	uint8_t data[10];
	modem_ctx_t modem;
	modem_init(&modem, UART_PORT_NUM);
	for (int j = 0; j < 10; j++) {
		modem_err_t ret = modem_send_command(&modem, "AT", data, sizeof(data), 1200);
		if (ret == MODEM_OK) return true;
	}
	return false;
}

static void uart_event_task(void *pvParameters) {
	uart_event_t event;

	modem_ctx_t modem;
	modem_init(&modem, UART_PORT_NUM);

	for (;;) {
		if (xQueueReceive(uart_queue, (void *)&event, (TickType_t)portMAX_DELAY)) {
			switch (event.type) {
				case UART_FIFO_OVF:
					ESP_LOGI(TAG, "hw fifo overflow");
					// If fifo overflow happened, you should consider adding flow control for your application.
					// The ISR has already reset the rx FIFO,
					// As an example, we directly flush the rx buffer here in order to read more data.
					uart_flush_input(UART_PORT_NUM);
					xQueueReset(uart_queue);
					break;
				case UART_BUFFER_FULL:
					ESP_LOGI(TAG, "ring buffer full");
					// If buffer full happened, you should consider increasing your buffer size
					// As an example, we directly flush the rx buffer here in order to read more data.
					uart_flush_input(UART_PORT_NUM);
					xQueueReset(uart_queue);
					break;
				case UART_BREAK:
					ESP_LOGI(TAG, "uart rx break");
					break;
				case UART_PARITY_ERR:
					ESP_LOGI(TAG, "uart parity error");
					break;
				case UART_FRAME_ERR:
					ESP_LOGI(TAG, "uart frame error");
					break;
				case UART_PATTERN_DET:
					uint8_t buffer[1024] = {0};
					size_t buffered_size = sizeof(buffer); 
					modem_read_uart(&modem, NULL, buffer, buffered_size, 200);
					//ESP_LOGI(TAG, "detect data pattern: %s", buffer);
					sms_cmti_t cmti = {0};
					bool new = sms_process_uart_pattern_event((char*)buffer, &cmti);
					if (new) {
						ESP_LOGI(TAG, "cmti.mem = %s", cmti.mem);
						ESP_LOGI(TAG, "cmti.index = %d", cmti.index);
						ESP_ERROR_CHECK(esp_event_post(SMS_EVENTS, SMS_EVENT_NEW_MESSAGE, &cmti, sizeof(cmti), portMAX_DELAY));
					}
					break;
				default: break;
			}
		}
	}
	vTaskDelete(NULL);
} 

static void gnss_task(void *pvParameters) {
	modem_ctx_t modem;
	modem_init(&modem, UART_PORT_NUM);

	gnss_info_t last_sent_gnss_info = {0};

	if (gnss_power_on(&modem) != MODEM_OK) {
		ESP_LOGE(TAG, "Failed to power GNSS module");
		// Handle failure. TODO: maybe we need to sleep for a while and retry
		vTaskDelete(NULL);
	}

	// TODO: define criteria to use the different start mode
	gnss_hot_start(&modem);

	ESP_LOGI(TAG, "GNSS powered on. Beginning acquisition loop...");

	for (;;) {
		if (gnss_has_fixed_position(&modem)) {
			gnss_info_t gnss_info = {0};
			gnss_get_position(&gnss_info);
			ESP_LOGI(TAG, "Lat %.6f, Lon %.6f", gnss_info.latitude, gnss_info.longitude);

			// TODO: validate the new location is significatly different of last location. 
			// Propose: update devices when location is different or certain time has passed
			double distances_m = 99;
			distances_m = calculate_geodesic_distances_gnss(&gnss_info, &last_sent_gnss_info);
			printf("distances_m =  %lf\n", distances_m);
			if (distances_m > 5 )  {
				http_request_t request = {0};
				http_response_t response = {0};

				char osmand_traccar_url[100] = {0};
				build_osmand_traccar_url(osmand_traccar_url, sizeof(osmand_traccar_url), &gnss_info);

				strcpy(request.url, osmand_traccar_url);
				printf("request.url = %s\n", request.url);

				if ((http_perform_action(&modem, &request, &response) == true)) {
					memcpy(&last_sent_gnss_info, &gnss_info, sizeof(gnss_info));

					printf("Http sucessfully operation\n");
					printf("response.statuscode = %d\n", response.statuscode);
					printf("response.datalen = %d\n", response.datalen);
					printf("response.content = %s\n", response.content);
				}	
			}
		} else {
			ESP_LOGI(TAG, "Waiting for satellite fix...");
		}
		vTaskDelay(pdMS_TO_TICKS(20000));
		remaining_task_stack();
	}

	gnss_sleep(&modem);
	vTaskDelete(NULL);
} 

// TODO: Convert this in a init_test_routine is every ok then letsgo
static void test_task(void *pvParameters) {
	modem_ctx_t modem;
	modem_init(&modem, UART_PORT_NUM);

	for (;;) {
		if (sim_card_is_ready(&modem)) {
			ESP_LOGI(TAG, "SIM Card is ready.");
		} else {
			ESP_LOGE(TAG, "SIM Card error.");
		}
		vTaskDelay(pdMS_TO_TICKS(1000)); 

		signal_quality_t signal_quality;
		status_control_query_signal_quality(&modem, &signal_quality);
		ESP_LOGI(TAG, "signal_quality.rssi = %d", signal_quality.rssi);
		ESP_LOGI(TAG, "signal_quality.ber = %d", signal_quality.ber);
		vTaskDelay(pdMS_TO_TICKS(1000)); 

		ue_system_information_t ue;
		network_query_ue_sys_information(&modem, &ue);
		ESP_LOGI(TAG, "ue.system_mode = %s", ue.system_mode);
		ESP_LOGI(TAG, "ue.operation_mode = %s", ue.operation_mode);
		vTaskDelay(pdMS_TO_TICKS(1000)); 


		network_registration_t network_registration;
		network_read_network_registration(&modem, &network_registration);
		ESP_LOGI(TAG, "network_registration.n = %d", network_registration.n);
		ESP_LOGI(TAG, "network_registration.stat = %d", network_registration.stat);
		vTaskDelay(pdMS_TO_TICKS(1000)); 

		operator_selection_t operator_selection;
		network_read_operator_selection(&modem, &operator_selection);
		ESP_LOGI(TAG, "operator_selection.mode = %d", operator_selection.mode);
		ESP_LOGI(TAG, "operator_selection.format = %d", operator_selection.format);
		ESP_LOGI(TAG, "operator_selection.oper = %s", operator_selection.oper);
		ESP_LOGI(TAG, "operator_selection.act = %d", operator_selection.act);
		vTaskDelay(pdMS_TO_TICKS(1000)); 

		packet_data_protocol_t pdp;
		packet_domain_read_pdp_context(&modem, &pdp);
		ESP_LOGI(TAG, "pdp.cid = %d", pdp.cid);
		ESP_LOGI(TAG, "pdp.pdp_type = %s", pdp.pdp_type);
		ESP_LOGI(TAG, "pdp.apn = %s", pdp.apn);
		ESP_LOGI(TAG, "pdp.apn_addr = %s", pdp.apn_addr);
		ESP_LOGI(TAG, "pdp.d_comp = %d", pdp.d_comp);
		ESP_LOGI(TAG, "pdp.h_comp = %d", pdp.h_comp);
		vTaskDelay(pdMS_TO_TICKS(1000)); 

		sms_service_centre_address_t sca;
		sms_read_sca(&modem, &sca);
		ESP_LOGI(TAG, "sca.sca = %d", sca.sca);
		ESP_LOGI(TAG, "sca.tosca = %d", sca.tosca);
		vTaskDelay(pdMS_TO_TICKS(1000)); 

		sms_preferred_message_storage_t pms[3];
		sms_read_preferred_message_storage(&modem, pms);
		for (int i = 0; i < 3; i++) {
			ESP_LOGI(TAG, "pms[%d].mem = %s",i, pms[i].mem);
			ESP_LOGI(TAG, "pms[%d].used = %d",i, pms[i].used);
			ESP_LOGI(TAG, "pms[%d].total = %d",i, pms[i].total);
		}
		vTaskDelay(pdMS_TO_TICKS(1000)); 

		sms_select_te_character_set(&modem, SMS_CHARACTER_SET_GSM);
		vTaskDelay(pdMS_TO_TICKS(1000)); 

		sms_select_message_format(&modem, TEXT_MODE);
		vTaskDelay(pdMS_TO_TICKS(1000)); 

		//sms_send_message(&modem, "+18298086111", "Hello from ESP32");
		//vTaskDelay(pdMS_TO_TICKS(1000)); 

		sms_message_t message = {0};
		sms_read_message(&modem, 3, &message);
		ESP_LOGI(TAG, "message.index = %d", message.index);
		ESP_LOGI(TAG, "message.stat = %d", message.stat);
		ESP_LOGI(TAG, "message.oa_da = %s", message.oa_da);
		ESP_LOGI(TAG, "message.alpha = %s", message.alpha);
		ESP_LOGI(TAG, "message.scts = %s", message.scts);
		ESP_LOGI(TAG, "message.data = |%s|", message.data);
		vTaskDelay(pdMS_TO_TICKS(1000)); 


		status_control_read_clock(&modem);
		vTaskDelay(pdMS_TO_TICKS(5000)); 

		remaining_task_stack();
	}

	vTaskDelete(NULL);
}

ESP_EVENT_DEFINE_BASE(SMS_EVENTS);
static void sms_new_message_handler(void* event_handler_arg, 
		esp_event_base_t event_base,
		int32_t event_id,
		void* event_data) {
	modem_ctx_t modem;
	modem_init(&modem, UART_PORT_NUM);

	sms_cmti_t *cmti = (sms_cmti_t *)event_data;
	ESP_LOGI(TAG, "new sms event process:");
	sms_process_cmti(&modem, cmti);
}

void app_main(void) {
	ESP_ERROR_CHECK(modem_board_init_and_poweron());

	// configure uart
	uart_config_t uart_config = init_uart_config();

	int intr_alloc_flags = 0;
	ESP_ERROR_CHECK(uart_driver_install(UART_PORT_NUM, UART_RX_BUF_SIZE * 2, 0, 20, &uart_queue, intr_alloc_flags));
	ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_config));
	ESP_ERROR_CHECK(uart_set_pin(UART_PORT_NUM, MODEM_UART_TXD, MODEM_UART_RXD, MODEM_UART_RTS, MODEM_UART_CTS));

	//Set uart pattern detect function.
	uart_enable_pattern_det_baud_intr(UART_PORT_NUM, '+', 1, 9, 0, 0);
	//Reset the pattern queue length to record at most 20 pattern positions.
	uart_pattern_queue_reset(UART_PORT_NUM, 20);

	// Configure esp event loop
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	ESP_ERROR_CHECK(esp_event_handler_instance_register(SMS_EVENTS, SMS_EVENT_NEW_MESSAGE, sms_new_message_handler, NULL, NULL));

	// TODO: is very unlike to have a error here... maybe
	modem_driver_init();

	// Check whether it has been started
	bool started = check_respond();
	if (!started) {
		ESP_LOGI(TAG, "Wait modem started...");
		// Wait for the modem to finish booting
		vTaskDelay(pdMS_TO_TICKS(MODEM_START_WAIT_MS));
	}

	xTaskCreate(uart_event_task, "uart_event_task", 3072, NULL, 12, NULL);
	xTaskCreate(gnss_task, "gnss_task", 8192, NULL, 12, NULL);
	xTaskCreate(test_task, "test_task", 8192, NULL, 12, NULL);
}
