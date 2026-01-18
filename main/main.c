#include <stdio.h>

#include "esp_err.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "freertos/task.h"
#include "onewire_types.h"
#include "portmacro.h"
#include "ds18b20.h"

#include "consts.h"
#include "http_client.h"
#include "secrets.h"
#include "sensors.h"
#include "wifi.h"

#define TAG "MAIN"

#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_HASH_TO_ELEMENT
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK

void app_main(void) {
	onewire_bus_handle_t bus = NULL;
	ds18b20_device_handle_t ds18b20s[ONEWIRE_MAX_DS18B20];

	// TODO test if this function is working post-refactor
	sensors_setup_device_bus(&bus, ds18b20s);
	int ds18b20_device_num = sensors_get_device_count(&bus, ds18b20s);
	
	wifi_handle_setup(WIFI_SSID, WIFI_PASSWORD);

	while (ds18b20_device_num > 0) {
		ESP_ERROR_CHECK(ds18b20_trigger_temperature_conversion_for_all(bus));
		for (int i = 0; i < ds18b20_device_num; i++) {
			sensors_temperature_report(ds18b20s[i]);
		}

		vTaskDelay((1000*2) / portTICK_PERIOD_MS);
	}

	ESP_LOGI(TAG, "Disconnecting from Wifi...");
	ESP_ERROR_CHECK(wifi_disconnect());
	ESP_ERROR_CHECK(wifi_deiniter());
}
