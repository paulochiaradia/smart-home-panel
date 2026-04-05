#ifndef ESPNOW_SENDER_H
#define ESPNOW_SENDER_H

#include <stdint.h>
#include <stdbool.h>

// Este é o "Contrato". O pacote máximo do ESP-NOW é 250 bytes.
// Nossa struct tem cerca de 22 bytes, vai sobrar espaço!
typedef struct {
    float temperature;
    float humidity;
    float accel_x;
    float accel_y;
    float accel_z;
    bool alert_motion; // Flag de movimento
    bool alert_temp;   // Flag de temperatura alta
} __attribute__((packed)) sensor_payload_t; 
// O __attribute__((packed)) garante que o compilador não coloque bytes vazios no meio, 
// o que poderia corromper a leitura no S3.

void espnow_sender_init(void);
void espnow_send_data(sensor_payload_t *payload);

#endif