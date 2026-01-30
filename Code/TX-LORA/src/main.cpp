#include <config/config.h>
#include <security/security.h>
#include <network/network.h>
#include <sensors/sensors.h>

void setup()
{
  Serial.begin(SERIAL_BAUD);
  Serial.println("------- Initialisation -------");
  LORA_SERIAL.begin(LORA_BAUD);
  pinMode(LED_PIN, OUTPUT);
  init_BME280_sensor();
  init_wifi();
  init_ntp_clock();
  init_lora_config();
  aes.setKey(((uint8_t*)aesKey), strlen(aesKey));
  Serial.println("------- Initialisation terminée -------");
  Serial.println();
}

void loop() {
  String message = format_message(read_BME280_sensor());  

  if(LORA_SERIAL.available())
  {
    String secure_message_str = secure_message(message.c_str());
    send_secure_message(secure_message_str);
  }
  sync_ntp_clock();
  delay(5000);
}
