#define ENABLE_GxEPD2_GFX 0
#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <SPI.h>
#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>

#define PIN_MOSI   7
#define PIN_SCK    6
#define PIN_CS     10
#define PIN_DC     9
#define PIN_RST    4
#define PIN_BUSY   5

GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> display(
  GxEPD2_154_D67(PIN_CS, PIN_DC, PIN_RST, PIN_BUSY)
);

typedef struct {
    float temperature;
    float humidity;
    float accel_x;
    float accel_y;
    float accel_z;
    bool alert_motion;
    bool alert_temp;
} __attribute__((packed)) sensor_payload_t;

sensor_payload_t dadosAtuais;
bool precisaAtualizarTela = false;
float ultimaTempDesenhada = -100.0;
bool ultimoEstadoMovimento = false; // Memória para detectar a volta ao modo seguro
unsigned long ultimaAtualizacaoMillis = 0;
bool recebeuDados = false;
bool ultimoEstadoComunicacao = false;
bool estadoComunicacaoInicializado = false;
const unsigned long TIMEOUT_COMUNICACAO_MS = 5000UL;

bool sensorEstaOnline() {
    if (!recebeuDados) {
        return false;
    }
    return (millis() - ultimaAtualizacaoMillis) <= TIMEOUT_COMUNICACAO_MS;
}

void desenharPainel() {
    display.setRotation(1);
    // Usamos PartialWindow para ser MUITO mais rápido e não piscar a tela toda
    display.setPartialWindow(0, 0, display.width(), display.height());

    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);
        display.setTextColor(GxEPD_BLACK);

        display.setFont(&FreeMonoBold9pt7b);
        display.setCursor(5, 20);
        display.print("Smart Home Panel");
        display.drawLine(0, 30, 200, 30, GxEPD_BLACK);

        display.setFont(&FreeMonoBold18pt7b);
        display.setCursor(10, 80);
        display.print(dadosAtuais.temperature, 1);
        display.print("C");

        display.setFont(&FreeMonoBold12pt7b);
        display.setCursor(10, 115);
        display.print("Umid: ");
        display.print(dadosAtuais.humidity, 0);
        display.print("%");

        // Exibir status de comunicação do sensor
        display.setFont(&FreeMonoBold9pt7b);
        display.setCursor(10, 135);
        if (!recebeuDados) {
            display.print("Sensor: AGUARDANDO");
        } else if (sensorEstaOnline()) {
            display.print("Sensor: ONLINE");
        } else {
            display.print("Sensor: OFFLINE");
        }

        display.setFont(&FreeMonoBold9pt7b);
        display.setCursor(5, 175);
        if (dadosAtuais.alert_motion) {
            display.print("! MOVIMENTO !");
        } else {
            display.print("Status: Seguro");
        }

    } while (display.nextPage());
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
    if (len == sizeof(sensor_payload_t)) {
        memcpy(&dadosAtuais, incomingData, sizeof(dadosAtuais));
        ultimaAtualizacaoMillis = millis();
        recebeuDados = true;

        // CONDIÇÃO DE OURO: Atualiza se a temp mudar OU se o estado de movimento for DIFERENTE do anterior
        if (abs(dadosAtuais.temperature - ultimaTempDesenhada) >= 0.5 || 
            dadosAtuais.alert_motion != ultimoEstadoMovimento) {
            
            precisaAtualizarTela = true;
            ultimaTempDesenhada = dadosAtuais.temperature;
            ultimoEstadoMovimento = dadosAtuais.alert_motion; // Atualiza a memória
        }
    }
}

void setup() {
    Serial.begin(115200);
    SPI.begin(PIN_SCK, -1, PIN_MOSI, PIN_CS);
    display.init(115200, true, 50, false);
    
    // Primeira limpeza total obrigatória
    display.setFullWindow();
    display.fillScreen(GxEPD_WHITE);
    display.display(); 

    WiFi.mode(WIFI_STA);
    if (esp_now_init() != ESP_OK) return;

    esp_now_register_recv_cb(OnDataRecv);
    precisaAtualizarTela = true;
}

void loop() {
    // Atualiza quando houver troca de estado de comunicação (online/offline)
    bool onlineAgora = sensorEstaOnline();
    if (!estadoComunicacaoInicializado) {
        ultimoEstadoComunicacao = onlineAgora;
        estadoComunicacaoInicializado = true;
    } else if (onlineAgora != ultimoEstadoComunicacao) {
            precisaAtualizarTela = true;
            ultimoEstadoComunicacao = onlineAgora;
    }

    if (precisaAtualizarTela) {
        desenharPainel();
        // Não usamos Hibernate aqui para manter o modo Partial ativo e rápido
        precisaAtualizarTela = false;
    }
    delay(50); 
}