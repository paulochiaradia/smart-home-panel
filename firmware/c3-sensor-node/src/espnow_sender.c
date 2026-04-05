#include "espnow_sender.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_idf_version.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include <string.h>

static const char *TAG = "ESPNOW_SENDER";

// MAC Address de Broadcast (Grita para quem quiser ouvir)
static uint8_t broadcast_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Assinatura do callback varia entre versões do ESP-IDF
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 5, 0)
static void on_data_sent(const esp_now_send_info_t *tx_info, esp_now_send_status_t status) {
    (void)tx_info;
#else
static void on_data_sent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    (void)mac_addr;
#endif
    ESP_LOGI(TAG, "Status do envio ESP-NOW: %s", status == ESP_NOW_SEND_SUCCESS ? "Sucesso" : "Falha");
}

void espnow_sender_init(void) {
    // 1. O Wi-Fi do ESP32 exige que o NVS esteja ligado
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 2. Prepara e liga o Wi-Fi em modo Station
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    // 3. Inicializa o protocolo ESP-NOW
    if (esp_now_init() != ESP_OK) {
        ESP_LOGE(TAG, "Erro ao inicializar ESP-NOW");
        return;
    }
    esp_now_register_send_cb(on_data_sent);
    esp_now_peer_info_t peer_info = {0};
    memcpy(peer_info.peer_addr, broadcast_mac, 6);
    peer_info.channel = 0; 
    peer_info.encrypt = false; 

    if (esp_now_add_peer(&peer_info) != ESP_OK) {
        ESP_LOGE(TAG, "Falha ao adicionar o peer de broadcast");
        return;
    }

    ESP_LOGI(TAG, "ESP-NOW Sender inicializado com sucesso (Modo Broadcast)!");
}

void espnow_send_data(sensor_payload_t *payload) {
    esp_err_t err = esp_now_send(broadcast_mac, (uint8_t *)payload, sizeof(sensor_payload_t));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Erro ao enviar pacote ESP-NOW: %s", esp_err_to_name(err));
    }
}