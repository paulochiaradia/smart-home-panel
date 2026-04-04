#ifndef DHT11_H
#define DHT11_H
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    float temperature;
    float humidity;
} dht11_reading_t;

void dht11_init(int gpio_pin);
bool dht11_read(dht11_reading_t *out_data);
#endif