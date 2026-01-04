#include "esp_err.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "onewire_bus_impl_rmt.h"
#include "onewire_device.h"
#include "onewire_types.h"
#include "portmacro.h"
#include "ds18b20.h"

void app_main(void) {
	#define ONEWIRE_BUS_GPIO 4
	#define ONEWIRE_MAX_DS18B20 2

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

	int ds18b20_device_num = 0;
	ds18b20_device_handle_t ds18b20s[ONEWIRE_MAX_DS18B20];
	onewire_device_iter_handle_t iter = NULL;
	onewire_device_t next_onewire_device;
	esp_err_t search_result = ESP_OK;
	const char *TAG = "DS18B20_EXAMPLE";

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
				ds18b20_device_num++;
			} else {
				ESP_LOGI(TAG, "Found an unknown device, address: %016llX", next_onewire_device.address);
			}
		}
	} while (search_result != ESP_ERR_NOT_FOUND);
	ESP_ERROR_CHECK(onewire_del_device_iter(iter));
	ESP_LOGI(TAG, "Searching done, %d DS18B20 device(s) found", ds18b20_device_num);

	while (ds18b20_device_num > 0) {
		ESP_ERROR_CHECK(ds18b20_trigger_temperature_conversion_for_all(bus));
		for (int i = 0; i < ds18b20_device_num; i++) {
			float temperature_c = 0;
			float temperature_f = 0;
			ESP_ERROR_CHECK(ds18b20_get_temperature(ds18b20s[i], &temperature_c));
			temperature_f = temperature_c * 9.0 / 5.0 + 32.0;
			ESP_LOGI(TAG, "Temperature read from DS18B20[%d]: %.2f F", i, temperature_f);
		}

		vTaskDelay(3000 / portTICK_PERIOD_MS);
	}
}
