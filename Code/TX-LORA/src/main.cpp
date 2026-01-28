#include <Arduino.h>
#include <SHA256.h>
#include <Ed25519.h>
#include <AES.h>
#include <ctime> 
#include <WiFiS3.h>
#include <time.h>
#include <WiFiUdp.h>
#include <config/config.h>
#include <NTPClient.h>
#include <security/security.h>

void setup()
{
  Serial.begin(SERIAL_BAUD);
  LORA_SERIAL.begin(LORA_BAUD);
  pinMode(LED_PIN, OUTPUT);
  init_wifi();
  init_ntp_clock();
  init_lora_config();
  aes.setKey(((uint8_t*)aesKey), strlen(aesKey));
}

void loop() {

  if(LORA_SERIAL.available())
  {
    String secure_message_str = secure_message(LORA_MSG);
    send_secure_message(secure_message_str);
  }
  sync_ntp_clock();
  delay(1000);
}
