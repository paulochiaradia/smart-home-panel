#ifndef ACTUATORS_H
#define ACTUATORS_H

#include <stdbool.h>

void actuators_init(int led_pin, int buzzer_pin);

// Controle do LED
void led_set(bool state);
void led_toggle();

// Controle do Buzzer (frequência em Hz, duração em milissegundos)
void buzzer_play_tone(int frequency, int duration_ms);

#endif