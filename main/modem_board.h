#ifndef MODEM_BOARD_H
#define MODEM_BOARD_H

#define BOARD_PWRKEY_PIN GPIO_NUM_4
#define MODEM_DTR_PIN GPIO_NUM_25
#define BOARD_POWERON_PIN GPIO_NUM_12
#define MODEM_RESET_PIN GPIO_NUM_5

#define MODEM_POWERON_PULSE_WIDTH_MS (100)
#define MODEM_POWEROFF_PULSE_WIDTH_MS (3000)
#define MODEM_START_WAIT_MS (3000)

#define PRODUCT_MODEL_NAME "LilyGo-A7670 ESP32 Version"

esp_err_t modem_board_init_and_poweron();
esp_err_t modem_board_reset();
esp_err_t modem_board_sleep();
esp_err_t modem_board_wakeup();

bool check_modem_respond();

esp_err_t test_routine();

#endif // MODEM_BOARD_H
