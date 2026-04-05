#ifndef ESPNOW_RECEIVER_H
#define ESPNOW_RECEIVER_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    float temperature;
    float humidity;
    float accel_x;
    float accel_y;
    float accel_z;
    bool alert_motion;
    bool alert_temp;
} __attribute__((packed)) sensor_payload_t;

void espnow_receiver_init(void);

#endif