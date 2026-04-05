#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "espnow_receiver.h"

static const char *TAG = "MAIN_S3";

void app_main(void) {
    ESP_LOGI(TAG, "Iniciando Smart Home Panel - Display Node S3");

    // O Wi-Fi do ESP32 precisa que a memória NVS esteja ligada para funcionar
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Liga a antena e fica escutando
    espnow_receiver_init();

    // Mantém o sistema vivo. O trabalho real acontece dentro do callback de recepção!
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}