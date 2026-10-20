#include "modem.h"
#include "at/gnss.h"
#include "at/status_control.h"
#include "at/sim_card.h"
#include "at/network.h"
#include "at/packet_domain.h"
#include "at/serial_interface.h"
#include "at/sms.h"
#include "at/v_25ter.h"
#include "a7670sa.h"

#include "sms_utils.h"
#include "utils.h"
#include "board.h"

static const char *TAG = "common_tasks";

void sms_worker_task(void *pvParameters) {
	modem_ctx_t *modem = (modem_ctx_t *)pvParameters; 

	uint32_t notify_val;

	for (;;) {
		//ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		xTaskNotifyWait(0, 0xFFFFFFFF, &notify_val, portMAX_DELAY);
		ESP_LOGI(TAG, "ISR detected");

		sms_message_t messages[3] = {0};
		size_t m_size = sizeof(messages) / sizeof(messages[0]);
		while (sms_list_messages(modem, messages, m_size) == MODEM_OK) {
			for(size_t i = 0; i < m_size; i++) {
				uint8_t sms_index = messages[i].index;
				if (sms_index > 0) {
					sms_cmti_t cmti = {0};
					cmti.index = sms_index;
					sms_process_cmti(modem, &cmti);
					memset(&messages[i], 0, sizeof(sms_message_t));
				}
			}	
		}
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
	vTaskDelete(NULL);
}

// TODO: Convert this in a init_test_routine is every ok then letsgo
void test_task(void *pvParameters) {
	modem_ctx_t *modem = (modem_ctx_t *)pvParameters;

	for (;;) {
		if (v_25ter_check_at(modem) == MODEM_OK) {
			ESP_LOGI(TAG, "Modem is OK.");
		} else {
			ESP_LOGE(TAG, "Modem has a error.");
		}
		vTaskDelay(pdMS_TO_TICKS(1000)); 

		v_25ter_product_id_info_t v_25ter_product_id_info;
		v_25ter_get_product_id_info(modem,  &v_25ter_product_id_info);
		ESP_LOGI(TAG, "v_25ter_product_id_info.manufacturer = %s", v_25ter_product_id_info.manufacturer);
		ESP_LOGI(TAG, "v_25ter_product_id_info.model = %s", v_25ter_product_id_info.model);
		ESP_LOGI(TAG, "v_25ter_product_id_info.revision = %s", v_25ter_product_id_info.revision);
		ESP_LOGI(TAG, "v_25ter_product_id_info.imei = %s", v_25ter_product_id_info.imei);
		ESP_LOGI(TAG, "v_25ter_product_id_info.gcap = %s", v_25ter_product_id_info.gcap);
		vTaskDelay(pdMS_TO_TICKS(1000)); 

		if (sim_card_is_ready(modem)) {
			ESP_LOGI(TAG, "SIM Card is ready.");
		} else {
			ESP_LOGE(TAG, "SIM Card error.");
		}
		vTaskDelay(pdMS_TO_TICKS(1000)); 

		signal_quality_t signal_quality;
		status_control_query_signal_quality(modem, &signal_quality);
		ESP_LOGI(TAG, "signal_quality.rssi = %d", signal_quality.rssi);
		ESP_LOGI(TAG, "signal_quality.ber = %d", signal_quality.ber);
		vTaskDelay(pdMS_TO_TICKS(1000)); 

		imei_t imei;
		status_control_query_imei(modem, &imei);
		ESP_LOGI(TAG, "imei = %s", imei.imei);
		vTaskDelay(pdMS_TO_TICKS(1000)); 

		uint32_t voltage_mv_out;
		board_read_battery_voltage_mv(&voltage_mv_out);
		ESP_LOGI(TAG, "voltage_mv_out = %d", voltage_mv_out);

		ue_system_information_t ue;
		network_query_ue_sys_information(modem, &ue);
		ESP_LOGI(TAG, "ue.system_mode = %s", ue.system_mode);
		ESP_LOGI(TAG, "ue.operation_mode = %s", ue.operation_mode);
		vTaskDelay(pdMS_TO_TICKS(1000)); 

		// TODO: test modem sleep when buy the battery
		//serial_interface_set_control_uart_sleep(modem, SERIAL_INTERFACE_UART_SLEEP_STATUS_DTR_SLEEP);
		//vTaskDelay(pdMS_TO_TICKS(1000)); 
		//board_sleep();
		//ESP_LOGI(TAG, "MODEM is in sleep_mode");
		//vTaskDelay(pdMS_TO_TICKS(1000)); 

		network_registration_t network_registration;
		network_read_network_registration(modem, &network_registration);
		ESP_LOGI(TAG, "network_registration.n = %d", network_registration.n);
		ESP_LOGI(TAG, "network_registration.stat = %d", network_registration.stat);
		vTaskDelay(pdMS_TO_TICKS(1000)); 

		operator_selection_t operator_selection;
		network_read_operator_selection(modem, &operator_selection);
		ESP_LOGI(TAG, "operator_selection.mode = %d", operator_selection.mode);
		ESP_LOGI(TAG, "operator_selection.format = %d", operator_selection.format);
		ESP_LOGI(TAG, "operator_selection.oper = %s", operator_selection.oper);
		ESP_LOGI(TAG, "operator_selection.act = %d", operator_selection.act);
		vTaskDelay(pdMS_TO_TICKS(1000)); 

		packet_data_protocol_t pdp;
		packet_domain_read_pdp_context(modem, &pdp);
		ESP_LOGI(TAG, "pdp.cid = %d", pdp.cid);
		ESP_LOGI(TAG, "pdp.pdp_type = %s", pdp.pdp_type);
		ESP_LOGI(TAG, "pdp.apn = %s", pdp.apn);
		ESP_LOGI(TAG, "pdp.apn_addr = %s", pdp.apn_addr);
		ESP_LOGI(TAG, "pdp.d_comp = %d", pdp.d_comp);
		ESP_LOGI(TAG, "pdp.h_comp = %d", pdp.h_comp);
		vTaskDelay(pdMS_TO_TICKS(1000)); 

		sms_service_centre_address_t sca;
		sms_read_sca(modem, &sca);
		ESP_LOGI(TAG, "sca.sca = %d", sca.sca);
		ESP_LOGI(TAG, "sca.tosca = %d", sca.tosca);
		vTaskDelay(pdMS_TO_TICKS(1000)); 

		sms_preferred_message_storage_t pms[3];
		sms_read_preferred_message_storage(modem, pms);
		for (int i = 0; i < 3; i++) {
			ESP_LOGI(TAG, "pms[%d].mem = %s",i, pms[i].mem);
			ESP_LOGI(TAG, "pms[%d].used = %d",i, pms[i].used);
			ESP_LOGI(TAG, "pms[%d].total = %d",i, pms[i].total);
		}
		vTaskDelay(pdMS_TO_TICKS(1000)); 

		sms_select_te_character_set(modem, SMS_CHARACTER_SET_GSM);
		vTaskDelay(pdMS_TO_TICKS(1000)); 

		sms_select_message_format(modem, TEXT_MODE);
		vTaskDelay(pdMS_TO_TICKS(1000)); 

		//sms_send_message(modem, "+18298086111", "Hello from ESP32");
		//vTaskDelay(pdMS_TO_TICKS(1000)); 

		sms_message_t messages[3] = {0};
		sms_list_messages(modem, messages, 3);
		vTaskDelay(pdMS_TO_TICKS(1000)); 

		//sms_message_t message = {0};
		//sms_read_message(modem, 3, &message);
		//ESP_LOGI(TAG, "message.index = %d", message.index);
		//ESP_LOGI(TAG, "message.stat = %d", message.stat);
		//ESP_LOGI(TAG, "message.oa_da = %s", message.oa_da);
		//ESP_LOGI(TAG, "message.alpha = %s", message.alpha);
		//ESP_LOGI(TAG, "message.scts = %s", message.scts);
		//ESP_LOGI(TAG, "message.data = |%s|", message.data);
		//vTaskDelay(pdMS_TO_TICKS(1000)); 

		status_control_read_clock(modem);
		vTaskDelay(pdMS_TO_TICKS(5000)); 

		remaining_task_stack();
	}

	vTaskDelete(NULL);
}

void uart_event_task(void *pvParameters) {
	uart_event_t event;
	modem_ctx_t *modem = (modem_ctx_t *)pvParameters; 

	QueueHandle_t uart_queue = a7670sa_get_uart_queue();

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
					//case UART_DATA:
					//uint8_t buffer[1024] = {0};
					//size_t buffered_size = sizeof(buffer); 
					//modem_read_uart(modem, NULL, buffer, buffered_size, 200);
					//sms_cmti_t cmti = {0};
					//bool new = sms_process_uart_pattern_event((char*)buffer, &cmti);
					//if (new) {
					//ESP_LOGI(TAG, "cmti.mem = %s", cmti.mem);
					//ESP_LOGI(TAG, "cmti.index = %d", cmti.index);
					//ESP_ERROR_CHECK(esp_event_post(SMS_EVENTS, SMS_EVENT_NEW_MESSAGE, &cmti, sizeof(cmti), portMAX_DELAY));
					//}
					//break;
					//case UART_PATTERN_DET:
					//uint8_t buffer[1024] = {0};
					//size_t buffered_size = sizeof(buffer); 
					//modem_read_uart(modem, NULL, buffer, buffered_size, 200);
					//ESP_LOGI(TAG, "detect data pattern: %s", buffer);
					//sms_cmti_t cmti = {0};
					//bool new = sms_process_uart_pattern_event((char*)buffer, &cmti);
					//if (new) {
					//ESP_LOGI(TAG, "cmti.mem = %s", cmti.mem);
					//ESP_LOGI(TAG, "cmti.index = %d", cmti.index);
					//ESP_ERROR_CHECK(esp_event_post(SMS_EVENTS, SMS_EVENT_NEW_MESSAGE, &cmti, sizeof(cmti), portMAX_DELAY));
					//}
					//break;
				default: break;
			}
		}
	}
	vTaskDelete(NULL);
} 

