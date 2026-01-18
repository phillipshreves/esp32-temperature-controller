#pragma once

#include "onewire_types.h"
#include "ds18b20.h"

void sensors_setup_device_bus(onewire_bus_handle_t *bus, ds18b20_device_handle_t *ds18b20s);

void sensors_temperature_report(ds18b20_device_handle_t ds18b20);

int sensors_get_device_count(onewire_bus_handle_t *bus, ds18b20_device_handle_t *ds18b20s);
