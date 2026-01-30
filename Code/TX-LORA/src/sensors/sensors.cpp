#include <sensors/sensors.h>

Adafruit_BME280 bme; // I2C

void init_BME280_sensor(){
  if (!bme.begin(0x76)) {  // Adresse I2C la plus courante (SDA -> A4 / SCL -> A5)
    Serial.println("Capteur BME280 non détecté !");
    while (1);
  }
}

float* read_BME280_sensor(){        
  static float data[3];
  data[0] = bme.readTemperature();
  data[1] = bme.readHumidity();
  data[2] = bme.readPressure() / 100.0F; // Convertir en hPa
  return data;
}
