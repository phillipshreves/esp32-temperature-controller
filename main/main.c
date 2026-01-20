#include <stdio.h>

#include "esp_err.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "freertos/task.h"
#include "onewire_types.h"
#include "portmacro.h"
#include "ds18b20.h"

#include "consts.h"
#include "secrets.h"
#include "sensors.h"
#include "wifi.h"

#define TAG "MAIN"

#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_HASH_TO_ELEMENT
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK

void app_main(void) {
	onewire_bus_handle_t bus = NULL;
	ds18b20_device_handle_t ds18b20s[ONEWIRE_MAX_DS18B20];

	wifi_handle_setup(WIFI_SSID, WIFI_PASSWORD);

	sensors_setup_device_bus(&bus, ds18b20s);

	int one_wire_retry_count = 0;
	int ds18b20_device_num = sensors_get_device_count(&bus, ds18b20s);
	while (ds18b20_device_num == 0 && one_wire_retry_count < ONEWIRE_SEARCH_RETRY_COUNT) {
		one_wire_retry_count++;
		ESP_LOGW(TAG, "No DS18B20 devices found, retrying(%s)...", one_wire_retry_count);
		vTaskDelay(ONEWIRE_SEARCH_RETRY_DELAY_MS / portTICK_PERIOD_MS);
		ds18b20_device_num = sensors_get_device_count(&bus, ds18b20s);
	}

	while (ds18b20_device_num > 0) {
		sensors_process_all(bus, ds18b20s, ds18b20_device_num);
	}

	ESP_LOGI(TAG, "Disconnecting from Wifi...");
	ESP_ERROR_CHECK(wifi_disconnect());
	ESP_ERROR_CHECK(wifi_deiniter());
}
