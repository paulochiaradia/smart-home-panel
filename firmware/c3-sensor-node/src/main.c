#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "dht11.h"
#include "mpu6050.h"
#include "actuators.h"
#include "espnow_sender.h" // Inclui o contrato de envio

static const char *TAG = "MAIN_NODE";

#define DHT11_PIN    4
#define MPU_SDA_PIN  8
#define MPU_SCL_PIN  9
#define LED_PIN      3
#define BUZZER_PIN   7

void sensor_task(void *pvParameters) {
    dht11_reading_t dht_data;
    mpu6050_reading_t mpu_data;
    sensor_payload_t payload;
    
    // Variável estática para lembrar o estado anterior do movimento
    static bool was_moving = false;

    while (1) {
        led_set(true);
        vTaskDelay(pdMS_TO_TICKS(50));
        led_set(false);

        // Reset básico do payload
        payload.temperature = 0;
        payload.humidity = 0;
        payload.alert_temp = false;

        // Leitura do DHT11
        if (dht11_read(&dht_data)) {
            payload.temperature = dht_data.temperature;
            payload.humidity = dht_data.humidity;
            ESP_LOGI(TAG, "DHT11 -> Temp: %.1f°C | Umidade: %.1f%%", dht_data.temperature, dht_data.humidity);
            if (dht_data.temperature > 35.0) {
                payload.alert_temp = true;
                buzzer_play_tone(2500, 500);
            }
        }

        // Leitura do MPU6050
        if (mpu6050_read(&mpu_data)) {
            payload.accel_x = mpu_data.accel_x;
            payload.accel_y = mpu_data.accel_y;
            payload.accel_z = mpu_data.accel_z;
            payload.alert_motion = mpu_data.motion_detected;
            ESP_LOGI(TAG, "MPU6050 -> X: %.2f | Y: %.2f | Z: %.2f", mpu_data.accel_x, mpu_data.accel_y, mpu_data.accel_z);

            if (mpu_data.motion_detected) {
                // Alarme de movimento (Sirene)
                ESP_LOGW(TAG, "MOVIMENTO DETECTADO!");
                buzzer_play_tone(800, 200);
                buzzer_play_tone(1200, 200);
                was_moving = true; // Marca que o sensor entrou em movimento
            } 
            else if (was_moving) {
                // ESTABILIZOU: Se antes estava a mexer e agora parou
                ESP_LOGI(TAG, "Sensor estabilizado novamente.");
                
                // Som de confirmação (dois bipes rápidos ascendentes)
                buzzer_play_tone(1000, 100);
                vTaskDelay(pdMS_TO_TICKS(50));
                buzzer_play_tone(1500, 100);
                
                was_moving = false; // Reset do estado
            }
        }

        espnow_send_data(&payload);
        vTaskDelay(pdMS_TO_TICKS(1400)); 
    }
}

void app_main(void) {
    ESP_LOGI(TAG, "Iniciando Smart Home Panel - Sensor Node C3");

    dht11_init(DHT11_PIN);
    mpu6050_init(MPU_SDA_PIN, MPU_SCL_PIN);
    actuators_init(LED_PIN, BUZZER_PIN);
    
    // Inicia a antena (Wi-Fi e ESP-NOW)
    espnow_sender_init();

    buzzer_play_tone(2000, 100);
    vTaskDelay(pdMS_TO_TICKS(100));
    buzzer_play_tone(2000, 100);

    xTaskCreate(sensor_task, "sensor_task", 4096, NULL, 5, NULL);
}