#include "actuators.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static int g_led_pin;
static int g_buzzer_pin;

// Configurações do PWM (LEDC) para o Buzzer
#define BUZZER_TIMER       LEDC_TIMER_0
#define BUZZER_MODE        LEDC_LOW_SPEED_MODE
#define BUZZER_CHANNEL     LEDC_CHANNEL_0
#define BUZZER_DUTY_RES    LEDC_TIMER_13_BIT
#define BUZZER_DUTY_50     4095 // 50% de 8191 (13 bits) para volume máximo

void actuators_init(int led_pin, int buzzer_pin) {
    g_led_pin = led_pin;
    g_buzzer_pin = buzzer_pin;

    // 1. Configura o GPIO do LED
    gpio_reset_pin(g_led_pin);
    gpio_set_direction(g_led_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(g_led_pin, 0);

    // 2. Configura o Timer do PWM do Buzzer
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = BUZZER_MODE,
        .timer_num        = BUZZER_TIMER,
        .duty_resolution  = BUZZER_DUTY_RES,
        .freq_hz          = 2000,  // Frequência inicial
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    // 3. Configura o Canal do PWM do Buzzer
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = BUZZER_MODE,
        .channel        = BUZZER_CHANNEL,
        .timer_sel      = BUZZER_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = g_buzzer_pin,
        .duty           = 0, // Começa mutado (Duty = 0)
        .hpoint         = 0
    };
    ledc_channel_config(&ledc_channel);
}

void led_set(bool state) {
    gpio_set_level(g_led_pin, state ? 1 : 0);
}

void led_toggle() {
    int current_level = gpio_get_level(g_led_pin);
    gpio_set_level(g_led_pin, !current_level);
}

void buzzer_play_tone(int frequency, int duration_ms) {
    if (frequency > 0) {
        // Altera a frequência do tom
        ledc_set_freq(BUZZER_MODE, BUZZER_TIMER, frequency);
        // Liga o som (50% duty cycle)
        ledc_set_duty(BUZZER_MODE, BUZZER_CHANNEL, BUZZER_DUTY_50);
        ledc_update_duty(BUZZER_MODE, BUZZER_CHANNEL);
    }

    // Espera o tempo da nota tocar
    vTaskDelay(pdMS_TO_TICKS(duration_ms));

    // Muta o som (Duty = 0)
    ledc_set_duty(BUZZER_MODE, BUZZER_CHANNEL, 0);
    ledc_update_duty(BUZZER_MODE, BUZZER_CHANNEL);
}