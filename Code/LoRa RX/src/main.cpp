#include <Arduino.h>
#include <SoftwareSerial.h>
#include <WiFiS3.h>
#include <PubSubClient.h>
#include <Crypto.h>
#include <SHA256.h>
#include <security/security.h>
#include <utils/utils.h>
#include <network/network.h>
// =====================
// SETUP
// =====================
void setup()
{
  // Set encryption key
  aes.setKey((uint8_t *)key, strlen(key));
  Serial.begin(9600);
  loraSerial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  // ---------- LoRa ----------
  initLora();
  // ---------- WiFi ----------
  connectWifi();
  // ---------- MQTT ----------
  initMqttClient();
}

// =====================
// LOOP
// =====================
void loop()
{
  mqttLoop();
  // ---------- Réception LoRa ----------
  if (loraSerial.available())
  {
    digitalWrite(LED_BUILTIN, HIGH);
    String input = loraSerial.readStringUntil('\n');
    if (input.indexOf("+TEST: RX") != -1 && input.indexOf("\"") != -1)
    {
      LoRaPayload lora = decrypt_secure_message(input);
      if(lora.id_device != 0)
      {
        //vérification du hmac
        bool verifyhmacvar = verifyHMAC((uint8_t*)&lora, lora.hmac, PAYLOAD_SIZE - HMAC_SIZE);
        if(verifyhmacvar)
        {
          Serial.println("HMAC OK");
          publishMessageToMqtt(lora.message);
        }
        else
        {
          Serial.println("HMAC NOT OK [Le message est peut-être corrompu, il ne sera pas envoyé au broker MQTT]");
        } 
      }
        Serial.println("---------------------------------------------------");
      }
    }
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
    loraSerial.println("AT+TEST=RXLRPKT");
  }