idf_component_register(SRCS "main.c" "http_client.c" "sensors.c" "wifi.c"
                       PRIV_REQUIRES esp_wifi
                       PRIV_REQUIRES esp_http_client
                       PRIV_REQUIRES nvs_flash
                       PRIV_REQUIRES esp_event
                       PRIV_REQUIRES esp-tls
                       INCLUDE_DIRS ".")

