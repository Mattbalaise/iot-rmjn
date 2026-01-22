#include <Arduino.h>
#include <SoftwareSerial.h>
#include <WiFiS3.h>
#include <PubSubClient.h>
#include <Crypto.h>
#include <SHA256.h>
#include <AES.h>

// =====================
// Sécurité LoRa P2P
// =====================
const char* HMAC_KEY = "super_secret_key_123";
unsigned long lastSeq = 0;

// =====================
// LoRa
// =====================
#define LORA_RX 0
#define LORA_TX 1
SoftwareSerial loraSerial(LORA_RX, LORA_TX);

// =====================
// WiFi
// =====================
const char* ssid = "Jean Galaxy S24+";
const char* password = "ABCDE12345";

// =====================
// MQTT
// =====================
const char* mqttServer = "10.208.19.19";
const int mqttPort = 1883;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// =====================
// Fonctions utilitaires
// =====================

// HEX → ASCII
String hexToString(String hex) {
  String text = "";
  for (unsigned int i = 0; i < hex.length(); i += 2) {
    char c = (char)strtol(hex.substring(i, i + 2).c_str(), NULL, 16);
    text += c;
  }
  return text;
}

// Connexion MQTT
void reconnectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Connexion MQTT...");
    if (mqttClient.connect("ArduinoR4_LoRa")) {
      Serial.println("OK");
    } else {
      Serial.print(" échec rc=");
      Serial.println(mqttClient.state());
      delay(2000);
    }
  }
}

// =====================
// decrypt message
// =====================
String decrypt_message(String message) {

}

// =====================
// SETUP
// =====================
void setup() {

  Serial.begin(9600);
  loraSerial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);

  // ---------- LoRa ----------
  loraSerial.println("AT");
  loraSerial.println("AT+RESET");
  delay(1500);
  loraSerial.println("AT+MODE=TEST");
  delay(500);
  loraSerial.println("AT+TEST=RFCFG,868.4,SF7,125,12,15,14,ON,OFF,OFF");
  loraSerial.println("AT+TEST=RXLRPKT");

  Serial.println("[LoRa RX] En écoute...");

  // ---------- WiFi ----------
  Serial.print("Connexion WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connecté");
  Serial.print("IP Arduino : ");
  Serial.println(WiFi.localIP());

  // ---------- MQTT ----------
  mqttClient.setServer(mqttServer, mqttPort);
}

// =====================
// LOOP
// =====================
void loop() {

  // Maintien MQTT
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  mqttClient.loop();

  // ---------- Réception LoRa ----------
  if (loraSerial.available()) {

    digitalWrite(LED_BUILTIN, HIGH);
    String input = loraSerial.readStringUntil('\n');

    if (input.indexOf("+TEST: RX") != -1 && input.indexOf("\"") != -1) {

      int firstQuote = input.indexOf('\"');
      int lastQuote  = input.lastIndexOf('\"');

      if (lastQuote > firstQuote) {

        // 1️⃣ Décodage
        String hexMsg = input.substring(firstQuote + 1, lastQuote);
        String vraiMessage = hexToString(hexMsg);
        Serial.println("Message reçu :");
        Serial.println(vraiMessage);

        mqttClient.publish("lora/reception", vraiMessage.c_str());
        Serial.println("Envoyé au broker MQTT");
      }
    }

    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
  }

  // Remise en écoute LoRa
  loraSerial.println("AT+TEST=RXLRPKT");
}
