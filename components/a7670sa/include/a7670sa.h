#include "modem.h"

#define BOARD_PWRKEY_PIN GPIO_NUM_4
#define MODEM_DTR_PIN GPIO_NUM_25
#define BOARD_POWERON_PIN GPIO_NUM_12
#define MODEM_RESET_PIN GPIO_NUM_5
#define MODEM_RING_PIN GPIO_NUM_33
#define MODEM_POWERON_PULSE_WIDTH_MS (100)
#define MODEM_POWEROFF_PULSE_WIDTH_MS (3000)

#define PRODUCT_MODEL_NAME "LilyGo-A7670 ESP32 Version"

#define MODEM_UART_TXD 26
#define MODEM_UART_RXD 27
#define MODEM_UART_RTS (UART_PIN_NO_CHANGE)
#define MODEM_UART_CTS (UART_PIN_NO_CHANGE)

#define UART_PORT_NUM 2
#define MODEM_UART_BAUD_RATE 115200 // default baud rate on a7670sa

#define UART_RX_BUF_SIZE (1024)

esp_err_t a7670sa_init(void);
esp_err_t a7670sa_modem_init(modem_ctx_t *modem);
esp_err_t a7670sa_sleep();
esp_err_t a7670sa_wakeup();

QueueHandle_t a7670sa_get_uart_queue();
esp_err_t a7670sa_setup_ri_wakeup();
void a7670sa_set_s_sms_task_handle(TaskHandle_t h);
