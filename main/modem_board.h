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

#define BATTERY_ADC_PIN	35
#define BATTERY_ADC_CHANNEL ADC_CHANNEL_7 // GPIO 35 corresponds to ADC1 Channel 7
#define ADC_VOLTAGE_DIVIDER_RATIO 2.0f  

typedef enum {
	BATTERY_STATE_NO_BATTERY = 0,
	BATTERY_STATE_DISCHARGING,
	BATTERY_STATE_CHARGING,
	BATTERY_STATE_FULL
} battery_state_t;

esp_err_t modem_board_init_and_poweron();
esp_err_t modem_board_reset();
esp_err_t modem_board_sleep();
esp_err_t modem_board_wakeup();

bool check_modem_respond();

esp_err_t test_routine();

esp_err_t battery_adc_init();

// When connected to the USB, the battery voltage data read is not the real battery voltage
esp_err_t read_battery_voltage_mv(uint32_t *voltage_mv_out);

battery_state_t evaluate_battery_status(uint32_t *voltage_mv_out);
esp_err_t battery_adc_del();

#endif // MODEM_BOARD_H
