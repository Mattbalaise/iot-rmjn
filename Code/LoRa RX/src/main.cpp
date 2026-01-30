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
      int firstQuote = input.indexOf('\"');
      int lastQuote = input.lastIndexOf('\"');
      if (lastQuote > firstQuote)
      {
        Serial.println("Message reçu.");
        //decrypt message
        String hexMsg = input.substring(firstQuote + 1, lastQuote);
        uint8_t resultDecrypted[PAYLOAD_SIZE];
        decrypt_message(hexMsg.c_str(), resultDecrypted);
        LoRaPayload lora = *(LoRaPayload *)resultDecrypted;
        printLoraPayload(lora);
        //vérification du hmac
        bool verifyhmacvar = verifyHMAC((uint8_t*)&lora, lora.hmac, PAYLOAD_SIZE - HMAC_SIZE);
        if(verifyhmacvar)
        {
          Serial.println("HMAC OK");
          if(verifySeq(lora.seq_count))
            {
              Serial.println("Séquence OK");
              publishMessageToMqtt(lora.message);
            }
            else
            {
              Serial.println("Séquence NOT OK [Message en double ou hors séquence]");
            }
        }
        else
        {
          Serial.println("HMAC NOT OK [Le message est peut-être corrompu, il ne sera pas envoyé au broker MQTT]");
        }
        Serial.println("---------------------------------------------------");
      }
        digitalWrite(LED_BUILTIN, LOW);
        delay(200);
        loraSerial.println("AT+TEST=RXLRPKT");
    }
  }
}