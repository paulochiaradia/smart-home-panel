#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "dht11.h"
#include "mpu6050.h"
#include "actuators.h" // Importando nosso novo contrato de atuadores

static const char *TAG = "MAIN_NODE";

// Pinos ajustados para o ESP32-C3 SuperMini
#define DHT11_PIN    4
#define MPU_SDA_PIN  8
#define MPU_SCL_PIN  9
#define LED_PIN      3
#define BUZZER_PIN   7

void sensor_task(void *pvParameters) {
    dht11_reading_t dht_data;
    mpu6050_reading_t mpu_data;

    while (1) {
        // Pisca o LED rapidamente para indicar que o ciclo começou (Heartbeat)
        led_set(true);
        vTaskDelay(pdMS_TO_TICKS(50));
        led_set(false);

        // Leitura real do DHT11
        if (dht11_read(&dht_data)) {
            ESP_LOGI(TAG, "DHT11 -> Temp: %.1f°C | Umidade: %.1f%%", dht_data.temperature, dht_data.humidity);
            
            // Exemplo de som 1: Alerta de temperatura alta
            if (dht_data.temperature > 35.0) {
                ESP_LOGW(TAG, "ALERTA: Temperatura muito alta!");
                buzzer_play_tone(2500, 500); // Apito agudo e contínuo de 500ms
            }
        } else {
            ESP_LOGE(TAG, "Falha ao ler o DHT11.");
        }

        // Leitura real do MPU6050
        if (mpu6050_read(&mpu_data)) {
            ESP_LOGI(TAG, "MPU6050 -> X: %.2f | Y: %.2f | Z: %.2f", 
                     mpu_data.accel_x, mpu_data.accel_y, mpu_data.accel_z);
            
            // Exemplo de som 2: Alarme de movimento (Sirene)
            if (mpu_data.motion_detected) {
                ESP_LOGW(TAG, "ALERTA: Movimento detectado no painel!");
                // Sirene: Alterna entre duas frequências
                buzzer_play_tone(800, 300);  // Som grave por 300ms
                buzzer_play_tone(1200, 300); // Som agudo por 300ms
            }
        } else {
            ESP_LOGE(TAG, "Falha ao ler o MPU6050.");
        }

        ESP_LOGI(TAG, "--------------------------------------------------");
        vTaskDelay(pdMS_TO_TICKS(1400)); // Espera o resto do tempo para fechar o ciclo
    }
}

void app_main(void) {
    ESP_LOGI(TAG, "Iniciando Smart Home Panel - Sensor Node C3");

    // Inicializa os hardwares
    dht11_init(DHT11_PIN);
    mpu6050_init(MPU_SDA_PIN, MPU_SCL_PIN);
    actuators_init(LED_PIN, BUZZER_PIN); // Inicializa o PWM do Buzzer e o GPIO do LED

    // Bipe duplo de inicialização confirmando que ligou
    buzzer_play_tone(2000, 100);
    vTaskDelay(pdMS_TO_TICKS(100));
    buzzer_play_tone(2000, 100);

    // Cria a task do FreeRTOS
    xTaskCreate(sensor_task, "sensor_task", 4096, NULL, 5, NULL);
}
