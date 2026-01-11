#include <stdio.h>

#include "esp_err.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "freertos/task.h"
#include "onewire_bus_impl_rmt.h"
#include "onewire_device.h"
#include "onewire_types.h"
#include "portmacro.h"
#include "ds18b20.h"
#include "esp_http_server.h"
#include "wifi.h"

#include "secrets.h"

#define TAG "MAIN"

#define ONEWIRE_BUS_GPIO 4
#define ONEWIRE_MAX_DS18B20 10

#define WIFI_MAXIMUM_RETRY 20
	
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_HASH_TO_ELEMENT
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK

int get_device_count(onewire_bus_handle_t bus, ds18b20_device_handle_t *ds18b20s) {
	int ds18b20_device_num = 0;
	onewire_device_iter_handle_t iter = NULL;
	onewire_device_t next_onewire_device;
	esp_err_t search_result = ESP_OK;
	// create 1-wire device iterator, which is used for device search
	ESP_ERROR_CHECK(onewire_new_device_iter(bus, &iter));
	ESP_LOGI(TAG, "Device iterator created, start searching...");
	do {
		search_result = onewire_device_iter_get_next(iter, &next_onewire_device);
		if (search_result == ESP_OK) {
			// found a new device, let's check if we can upgrade it to a DS18B20
			ds18b20_config_t ds_cfg = {};
			onewire_device_address_t address;
			// check if hte device is a DS18B20, if so, return the ds18b20 handle
			if (ds18b20_new_device_from_enumeration(&next_onewire_device, &ds_cfg, &ds18b20s[ds18b20_device_num]) == ESP_OK){
				ds18b20_get_device_address(ds18b20s[ds18b20_device_num], &address);
				ESP_LOGI(TAG, "Found a DS18B20{%d], address %016llX", ds18b20_device_num, address);
				// TODO: use the address to identify the unique sensors
				// (408) DS18B20_EXAMPLE: Found a DS18B20{0], address BE011432B3EFBE28
				// May be best to setup a data structure to map address to location/usage
				ds18b20_device_num++;
			} else {
				ESP_LOGI(TAG, "Found an unknown device, address: %016llX", next_onewire_device.address);
			}
		}
	} while (search_result != ESP_ERR_NOT_FOUND);
	ESP_ERROR_CHECK(onewire_del_device_iter(iter));

	return ds18b20_device_num;
}

void temperature_report(ds18b20_device_handle_t ds18b20) {
		float temperature_c = 0;
		float temperature_f = 0;
		ESP_ERROR_CHECK(ds18b20_get_temperature(ds18b20, &temperature_c));
		temperature_f = temperature_c * 9.0 / 5.0 + 32.0;
		ESP_LOGI(TAG, "Temperature read from DS18B20: %.2f F", temperature_f);
}

void app_main(void) {

	// -- START Temperature sensor setup --
	// install 1-wire bus
	onewire_bus_handle_t bus = NULL;
	onewire_bus_config_t bus_config = {
		.bus_gpio_num = ONEWIRE_BUS_GPIO,
		.flags = {
			.en_pull_up = true, // enable the internal pull-up resistor in case the external device didn't have one
		}
	};
	onewire_bus_rmt_config_t rmt_config = {
		.max_rx_bytes = 10, // 1byte ROM command + 8byte ROM number + 1byte device command
	};
	ESP_ERROR_CHECK(onewire_new_bus_rmt(&bus_config, &rmt_config, &bus));

	ds18b20_device_handle_t ds18b20s[ONEWIRE_MAX_DS18B20];

	int ds18b20_device_num = get_device_count(bus, ds18b20s);
	ESP_LOGI(TAG, "Searching done, %d DS18B20 device(s) found", ds18b20_device_num);
	// -- END Temperature sensor setup --

 	// -- START WiFi setup --
	ESP_ERROR_CHECK(wifi_init());

	esp_err_t wifi_result = wifi_connect(WIFI_SSID, WIFI_PASSWORD);
	if (wifi_result != ESP_OK) {
		ESP_LOGE(TAG, "Failed to connect to Wifi network");
	}

	wifi_ap_record_t ap_info;
	wifi_result = esp_wifi_sta_get_ap_info(&ap_info);
	if (wifi_result == ESP_ERR_WIFI_CONN) {
		ESP_LOGE(TAG, "WiFi station interface not initialized");
	} else if (wifi_result == ESP_ERR_WIFI_NOT_CONNECT) {
		ESP_LOGE(TAG, "WiFi station is not connected");
	} else {
		ESP_LOGI(TAG, "--- Access Point Information ---");
		ESP_LOG_BUFFER_HEX("MAC Address", ap_info.bssid, sizeof(ap_info.bssid));
		ESP_LOG_BUFFER_CHAR("SSID", ap_info.ssid, sizeof(ap_info.ssid));
		ESP_LOGI(TAG, "Primary Channel: %d", ap_info.primary);
		ESP_LOGI(TAG, "RSSI: %d", ap_info.rssi);
	}
	// -- END WiFi setup --

	while (ds18b20_device_num > 0) {
		ESP_ERROR_CHECK(ds18b20_trigger_temperature_conversion_for_all(bus));
		for (int i = 0; i < ds18b20_device_num; i++) {
			temperature_report(ds18b20s[i]);
		}

		vTaskDelay((1000*2) / portTICK_PERIOD_MS);
	}

	ESP_LOGI(TAG, "Disconnecting from Wifi...");
	ESP_ERROR_CHECK(wifi_disconnect());
	ESP_ERROR_CHECK(wifi_deiniter());
}
