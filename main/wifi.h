#pragma once

#include "esp_err.h"

esp_err_t wifi_init(void);

esp_err_t wifi_connect(char* wifi_ssid, char* wifi_password);

esp_err_t wifi_disconnect(void);

esp_err_t wifi_deiniter(void);

void wifi_handle_setup(char* wifi_ssid, char* wifi_password);
