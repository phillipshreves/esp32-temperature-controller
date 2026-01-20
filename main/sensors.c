#include "sensors.h"
#include "http_client.h"

#include "ds18b20.h"

#include "esp_http_client.h"
#include "onewire_bus_impl_rmt.h"
#include "onewire_device.h"
#include "onewire_types.h"
#include "ds18b20.h"

#include "esp_err.h"
#include "esp_log.h"

#include "consts.h"

#define TAG "SENSORS"

heater_settings get_heater_settings_for_address(onewire_device_address_t address) {
	switch (address) {
		case 0x78634B008748EB28: 
			return (struct heater_settings){.turn_on_below_temp = 60, .turn_off_above_temp = 65, .plug_hostname = "192.168.1.96"};
		case 0xCC0FFD0087C20C28: 
			return (struct heater_settings){.turn_on_below_temp = 50, .turn_off_above_temp = 55, .plug_hostname = "192.168.1.97"};
		case 0xB6161A0087BB3228: 
			return (struct heater_settings){.turn_on_below_temp = 45, .turn_off_above_temp = 50, .plug_hostname = "192.168.1.98"};
		case 0xBE011432B3EFBE28: 
			return (struct heater_settings){.turn_on_below_temp = 35, .turn_off_above_temp = 40, .plug_hostname = "192.168.1.99"};
		default:
			return (struct heater_settings){.turn_on_below_temp = 35, .turn_off_above_temp = 45, .plug_hostname = NULL};
	}
}

int sensors_get_device_count(onewire_bus_handle_t *bus, ds18b20_device_handle_t *ds18b20s) {
	int ds18b20_device_num = 0;
	onewire_device_iter_handle_t iter = NULL;
	onewire_device_t next_onewire_device;
	esp_err_t search_result = ESP_OK;
	// create 1-wire device iterator, which is used for device search
	ESP_ERROR_CHECK(onewire_new_device_iter(*bus, &iter));
	ESP_LOGI(TAG, "Device iterator created, start searching...");
	do {
		search_result = onewire_device_iter_get_next(iter, &next_onewire_device);
		if (search_result == ESP_OK) {
			// found a new device, let's check if we can upgrade it to a DS18B20
			ds18b20_config_t ds_cfg = {};
			onewire_device_address_t address;
			if (ds18b20_new_device_from_enumeration(&next_onewire_device, &ds_cfg, &ds18b20s[ds18b20_device_num]) == ESP_OK){
				ds18b20_get_device_address(ds18b20s[ds18b20_device_num], &address);
				ESP_LOGI(TAG, "Found a DS18B20{%d], address %016llX", ds18b20_device_num, address);
				ds18b20_device_num++;
			} else {
				ESP_LOGI(TAG, "Found an unknown device, address: %016llX", next_onewire_device.address);
			}
		} else if (search_result != ESP_ERR_NOT_FOUND) {
			ESP_LOGE(TAG, "Error during device search: %s", esp_err_to_name(search_result));
		}
	} while (search_result != ESP_ERR_NOT_FOUND);
	ESP_ERROR_CHECK(onewire_del_device_iter(iter));

	ESP_LOGI(TAG, "Searching done, %d DS18B20 device(s) found", ds18b20_device_num);

	return ds18b20_device_num;
}

float sensors_temperature_report(ds18b20_device_handle_t ds18b20) {
		float temperature_c = 0;
		float temperature_f = 0;
		ESP_ERROR_CHECK(ds18b20_get_temperature(ds18b20, &temperature_c));
		temperature_f = temperature_c * 9.0 / 5.0 + 32.0;
		ESP_LOGI(TAG, "Temperature read: %.2f F", temperature_f);
		return temperature_f;
}

void sensors_setup_device_bus(onewire_bus_handle_t *bus, ds18b20_device_handle_t *ds18b20s) {
	// install 1-wire bus
	onewire_bus_config_t bus_config = {
		.bus_gpio_num = ONEWIRE_BUS_GPIO,
		.flags = {
			.en_pull_up = false, 
		}
	};
	onewire_bus_rmt_config_t rmt_config = {
		.max_rx_bytes = 10, // 1byte ROM command + 8byte ROM number + 1byte device command
	};

	ESP_ERROR_CHECK(onewire_new_bus_rmt(&bus_config, &rmt_config, bus));
}

void sensors_process_reading(ds18b20_device_handle_t ds18b20s[], int ds18b20_device_num) {
	float temp = sensors_temperature_report(ds18b20s[ds18b20_device_num]);
	onewire_device_address_t address;
	ds18b20_get_device_address(ds18b20s[ds18b20_device_num], &address);
	heater_settings settings = get_heater_settings_for_address(address);
	ESP_LOGI(TAG, "Heater settings at address %016llX: turn on below %d, turn off above %d", address, settings.turn_on_below_temp, settings.turn_off_above_temp);

	if (settings.plug_hostname == NULL || settings.plug_hostname[0] == '\0') {
		ESP_LOGW(TAG, "No plug hostname configured for address %016llX, skipping control", address);
		return;
	}

	char local_response_buffer[MAX_HTTP_OUTPUT_BUFFER + 1] = {0};
	esp_http_client_config_t plug_control_config = {
				.user_data = local_response_buffer,        // Pass address of local buffer to get response
	};

	if (temp < settings.turn_on_below_temp) {
		ESP_LOGI(TAG, "Temperature %.2f F is below threshold %d F, turning ON plug %s", temp, settings.turn_on_below_temp, settings.plug_hostname);
		http_client_toggle_tasmota_plug(&plug_control_config, settings.plug_hostname, true);
	} else if (temp > settings.turn_off_above_temp) {
		ESP_LOGI(TAG, "Temperature %.2f F is above threshold %d F, turning OFF plug %s", temp, settings.turn_off_above_temp, settings.plug_hostname);
		http_client_toggle_tasmota_plug(&plug_control_config, settings.plug_hostname, false);
	} else {
		ESP_LOGI(TAG, "Temperature %.2f F is within thresholds (%d F - %d F), no action taken for plug %s", temp, settings.turn_on_below_temp, settings.turn_off_above_temp, settings.plug_hostname);
	}
}

void sensors_process_all(onewire_bus_handle_t bus, ds18b20_device_handle_t ds18b20s[], int ds18b20_device_num) {
	ESP_ERROR_CHECK(ds18b20_trigger_temperature_conversion_for_all(bus));
	for (int i = 0; i < ds18b20_device_num; i++) {
		sensors_process_reading(ds18b20s, i);
	}

	vTaskDelay((1000*5) / portTICK_PERIOD_MS);
}


