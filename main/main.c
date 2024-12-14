#include <stdio.h>
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "dht.h"

#define DHT_PIN GPIO_NUM_27
#define TRIGGER_PIN GPIO_NUM_33
#define ECHO_PIN GPIO_NUM_25
#define SSID "ssid"
#define PASS "pass"

void read_temp_humi(float *temperature, float *humidity) {
    if (dht_read_float_data(DHT_TYPE_DHT11, DHT_PIN, humidity, temperature) != ESP_OK) {
        printf("Erro ao ler dados do sensor DHT11\n");
    }
}

float measure_distance() {
    gpio_set_direction(TRIGGER_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(ECHO_PIN, GPIO_MODE_INPUT);
    
    gpio_set_level(TRIGGER_PIN, 0);
    vTaskDelay(2 / portTICK_PERIOD_MS);
    gpio_set_level(TRIGGER_PIN, 1);
    vTaskDelay(10 / portTICK_PERIOD_MS);
    gpio_set_level(TRIGGER_PIN, 0);

    int pulse_duration = 0;
    while (gpio_get_level(ECHO_PIN) == 0);
    int start_time = esp_timer_get_time();
    while (gpio_get_level(ECHO_PIN) == 1);
    int end_time = esp_timer_get_time();

    pulse_duration = end_time - start_time;
    float distance = (pulse_duration / 2.0) * 0.0343;
    return distance;
}

void print_data() {
    float temperature = 0.0;
    float humidity = 0.0;
    float distance = 0.0;

    read_temp_humi(&temperature, &humidity);
    distance = measure_distance();

    printf("Temperatura: %.2f°C, Umidade: %.2f%%, Distância: %.2f cm\n", temperature, humidity, distance);
}

void wifi_init_sta(void) {
    esp_wifi_init(NULL);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = SSID,
            .password = PASS,
        },
    };
    
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_connect();
    printf("Conectando-se à rede Wi-Fi %s...\n", SSID);
}

void wifi_reconnect(void) {
    wifi_init_sta();
}

void task_print_data(void *pvParameters) {
    wifi_ap_record_t ap_info; 

    while (1) {
        esp_err_t err = esp_wifi_sta_get_ap_info(&ap_info);
        if (err == ESP_ERR_WIFI_NOT_CONNECT) {
            wifi_reconnect();
        }
        print_data();
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}


void app_main(void) {
    nvs_flash_init();
    wifi_init_sta();
    xTaskCreate(&task_print_data, "task_print_data", 2048, NULL, 5, NULL);
} 