#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "dht11.h"
#include "mpu6050.h"

static const char *TAG = "MAIN_NODE";

// Pinos ajustados para o ESP32-C3 SuperMini
#define DHT11_PIN 4
#define MPU_SDA_PIN 8
#define MPU_SCL_PIN 9

void sensor_task(void *pvParameters) {
    dht11_reading_t dht_data;
    mpu6050_reading_t mpu_data;

    while (1) {
        // Leitura real do DHT11 via GPIO
        if (dht11_read(&dht_data)) {
            ESP_LOGI(TAG, "DHT11 -> Temp: %.1f°C | Umidade: %.1f%%", dht_data.temperature, dht_data.humidity);
        } else {
            ESP_LOGE(TAG, "Falha ao ler o DHT11. Verifique a conexao no pino 4.");
        }

        // Leitura real do MPU6050 via I2C
        if (mpu6050_read(&mpu_data)) {
            ESP_LOGI(TAG, "MPU6050 -> X: %.2f | Y: %.2f | Z: %.2f", 
                     mpu_data.accel_x, mpu_data.accel_y, mpu_data.accel_z);
        } else {
            ESP_LOGE(TAG, "Falha ao ler o MPU6050. Verifique as conexoes (SDA=8, SCL=9).");
        }

        ESP_LOGI(TAG, "--------------------------------------------------");
        vTaskDelay(pdMS_TO_TICKS(2000)); // Espera 2 segundos
    }
}

void app_main(void) {
    ESP_LOGI(TAG, "Iniciando Smart Home Panel - Sensor Node C3");

    // Inicializa os hardwares passando os pinos definidos
    dht11_init(DHT11_PIN);
    mpu6050_init(MPU_SDA_PIN, MPU_SCL_PIN);

    // Cria a task do FreeRTOS para ler os sensores em background
    xTaskCreate(sensor_task, "sensor_task", 4096, NULL, 5, NULL);
}