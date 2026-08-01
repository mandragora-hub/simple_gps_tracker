#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "modem.h"
#include "gnss.h"
#include "http.h"
#include "status_control.h"
#include "sim_card.h"
#include "network.h"
#include "packet_domain.h"
#include "utils.h"

#define ECHO_TEST_TXD 26
#define ECHO_TEST_RXD 27
#define ECHO_TEST_RTS (UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS (UART_PIN_NO_CHANGE)

#define UART_PORT_NUM 2
#define ECHO_UART_BAUD_RATE 115200
#define ECHO_TASK_STACK_SIZE 3072  

#define BOARD_PWRKEY_PIN GPIO_NUM_4
#define MODEM_DTR_PIN GPIO_NUM_25
#define BOARD_POWERON_PIN GPIO_NUM_12
#define MODEM_RESET_PIN GPIO_NUM_5

#define MODEM_POWERON_PULSE_WIDTH_MS (100)
#define MODEM_POWEROFF_PULSE_WIDTH_MS (3000)
#define MODEM_START_WAIT_MS (3000)

#define PRODUCT_MODEL_NAME "LilyGo-A7670 ESP32 Version"

#define PATTERN_CHR_NUM    (3)         /*!< Set the number of consecutive and identical characters received by receiver which defines a UART pattern*/

#define BUF_SIZE (1024)
#define RD_BUF_SIZE (BUF_SIZE)

static const char *TAG = "uart_debug";

static QueueHandle_t uart_queue;

uart_config_t init_uart_config() {
	uart_config_t uart_config = {
		.baud_rate = ECHO_UART_BAUD_RATE,
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
	size_t buffered_size;
	uint8_t* dtmp = (uint8_t*) malloc(RD_BUF_SIZE);
	assert(dtmp);

	for (;;) {
		//Waiting for UART event.
		if (xQueueReceive(uart_queue, (void *)&event, (TickType_t)portMAX_DELAY)) {
			memset(dtmp, 0, RD_BUF_SIZE);
			//bzero(dtmp, RD_BUF_SIZE);
			ESP_LOGI(TAG, "uart[%d] event:", UART_PORT_NUM);
			switch (event.type) {
				//Event of UART receiving data
				/*We'd better handler data event fast, there would be much more data events than
					other types of events. If we take too much time on data event, the queue might
					be full.*/
				case UART_DATA:
					ESP_LOGI(TAG, "[UART DATA]: %d", event.size);
					uart_read_bytes(UART_PORT_NUM, dtmp, event.size, portMAX_DELAY);
					ESP_LOGI(TAG, "%s", dtmp);
					//ESP_LOGI(TAG, "[DATA EVT]:");
					//uart_write_bytes(UART_PORT_NUM, (const char*) dtmp, event.size);
					break;
					//Event of HW FIFO overflow detected
				case UART_FIFO_OVF:
					ESP_LOGI(TAG, "hw fifo overflow");
					// If fifo overflow happened, you should consider adding flow control for your application.
					// The ISR has already reset the rx FIFO,
					// As an example, we directly flush the rx buffer here in order to read more data.
					uart_flush_input(UART_PORT_NUM);
					xQueueReset(uart_queue);
					break;
					//Event of UART ring buffer full
				case UART_BUFFER_FULL:
					ESP_LOGI(TAG, "ring buffer full");
					// If buffer full happened, you should consider increasing your buffer size
					// As an example, we directly flush the rx buffer here in order to read more data.
					uart_flush_input(UART_PORT_NUM);
					xQueueReset(uart_queue);
					break;
					//Event of UART RX break detected
				case UART_BREAK:
					ESP_LOGI(TAG, "uart rx break");
					break;
					//Event of UART parity check error
				case UART_PARITY_ERR:
					ESP_LOGI(TAG, "uart parity error");
					break;
					//Event of UART frame error
				case UART_FRAME_ERR:
					ESP_LOGI(TAG, "uart frame error");
					break;
					//UART_PATTERN_DET
				case UART_PATTERN_DET:
					uart_get_buffered_data_len(UART_PORT_NUM, &buffered_size);
					int pos = uart_pattern_pop_pos(UART_PORT_NUM);
					ESP_LOGI(TAG, "[UART PATTERN DETECTED] pos: %d, buffered size: %d", pos, buffered_size);
					if (pos == -1) {
						// There used to be a UART_PATTERN_DET event, but the pattern position queue is full so that it can not
						// record the position. We should set a larger queue size.
						// As an example, we directly flush the rx buffer here.
						uart_flush_input(UART_PORT_NUM);
					} else {
						uart_read_bytes(UART_PORT_NUM, dtmp, pos, 100 / portTICK_PERIOD_MS);
						uint8_t pat[PATTERN_CHR_NUM + 1];
						memset(pat, 0, sizeof(pat));
						uart_read_bytes(UART_PORT_NUM, pat, PATTERN_CHR_NUM, 100 / portTICK_PERIOD_MS);
						ESP_LOGI(TAG, "read data: %s", dtmp);
						ESP_LOGI(TAG, "read pat : %s", pat);
					}
					break;
					//Others
				default:
					ESP_LOGI(TAG, "uart event type: %d", event.type);
					break;
			}
		}
	}
	free(dtmp);
	dtmp = NULL;
	vTaskDelete(NULL);
} 

static void gnss_task(void *pvParameters) {
	modem_ctx_t modem;
	modem_init(&modem, UART_PORT_NUM);

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
			http_request_t request = {0};
			http_response_t response = {0};

			char osmand_traccar_url[100] = {0};
			build_osmand_traccar_url(osmand_traccar_url, sizeof(osmand_traccar_url), &gnss_info);

			strcpy(request.url, osmand_traccar_url);
			printf("request.url = %s\n", request.url);

			if ((http_perform_action(&modem, &request, &response) == true)) {
				printf("Http sucessfully operation\n");
				printf("response.statuscode = %d\n", response.statuscode);
				printf("response.datalen = %d\n", response.datalen);
				printf("response.content = %s\n", response.content);
			}

		} else {
			ESP_LOGI(TAG, "Waiting for satellite fix...");
		}
		vTaskDelay(pdMS_TO_TICKS(5000));
		remaining_task_stack();
	}

	gnss_sleep(&modem);
	vTaskDelete(NULL);
} 

// only for test. remove me
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


		status_control_read_clock(&modem);
		vTaskDelay(pdMS_TO_TICKS(3000)); 

		remaining_task_stack();
	}

	vTaskDelete(NULL);
}

void app_main(void) {
	gpio_config_t io_conf = {
		.mode = GPIO_MODE_OUTPUT,
		.pin_bit_mask =
			(1ULL << BOARD_PWRKEY_PIN) |
			(1ULL << BOARD_POWERON_PIN) |
			(1ULL << MODEM_DTR_PIN) |
			(1ULL << MODEM_RESET_PIN),
	};

	ESP_ERROR_CHECK(gpio_config(&io_conf));

	ESP_LOGI(TAG, "Set power control pin %d HIGH\n", BOARD_POWERON_PIN);
	gpio_set_level(BOARD_POWERON_PIN, 1);

	// Reset
	ESP_LOGI(TAG, "Reset modem via pin %d\n", MODEM_RESET_PIN);
	gpio_set_level(MODEM_RESET_PIN, 0);
	vTaskDelay(pdMS_TO_TICKS(100));
	gpio_set_level(MODEM_RESET_PIN, 1);
	vTaskDelay(pdMS_TO_TICKS(2600));
	gpio_set_level(MODEM_RESET_PIN, 0);

	// Pull down DTR to ensure the modem is not in sleep state
	ESP_LOGI(TAG, "Set DTR pin %d LOW\n", MODEM_DTR_PIN);
	gpio_set_level(MODEM_DTR_PIN, 0);

	// Turn on the modem
	ESP_LOGI(TAG, "Power on modem via pin %d\n", BOARD_PWRKEY_PIN);
	gpio_set_level(BOARD_PWRKEY_PIN, 0);
	vTaskDelay(pdMS_TO_TICKS(100));
	gpio_set_level(BOARD_PWRKEY_PIN, 1);
	vTaskDelay(pdMS_TO_TICKS(MODEM_POWERON_PULSE_WIDTH_MS));
	gpio_set_level(BOARD_PWRKEY_PIN, 0);

	ESP_LOGI(TAG, "Product model name: %s", PRODUCT_MODEL_NAME);

	// configure uart
	uart_config_t uart_config = init_uart_config();

	int intr_alloc_flags = 0;
	ESP_ERROR_CHECK(uart_driver_install(UART_PORT_NUM, BUF_SIZE * 2, 0, 20, &uart_queue, intr_alloc_flags));
	ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_config));
	ESP_ERROR_CHECK(uart_set_pin(UART_PORT_NUM, ECHO_TEST_TXD, ECHO_TEST_RXD, ECHO_TEST_RTS, ECHO_TEST_CTS));

	//Set uart pattern detect function.
	uart_enable_pattern_det_baud_intr(UART_PORT_NUM, '+', PATTERN_CHR_NUM, 9, 0, 0);
	//Reset the pattern queue length to record at most 20 pattern positions.
	uart_pattern_queue_reset(UART_PORT_NUM, 20);

	// Check whether it has been started
	bool started = check_respond();
	if (!started) {
		ESP_LOGI(TAG, "Wait modem started...");
		// Wait for the modem to finish booting
		vTaskDelay(pdMS_TO_TICKS(MODEM_START_WAIT_MS));
	}

	//xTaskCreate(uart_event_task, "uart_event_task", ECHO_TASK_STACK_SIZE, NULL, 12, NULL);
	//xTaskCreate(gnss_task, "gnss_task", 8192, NULL, 12, NULL);
	xTaskCreate(test_task, "test_task", 8192, NULL, 12, NULL);
}
