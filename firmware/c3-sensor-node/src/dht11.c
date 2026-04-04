#include "dht11.h"

void dht11_init(int gpio_pin) { }

bool dht11_read(dht11_reading_t *out_data) {
    out_data->temperature = 25.5;
    out_data->humidity = 60.0;
    return true; 
}