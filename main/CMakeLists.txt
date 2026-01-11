idf_component_register(SRCS "main.c" "wifi.c" 
                       PRIV_REQUIRES esp_wifi
                       PRIV_REQUIRES esp_http_server
                       PRIV_REQUIRES nvs_flash
                       PRIV_REQUIRES esp_event
                       INCLUDE_DIRS ".")
