#include "espnow_receiver.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_idf_version.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "ESPNOW_RECV";

static void log_local_mac_once(const char *context_tag) {
    static bool mac_logged = false;
    if (mac_logged) {
        return;
    }

    uint8_t mac[6];
    esp_err_t err = esp_wifi_get_mac(WIFI_IF_STA, mac);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "%s MAC local (S3): %02X:%02X:%02X:%02X:%02X:%02X",
                 context_tag,
                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        mac_logged = true;
    } else {
        ESP_LOGW(TAG, "Nao foi possivel ler MAC local: %s", esp_err_to_name(err));
    }
}

// Callback disparado magicamente pelo hardware quando um pacote chega pelo ar
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
static void on_data_recv(const esp_now_recv_info_t *esp_now_info, const uint8_t *data, int len) {
    const uint8_t *mac_addr = esp_now_info->src_addr;
#else
static void on_data_recv(const uint8_t *mac_addr, const uint8_t *data, int len) {
#endif

    // Garante que o MAC do S3 apareca mesmo se o monitor abrir depois do boot.
    log_local_mac_once("RX");

    // Verifica se a caixa recebida tem o tamanho exato da nossa struct
    if (len != sizeof(sensor_payload_t)) {
        ESP_LOGW(TAG, "Tamanho de pacote inesperado: %d bytes", len);
        return;
    }

    // Desempacota os dados
    sensor_payload_t *payload = (sensor_payload_t *)data;

    // Imprime na tela o que chegou!
    ESP_LOGI(TAG, "--- PACOTE RECEBIDO DE %02X:%02X:%02X:%02X:%02X:%02X ---",
             mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    ESP_LOGI(TAG, "Temperatura: %.1f°C | Umidade: %.1f%%", payload->temperature, payload->humidity);
    ESP_LOGI(TAG, "MPU6050 -> X: %.2f | Y: %.2f | Z: %.2f", payload->accel_x, payload->accel_y, payload->accel_z);
    
    if (payload->alert_motion) ESP_LOGW(TAG, ">>> ALERTA DE MOVIMENTO RECEBIDO! <<<");
    if (payload->alert_temp) ESP_LOGW(TAG, ">>> ALERTA DE AQUECIMENTO RECEBIDO! <<<");
    ESP_LOGI(TAG, " "); // Linha em branco para separar as leituras
}

void espnow_receiver_init(void) {
    // Liga a antena Wi-Fi em modo Station (Estação)
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));

    log_local_mac_once("BOOT");
             
    // Inicia o ESP-NOW e liga o ouvido (callback)
    if (esp_now_init() != ESP_OK) {
        ESP_LOGE(TAG, "Erro ao inicializar ESP-NOW");
        return;
    }
    esp_now_register_recv_cb(on_data_recv);
    ESP_LOGI(TAG, "ESP-NOW Receiver inicializado. Antena escutando...");
}