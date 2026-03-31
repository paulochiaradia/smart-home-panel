idf_component_register(
    SRCS
        "main.c"
        "dht11.c"
        "mpu6050.c"
        "actuators.c"
        "mqtt_client.c"
        "espnow_sender.c"
    INCLUDE_DIRS "."
)
