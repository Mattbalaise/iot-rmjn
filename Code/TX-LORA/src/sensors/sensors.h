#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

extern Adafruit_BME280 bme; // I2C

void init_BME280_sensor();
float* read_BME280_sensor();
