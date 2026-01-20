#pragma once

#include "onewire_types.h"
#include "ds18b20.h"

typedef struct heater_settings heater_settings;
struct heater_settings {
	int turn_on_below_temp;
	int turn_off_above_temp;
	char* plug_hostname;
};

void sensors_setup_device_bus(onewire_bus_handle_t *bus, ds18b20_device_handle_t *ds18b20s);

float sensors_temperature_report(ds18b20_device_handle_t ds18b20);

int sensors_get_device_count(onewire_bus_handle_t *bus, ds18b20_device_handle_t *ds18b20s);

heater_settings get_heater_settings_for_address(onewire_device_address_t address);

void sensors_process_reading(ds18b20_device_handle_t ds18b20s[], int ds18b20_device_num);

void sensors_process_all(onewire_bus_handle_t bus, ds18b20_device_handle_t ds18b20s[], int ds18b20_device_num);
