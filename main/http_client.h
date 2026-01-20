#pragma once

#include "esp_http_client.h"

void http_client_call(esp_http_client_config_t *config_ref);

esp_err_t http_client_get_tasmota_command_config(esp_http_client_config_t* config, const char* hostname, const char* command);

void http_client_toggle_tasmota_plug(esp_http_client_config_t* config, char* hostname, bool state);
