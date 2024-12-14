idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS "."
                       REQUIRES dht ultrasonic esp_wifi nvs_flash esp_system esp_netif)
