#include "mpu6050.h"
#include "driver/i2c.h"

#define I2C_MASTER_PORT I2C_NUM_0
#define MPU6050_ADDR    0x68

void mpu6050_init(int sda_pin, int scl_pin) {
    // Configura os parâmetros do barramento I2C
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = sda_pin,
        .scl_io_num = scl_pin,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000,
    };
    i2c_param_config(I2C_MASTER_PORT, &conf);
    i2c_driver_install(I2C_MASTER_PORT, conf.mode, 0, 0, 0);

    // Acorda o sensor (escreve 0x00 no registrador de energia 0x6B)
    uint8_t write_buf[2] = {0x6B, 0x00}; 
    i2c_master_write_to_device(I2C_MASTER_PORT, MPU6050_ADDR, write_buf, sizeof(write_buf), pdMS_TO_TICKS(1000));
}

bool mpu6050_read(mpu6050_reading_t *out_data) {
    uint8_t reg = 0x3B; 
    uint8_t data[6];
    
    // Lê os 6 bytes de aceleração
    esp_err_t err = i2c_master_write_read_device(I2C_MASTER_PORT, MPU6050_ADDR, &reg, 1, data, 6, pdMS_TO_TICKS(1000));

    if (err == ESP_OK) {
        // Junta os bytes altos e baixos
        int16_t raw_ax = (data[0] << 8) | data[1];
        int16_t raw_ay = (data[2] << 8) | data[3];
        int16_t raw_az = (data[4] << 8) | data[5];

        // Converte para Força G
        out_data->accel_x = raw_ax / 16384.0;
        out_data->accel_y = raw_ay / 16384.0;
        out_data->accel_z = raw_az / 16384.0;
        
        // Lógica de detecção: se os eixos X ou Y passarem de 0.4G, acusa movimento
        if (out_data->accel_x > 0.4 || out_data->accel_x < -0.4 || 
            out_data->accel_y > 0.4 || out_data->accel_y < -0.4) {
            out_data->motion_detected = true;
        } else {
            out_data->motion_detected = false;
        }
        
        return true;
    }
    
    return false;
}