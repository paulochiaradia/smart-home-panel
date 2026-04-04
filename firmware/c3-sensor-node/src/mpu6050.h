#ifndef MPU6050_H
#define MPU6050_H
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    float accel_x;
    float accel_y;
    float accel_z;
    bool motion_detected;
} mpu6050_reading_t;

void mpu6050_init(int sda_pin, int scl_pin);
bool mpu6050_read(mpu6050_reading_t *out_data);
#endif