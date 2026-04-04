#include "dht11.h"
#include "driver/gpio.h"
#include "rom/ets_sys.h" // Para usar ets_delay_us (delay de microssegundos)
#include "freertos/FreeRTOS.h"

static int dht_pin;

// Função auxiliar para esperar o pino mudar de estado com limite de tempo (timeout)
static int wait_for_state(int state, int timeout_us) {
    int count = 0;
    while (gpio_get_level(dht_pin) != state) {
        if (count++ > timeout_us) return -1; // Deu erro/timeout
        ets_delay_us(1);
    }
    return count; // Retorna quantos microssegundos demorou
}

void dht11_init(int gpio_pin) {
    dht_pin = gpio_pin;
    gpio_reset_pin(dht_pin);
    // Inicia o pino como entrada para não atrapalhar o barramento
    gpio_set_direction(dht_pin, GPIO_MODE_INPUT); 
}

bool dht11_read(dht11_reading_t *out_data) {
    uint8_t data[5] = {0, 0, 0, 0, 0};

    // 1. O ESP32 inicia a conversa mandando um sinal BAIXO por 20ms
    gpio_set_direction(dht_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(dht_pin, 0);
    vTaskDelay(pdMS_TO_TICKS(20)); // Delay em milissegundos
    
    // 2. Solta o pino (nível ALTO) e volta a ouvir
    gpio_set_level(dht_pin, 1);
    ets_delay_us(30);
    gpio_set_direction(dht_pin, GPIO_MODE_INPUT);

    // Desliga as interrupções do sistema rapidamente para que o FreeRTOS 
    // não pause nossa função bem na hora de contar os microssegundos
    portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
    portENTER_CRITICAL(&mux);

    // 3. Espera a resposta inicial do DHT11 (ele puxa pra baixo por 80us, depois alto por 80us)
    if (wait_for_state(0, 80) == -1) { portEXIT_CRITICAL(&mux); return false; }
    if (wait_for_state(1, 80) == -1) { portEXIT_CRITICAL(&mux); return false; }
    if (wait_for_state(0, 80) == -1) { portEXIT_CRITICAL(&mux); return false; }

    // 4. Lê os 40 bits (5 bytes) de dados de temperatura e umidade
    for (int i = 0; i < 40; i++) {
        // Espera o sinal ficar ALTO
        if (wait_for_state(1, 80) == -1) { portEXIT_CRITICAL(&mux); return false; }
        
        // Mede QUANTO TEMPO o sinal fica ALTO
        int high_time = wait_for_state(0, 100);
        if (high_time == -1) { portEXIT_CRITICAL(&mux); return false; }

        int byte_idx = i / 8;
        data[byte_idx] <<= 1; // Empurra os bits para a esquerda
        
        // Se ficou ALTO por mais de 40us, o DHT11 quis mandar o bit '1'
        if (high_time > 40) { 
            data[byte_idx] |= 1; 
        }
    }

    // Liga as interrupções de volta, o sistema volta ao normal
    portEXIT_CRITICAL(&mux);

    // 5. Verifica se os dados não corromperam no caminho (Checksum)
    if (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
        out_data->humidity = data[0];     // O DHT11 só manda inteiros confiáveis
        out_data->temperature = data[2];
        return true;
    }

    return false; // Checksum falhou (interferência no fio)
}